//
// Created by Dean Calver on 18/09/2018.
//

#include "core/core.h"
#include "resourceman.h"
#include <array>

namespace ResourceManager {

std::string const ResourceMan::DeletedString =  { "**DELETED**" };

// resource managers are sort of singletons, we keep a static registry to save a full ptr per resource etc.
// intended usage pattern is to store the shared_ptr returned by create for the lifetime of the manager
// and it should be released last thing after all resource are finished with by reseting the unique_ptr
std::vector<std::weak_ptr<ResourceMan>> s_resourceManagers;
int s_curResourceManagerCount = 0; // TODO keep a free list

ResourceMan::ResourceMan() {}
ResourceMan::~ResourceMan()
{
	assert(myIndex < s_curResourceManagerCount);
	// TODO reclaim the index
}

auto ResourceMan::Create() -> std::shared_ptr<ResourceMan>
{
	struct ResourceManCreator : public ResourceManager::ResourceMan {};

	auto resourceMan = std::make_shared<ResourceManCreator>();
	s_resourceManagers.push_back(resourceMan);
	resourceMan->myIndex = s_curResourceManagerCount++;

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

void ResourceMan::registerResourceHandler( uint32_t type_, int stage_, ResourceHandler handler_ )
{
	if(typeToHandler.find(type_) == typeToHandler.end())
	{
		typeToHandler[type_][stage_] = handler_;
	} else
	{
		assert(std::get<0>(typeToHandler[type_][stage_]) == 0);
		typeToHandler[type_][stage_] = handler_;
	}
}

auto ResourceMan::getIdFromName(uint32_t type_,ResourceNameView name_) -> uint64_t
{
	auto it = nameToResourceId.find(name_);
	if(it == nameToResourceId.end())
	{
		std::lock_guard guard(nameCacheLock);
		// first re-check in case some one get here first
		it = nameToResourceId.find(name_);
		if(it == nameToResourceId.end())
		{
			// lets do the allocation
			ResourceName name(name_);
			uint64_t id = idToBase.alloc();
			idToResourceName[id] = name;
			nameToResourceId[name_] = id;

			idToBase[id] = ResourceHandleBase{id, type_, myIndex, (uint16_t)0};
			it = nameToResourceId.find(name_);
		}
	}

	return nameToResourceId[name_];
}

auto ResourceMan::getNameFromHandleBase(ResourceHandleBase const& base_) -> std::string_view
{
	if(idToBase[base_.id].generation != base_.generation) return DeletedString;

	return idToResourceName[base_.id].getName();
}

auto ResourceMan::openBaseResourceByTypeAndName(uint32_t type_, ResourceNameView const name_) -> ResourceHandleBase&
{
	uint64_t id = getIdFromName(type_, name_);
	return idToBase[id];
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
	ResourceBase::Ptr cached = resourceCache.lookup(base_.id);
	if(cached) return cached;

	assert(typeToHandler.find(base_.type) != typeToHandler.end());

	// get storage manager
	assert(idToResourceName.find(base_.id) != idToResourceName.end());
	ResourceNameView resourceName = idToResourceName[base_.id].getView();
	std::string_view prefix = resourceName.getStorage();
	std::string_view name = resourceName.getName();
	std::string_view subObject = resourceName.getSubObject();

	assert(!prefix.empty());
	assert(!name.empty());
	assert(prefixToStorage.find(prefix) != prefixToStorage.end());

	IStorage::Ptr storage = prefixToStorage[prefix];

	ChunkHandlers chunks;
	chunks.reserve(typeToHandler.size());

	for(auto const& [type, orderedHandler] : typeToHandler)
	{
		auto lambdaType = type;

		for(int i = 0; i < MaxHandlerStages; ++i)
		{
			int extramem = 0;
			HandlerInit init;
			std::tie(extramem, init, std::ignore) = orderedHandler[i];
			if(init == nullptr) continue;

			chunks.emplace_back(
					type,
					i,
					extramem,
					[this, lambdaType, prefix, name, init]( std::string_view subObject_, uint16_t majorVersion_, uint16_t minorVersion_, std::shared_ptr<void> ptr_ ) -> bool
					{
						uint64_t id;
						ResourceName newName(prefix, name , subObject_);
						id = getIdFromName(lambdaType, newName.getResourceName());

						auto ptr = std::static_pointer_cast<ResourceBase>(ptr_);

						bool okay = init( majorVersion_, minorVersion_, ptr );
						if(okay)
						{
							resourceCache.insert(id, ptr);
							return true;
						} else
							return false;
					});

		}
	}
	bool okay = storage->read(resourceName, chunks);
	if(okay)
	{
		return resourceCache.lookup(base_.id);
	} else
	{
		return {};
	}

}

} // end namespace