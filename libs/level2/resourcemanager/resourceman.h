#pragma once
#ifndef WYRD_RESOURCEMANANAGER_RESOURCEMAN_H
#define WYRD_RESOURCEMANANAGER_RESOURCEMAN_H

#include "core/core.h"
#include "binny/bundle.h"
#include "core/freelist.h"
#include "resourcemanager/resourcehandle.h"
#include "resourcemanager/resource.h"
#include "resourcemanager/resourcename.h"
#include "resourcemanager/resourcecache.h"
#include "resourcemanager/memstorage.h"
#include "resourcemanager/istorage.h"
#include "tbb/concurrent_unordered_map.h"
#include "tbb/concurrent_vector.h"
#include <string_view>
#include <unordered_map>
#include <functional>

namespace ResourceManager {
class ResourceMan;
class Writer;

struct ISaver
{
	virtual auto suggestedName() -> std::string_view = 0;

	virtual auto setName(std::string_view name_) -> void = 0;
	virtual auto setMajorVersion(uint16_t majorVersion_) -> void = 0;
	virtual auto setMinorVersion(uint16_t minorVersion_) -> void = 0;
	virtual auto addDependency(uint32_t dependency_) -> void = 0;
	virtual auto setWriterFunction(std::function<void( Writer& writer_ )>) -> void = 0;
};
using ResolveGetResourceMan = std::function<ResourceMan*()>;
using ResolveLinkFunc = std::function<void(ResourceHandleBase const&)>;
using ResolveNameFunc = std::function<ResourceNameView const()>;

using ResolverInterface = std::tuple<ResolveGetResourceMan, ResolveLinkFunc, ResolveNameFunc>;
using HandlerInit = std::function<bool(int stage_, ResolverInterface resolver_, uint16_t majorVersion_,
									   uint16_t minorVersion_, std::shared_ptr<ResourceBase> ptr_)>;
using HandlerDestroy = std::function<bool(int stage_, void* ptr_)>;

using HasResourceChangedFunc = std::function<bool(std::shared_ptr<ResourceBase const> ptr_)>;
using SaveResourceFunc = std::function<bool(std::shared_ptr<ResourceBase const> ptr_, ISaver& saver_)>;

// extra memory, init, destroy
using ResourceHandler = std::tuple<size_t, HandlerInit, HandlerDestroy>;

class ResourceMan
{
public:
	using Ptr = std::shared_ptr<ResourceMan>;
	using WeakPtr = std::weak_ptr<ResourceMan>;

	friend struct ResourceHandleBase;

	constexpr static unsigned int MaxHandlerStages = Binny::IBundle::MaxHandlerStages;

	static auto Create() -> std::shared_ptr<ResourceMan>;


	static auto GetFromIndex( uint32_t index ) -> std::shared_ptr<ResourceMan>;

	static auto GetNameFromHandleBase( ResourceHandleBase const& base_ ) -> ResourceNameView;

	~ResourceMan();

	auto registerStorageHandler( IStorage::Ptr storage_ ) -> void;
	auto getStorageForPrefix(std::string_view prefix_) -> IStorage::Ptr;
	template<typename T>
	auto placeInStorage(ResourceManager::ResourceNameView name_, T const& renderPass_) -> void;
	template<typename T>
	auto placeInStorage(ResourceManager::ResourceNameView name_, std::shared_ptr<T> const& resource_) -> void;
	auto removeFromStorage(ResourceManager::ResourceNameView name_) -> void;

	auto registerHandler(ResourceId id_,
						 ResourceHandler funcs_,
						 HasResourceChangedFunc changed_ = nullptr,
						 SaveResourceFunc save_ = nullptr) -> void;
	auto registerNextHandler(ResourceId id_,
							 ResourceHandler funcs_) -> int;
	auto removeHandler(ResourceId id_, int stage_) -> void;

	auto flushCache() -> void;

	template<ResourceId id_>
	auto openByIndex(uint64_t const index_) -> ResourceHandle<id_>
	{
		auto resourceHandleBase = indexToBase[index_];
		assert(id_ == resourceHandleBase.id);
		return ResourceHandle<id_>(resourceHandleBase);
	}

	template<ResourceId id_>
	auto openByName(ResourceNameView const name_) -> ResourceHandle<id_>
	{
		uint64_t id = getIndexFromName(id_, name_);
		return openByIndex<id_>(id);
	}

	// for single usage, this get the handle and acquires the resource in one shot
	template<typename type_>
	auto acquireByName(ResourceNameView const name_) -> std::shared_ptr<type_ const>
	{
		uint64_t id = getIndexFromName(type_::Id, name_);
		auto handle = openByIndex<type_::Id>(id);
		return handle.template acquire<type_>();
	}

	auto getIndexFromName(ResourceId id_, ResourceNameView const name_) -> uint64_t;

protected:
	ResourceMan();

	auto acquire(ResourceHandleBase const& base_) -> std::shared_ptr<ResourceBase>;
	auto tryAcquire(ResourceHandleBase const& base_) -> std::shared_ptr<ResourceBase>;
	auto resolveLink(ResourceHandleBase& link_, ResourceNameView const& current_) -> void;

	using PrefixToStorage = std::unordered_map<std::string_view, IStorage::Ptr>;
	using ResourceNameToIndex = tbb::concurrent_unordered_map<ResourceNameView, uint64_t>;
	using IndexToResourceName = tbb::concurrent_unordered_map<uint64_t, ResourceName>;
	using IndexToBase = Core::MTFreeList<ResourceHandleBase, uint64_t>;
	using IdToHandler = tbb::concurrent_unordered_map<ResourceId, std::array<ResourceHandler, MaxHandlerStages>>;
	using IdToSavers = tbb::concurrent_unordered_map<ResourceId, std::pair<HasResourceChangedFunc, SaveResourceFunc>>;
	using ChunkHandlers = std::vector<IStorage::ChunkHandler>;

	PrefixToStorage prefixToStorage;
	IdToHandler typeToHandler;
	IdToSavers typeToSavers;

	IndexToResourceName indexToResourceName;
	ResourceNameToIndex nameToResourceIndex;
	IndexToBase indexToBase;
	ResourceCache resourceCache;

	std::mutex nameCacheLock;

	uint16_t managerIndex;

	static std::string_view const DeletedString;
};

template<typename T>
auto ResourceMan::placeInStorage(ResourceManager::ResourceNameView name_, std::shared_ptr<T> const& resource_) -> void
{
	placeInStorage(name_, *resource_.get());
}

template<typename T>
auto ResourceMan::placeInStorage(ResourceManager::ResourceNameView name_, T const& resource_) -> void
{
	using namespace std::string_view_literals;

	auto storage = getStorageForPrefix(name_.getStorage());
	assert(storage);

	switch(Core::QuickHash(name_.getStorage()))
	{
		case Core::QuickHash("mem"sv):
		{
			auto memstorage = std::static_pointer_cast<ResourceManager::MemStorage>(storage);
			memstorage->addMemory(
					std::string(name_.getName()),
					T::Id, T::MajorVersion, T::MinorVersion, &resource_, resource_.getSize());
			return;
		}
		default:
			LOG_S(ERROR) << "Unknown storage type for PlaceInStore";
			return ;
	}
}

} // end namespace

#endif //WYRD_RESOURCEMAN_RESOURCEMAN_H
