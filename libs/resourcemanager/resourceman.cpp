//
// Created by Dean Calver on 18/09/2018.
//

#include "core/core.h"
#include "resourceman.h"
#include <array>
#include "resourcemanager/textresource.h"

namespace ResourceManager {

std::string_view const ResourceMan::DeletedString = {"**DELETED**"};

// resource managers are sort of singletons, we keep a static registry to save a full ptr per resource etc.
// intended usage pattern is to store the shared_ptr returned by create for the lifetime of the manager
// and it should be released last thing after all resource are finished with by reseting the unique_ptr
std::vector<std::weak_ptr<ResourceMan>> s_resourceManagers;
uint32_t s_curResourceManagerCount = 0; // TODO keep a free list

ResourceMan::ResourceMan() :
		indexToBase(1000000) {}

ResourceMan::~ResourceMan()
{
	assert(managerIndex < s_curResourceManagerCount);
	// TODO reclaim the index
}

auto ResourceMan::Create() -> std::shared_ptr<ResourceMan>
{
	struct ResourceManCreator : public ResourceManager::ResourceMan
	{
	};

	auto resourceMan = std::make_shared<ResourceManCreator>();
	s_resourceManagers.push_back(resourceMan);
	resourceMan->managerIndex = s_curResourceManagerCount++;

	TextResource::RegisterResourceHandler(*resourceMan);

	return resourceMan;
}

auto ResourceMan::GetFromIndex(uint32_t index_) -> std::shared_ptr<ResourceMan>
{
	assert(index_ < s_curResourceManagerCount);
	return s_resourceManagers[index_].lock();
}

void ResourceMan::registerStorageHandler(IStorage::Ptr storage_)
{
	assert(prefixToStorage.find(storage_->getPrefix()) == prefixToStorage.end());
	prefixToStorage[storage_->getPrefix()] = storage_;
}

auto ResourceMan::getStorageForPrefix(std::string_view prefix_) -> IStorage::Ptr
{
	auto storage = prefixToStorage.find(prefix_);
	if(storage == prefixToStorage.end()) return {};
	else return storage->second;
}

void ResourceMan::registerHandler(ResourceId id_, ResourceHandler handler_, HasResourceChangedFunc changed_,
								  SaveResourceFunc save_)
{
	if(typeToHandler.find(id_) != typeToHandler.end())
	{
		assert(std::get<1>(typeToHandler[id_][0]) == nullptr);
	}
	// stage 0 must have no extramem wanted
	assert(std::get<0>(handler_) == 0);

	typeToHandler[id_][0] = handler_;
	typeToSavers[id_] = {changed_, save_};
}

auto ResourceMan::registerNextHandler(ResourceId id_, ResourceHandler handler_) -> int
{
	assert(typeToHandler.find(id_) != typeToHandler.end());
	assert(std::get<1>(typeToHandler[id_][0]) != nullptr);

	auto handlers = typeToHandler[id_];
	for(int i = 1; i < MaxHandlerStages; ++i)
	{
		int extramem = 0;
		HandlerInit init;
		std::tie(std::ignore, init, std::ignore) = handlers[i];
		if(init != nullptr) continue;
		typeToHandler[id_][i] = handler_;
		return i;
	}
	return -1;
}

auto ResourceMan::removeHandler(ResourceId id_, int stage_) -> void
{
	assert(typeToHandler.find(id_) != typeToHandler.end());
	if(stage_ == 0)
	{
		for(int i = 0; i < MaxHandlerStages; ++i)
		{
			typeToHandler[id_][i] = {0, nullptr, nullptr};
		}
	} else
	{
		typeToHandler[id_][stage_] = {0, nullptr, nullptr};
	}
}

auto ResourceMan::getIndexFromName(ResourceId id_, ResourceNameView const name_) -> uint64_t
{
	auto it = nameToResourceIndex.find(name_);
	if(it == nameToResourceIndex.end())
	{
		std::lock_guard guard(nameCacheLock);
		// first re-check in case some one get here first
		it = nameToResourceIndex.find(name_);
		if(it == nameToResourceIndex.end())
		{
			// lets do the allocation
			ResourceName name(name_);
			uint64_t id = indexToBase.alloc();
			indexToResourceName[id] = name;
			nameToResourceIndex[name_] = id;

			indexToBase[id] = ResourceHandleBase{id, id_, managerIndex, (uint16_t) 0};
			it = nameToResourceIndex.find(name_);
		}
	}

	return it->second;
}

auto ResourceMan::GetNameFromHandleBase(ResourceHandleBase const& base_) -> ResourceNameView
{
	auto rm = GetFromIndex(base_.index);
	if(rm->indexToBase[base_.index].generation != base_.generation) return ResourceNameView(DeletedString);

	return rm->indexToResourceName[base_.index].getName();
}

auto ResourceHandleBase::acquire() const -> ResourceBase::ConstPtr
{
	auto resourceMan = ResourceMan::GetFromIndex(managerIndex);
	return resourceMan->acquire(*this);
}

auto ResourceHandleBase::tryAcquire() const -> ResourceBase::ConstPtr
{
	auto resourceMan = ResourceMan::GetFromIndex(managerIndex);
	return resourceMan->tryAcquire(*this);
}

auto ResourceMan::acquire(ResourceHandleBase const& base_) -> ResourceBase::Ptr
{
	if(base_.index == ResourceHandleBase::InvalidIndex) return ResourceBase::Ptr();

	assert(typeToHandler.find(base_.id) != typeToHandler.end());
	ResourceBase::Ptr ptr;
	while(!ptr)
	{
		ptr = tryAcquire(base_);
		if(!ptr)
		{
			using namespace std::literals;
			std::string resourceName = "**UNKNOWN**"s;
			auto const it = indexToResourceName.find(base_.index);
			if(it != indexToResourceName.end())
			{
				resourceName = std::string(it->second.getResourceName());
			}
			LOG_S(INFO) << "tryAcquire failed acquiring " << resourceName << " retrying...";
			// TODO back off and sleep a bit here
		}
	}
	return ptr;
}

auto ResourceMan::tryAcquire(ResourceHandleBase const& base_) -> ResourceBase::Ptr
{
	if(base_.index == ResourceHandleBase::InvalidIndex) return ResourceBase::Ptr();

	ResourceBase::Ptr cached = resourceCache.lookup(base_.index);
	if(cached) return cached;

	assert(typeToHandler.find(base_.id) != typeToHandler.end());

	// get storage manager
	assert(indexToResourceName.find(base_.index) != indexToResourceName.end());
	ResourceNameView resourceName = indexToResourceName[base_.index].getView();
	std::string_view prefix = resourceName.getStorage();
	std::string_view name = resourceName.getName();
	std::string_view subObject = resourceName.getSubObject();

	assert(!prefix.empty());
	assert(!name.empty());
	assert(prefixToStorage.find(prefix) != prefixToStorage.end());

	IStorage::Ptr storage = prefixToStorage[prefix];

	ResolverInterface resolver{
			[this]() -> ResourceMan*
			{ return this; },
			[this, resourceName](ResourceHandleBase const& base_) -> void
			{
				this->resolveLink(const_cast<ResourceHandleBase&>(base_), resourceName);
			},
			[&resourceName]()
			{ return resourceName; }
	};

	ChunkHandlers chunks;
	chunks.reserve(typeToHandler.size());

	for(auto const&[type, orderedHandler] : typeToHandler)
	{
		auto lambdaType = type;

		for(int stage = 0; stage < MaxHandlerStages; ++stage)
		{
			size_t extramem = 0;
			HandlerInit init;
			HandlerDestroy destroy;
			std::tie(extramem, init, destroy) = orderedHandler[stage];
			if(init == nullptr) continue;

			auto createFun = [this, lambdaType, resolver, prefix, name, init]
					(std::string_view subObject_, int stage_,
					 uint16_t majorVersion_, uint16_t minorVersion_,
					 size_t size_, std::shared_ptr<void> ptr_) -> bool
			{
				auto ptr = std::static_pointer_cast<ResourceBase>(ptr_);
				bool okay = init(stage_, resolver, majorVersion_, minorVersion_, ptr);
				if(okay)
				{
					if(stage_ == 0)
					{
						assert((size_ & 0x3) == 0);
						ptr->sizeAndStageCount = size_;

						uint64_t id;
						ResourceName newName(prefix, name, subObject_);
						id = getIndexFromName(lambdaType, newName.getResourceName());
						resourceCache.insert(id, ptr);
					} else
					{
						ptr->sizeAndStageCount = (ptr->sizeAndStageCount & ~0x3) |
												 std::max(ptr->getStageCount(), (uint8_t) stage_);

					}
					return true;
				} else
				{
					LOG_S(WARNING) << name << " load stage " << stage_ << "has failed to process";
					return false;
				}
			};

			chunks.emplace_back(
					Binny::IBundle::ChunkHandler{(uint32_t) type, stage, (uint32_t) extramem, createFun, destroy, true,
												 false}
			);

		}
	}
	bool okay = storage->read(resourceName, &malloc, &free, chunks);
	if(okay)
	{
		return resourceCache.lookup(base_.index);
	} else
	{
		return {};
	}

}

auto ResourceMan::resolveLink(ResourceHandleBase& link_, ResourceNameView const& current_) -> void
{
	if(link_.isValid())
	{
		return;
	}

	ResourceNameView nameView(std::string_view(link_.linkName, link_.linkNameLength));
	if(!nameView.isValid() || nameView.isNull())
	{
		link_.index = ResourceHandleBase::InvalidIndex;
		link_.managerIndex = 0;
		link_.generation = 0;
		return;
	}

	uint64_t index = 0;
	if(nameView.isCurrentLink())
	{
		ResourceName name(current_.getStorage(), current_.getName(), nameView.getSubObject());
		index = getIndexFromName(link_.id, name.getView());
	} else
	{
		index = getIndexFromName(link_.id, nameView);
	}

	link_.index = index;
	link_.managerIndex = managerIndex;
	link_.generation = indexToBase.at(index).generation;
}


} // end namespace