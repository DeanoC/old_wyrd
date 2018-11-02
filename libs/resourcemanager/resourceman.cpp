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

ResourceMan::ResourceMan() {}

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

void ResourceMan::registerResourceHandler(uint32_t type_, ResourceHandler handler_, HasResourceChangedFunc changed_,
										  SaveResourceFunc save_)
{
	if(typeToHandler.find(type_) != typeToHandler.end())
	{
		assert(std::get<1>(typeToHandler[type_][0]) == nullptr);
	}

	typeToHandler[type_][0] = handler_;
	typeToSavers[type_] = {changed_, save_};
}

auto ResourceMan::registerNextResourceHandler(uint32_t type_, ResourceHandler handler_) -> int
{
	assert(typeToHandler.find(type_) != typeToHandler.end());
	assert(std::get<1>(typeToHandler[type_][0]) != nullptr);

	auto handlers = typeToHandler[type_];
	for(int i = 1; i < MaxHandlerStages; ++i)
	{
		int extramem = 0;
		HandlerInit init;
		std::tie(std::ignore, init, std::ignore) = handlers[i];
		if(init != nullptr) continue;
		typeToHandler[type_][i] = handler_;
		return i;
	}
	return -1;
}

auto ResourceMan::removeResourceHandler(uint32_t type_, int stage_) -> void
{
	assert(typeToHandler.find(type_) != typeToHandler.end());
	if(stage_ == 0)
	{
		for(int i = 0; i < MaxHandlerStages; ++i)
		{
			typeToHandler[type_][i] = {0, nullptr, nullptr};
		}
	} else
	{
		typeToHandler[type_][stage_] = {0, nullptr, nullptr};
	}
}

auto ResourceMan::getIndexFromName(uint32_t type_, ResourceNameView const name_) -> uint64_t
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

			indexToBase[id] = ResourceHandleBase{id, type_, managerIndex, (uint16_t) 0};
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

auto ResourceMan::openBaseResourceByTypeAndName(uint32_t type_, ResourceNameView const name_) -> ResourceHandleBase&
{
	uint64_t id = getIndexFromName(type_, name_);
	return indexToBase[id];
}

auto ResourceHandleBase::acquire() -> ResourceBase::Ptr
{
	auto resourceMan = ResourceMan::GetFromIndex(managerIndex);
	return resourceMan->acquire(*this);
}

auto ResourceHandleBase::tryAcquire() -> ResourceBase::Ptr
{
	auto resourceMan = ResourceMan::GetFromIndex(managerIndex);
	return resourceMan->tryAcquire(*this);
}

auto ResourceMan::acquire(ResourceHandleBase const& base_) -> ResourceBase::Ptr
{
	if(base_.index == ResourceHandleBase::InvalidIndex) return ResourceBase::Ptr();

	assert(typeToHandler.find(base_.type) != typeToHandler.end());
	ResourceBase::Ptr ptr;
	while(!ptr)
	{
		ptr = tryAcquire(base_);
	}
	return ptr;
}

auto ResourceMan::tryAcquire(ResourceHandleBase const& base_) -> ResourceBase::Ptr
{
	if(base_.index == ResourceHandleBase::InvalidIndex) return ResourceBase::Ptr();

	ResourceBase::Ptr cached = resourceCache.lookup(base_.index);
	if(cached) return cached;

	assert(typeToHandler.find(base_.type) != typeToHandler.end());

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

	ResolverInterface resolver = {[this]() -> ResourceMan*
								  { return this; }, [this, resourceName](ResourceHandleBase& base_) -> void
								  {
									  this->resolveLink(base_, resourceName);
								  }};

	ChunkHandlers chunks;
	chunks.reserve(typeToHandler.size());

	for(auto const&[type, orderedHandler] : typeToHandler)
	{
		auto lambdaType = type;

		for(int stage = 0; stage < MaxHandlerStages; ++stage)
		{
			uint32_t extramem = 0;
			HandlerInit init;
			HandlerDestroy destroy;
			std::tie(extramem, init, destroy) = orderedHandler[stage];
			if(init == nullptr) continue;

			auto createFun = [this, lambdaType, resolver, prefix, name, init]
					(std::string_view subObject_, int stage_,
					 uint16_t majorVersion_, uint16_t minorVersion_,
					 std::shared_ptr<void> ptr_) -> bool
			{
				uint64_t id;
				ResourceName newName(prefix, name, subObject_);
				id = getIndexFromName(lambdaType, newName.getResourceName());

				auto ptr = std::static_pointer_cast<ResourceBase>(ptr_);

				bool okay = init(stage_, resolver, majorVersion_, minorVersion_, ptr);
				if(okay)
				{
					resourceCache.insert(id, ptr);
					return true;
				} else
					return false;
			};

			chunks.emplace_back(
					Binny::IBundle::ChunkHandler{type, stage, extramem, createFun, destroy, true, false}
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
	if(!link_.isValid())
	{
		link_.index = ResourceHandleBase::InvalidIndex;
		link_.managerIndex = 0;
		link_.generation = 0;
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
		index = getIndexFromName(link_.type, name.getView());
	} else
	{
		index = getIndexFromName(link_.type, nameView);
	}

	link_.index = index;
	link_.managerIndex = managerIndex;
	link_.generation = indexToBase.at(index).generation;
}


} // end namespace