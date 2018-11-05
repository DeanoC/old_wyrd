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
#include "tbb/concurrent_unordered_map.h"
#include "istorage.h"
#include <iostream>
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
using ResolveLinkFunc = std::function<void(ResourceHandleBase&)>;
using ResolverInterface = std::tuple<ResolveGetResourceMan, ResolveLinkFunc>;
using HandlerInit = std::function<bool(int stage_, ResolverInterface resolver_, uint16_t majorVersion_, uint16_t minorVersion_, ResourceBase::Ptr ptr_ )>;
using HandlerDestroy = std::function<bool(int stage_, void* ptr_)>;

using HasResourceChangedFunc = std::function<bool(ResourceBase::ConstPtr ptr_)>;
using SaveResourceFunc = std::function<bool(ResourceBase::ConstPtr ptr_, ISaver& saver_)>;

// extra memory, init, destroy
using ResourceHandler = std::tuple<int, HandlerInit, HandlerDestroy>;

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
	auto placeInStorage(ResourceManager::ResourceNameView name_, T const& renderPass_) -> bool;

	auto registerResourceHandler( 	uint32_t type_,
									ResourceHandler funcs_,
									HasResourceChangedFunc changed_ = nullptr,
									SaveResourceFunc save_ = nullptr) -> void;
	auto registerNextResourceHandler( 	uint32_t type_,
										ResourceHandler funcs_ ) -> int;
	auto removeResourceHandler(uint32_t type_, int stage_) -> void;

	template<uint32_t type_>
	auto openResourceById( uint64_t const id_ ) -> ResourceHandle<type_>
	{
		auto resourceHandleBase = indexToBase[id_];
		assert( type_ == resourceHandleBase.type );
		return ResourceHandle<type_>( resourceHandleBase );
	}

	template<uint32_t type_>
	auto openResourceByName( ResourceNameView const name_ ) -> ResourceHandle<type_>
	{
		uint64_t id = getIndexFromName(type_, name_);
		return openResourceById<type_>( id );
	}

	auto getIndexFromName(uint32_t type_, ResourceNameView const name_) -> uint64_t;

protected:
	ResourceMan();

	auto openBaseResourceByTypeAndName( uint32_t type_, ResourceNameView const name_ ) -> ResourceHandleBase&;
	auto acquire( ResourceHandleBase const& base_ ) -> ResourceBase::Ptr;
	auto tryAcquire( ResourceHandleBase const& base_ ) -> ResourceBase::Ptr;
	auto resolveLink(ResourceHandleBase& link_, ResourceNameView const& current_) -> void;


	using PrefixToStorage = std::unordered_map<std::string_view, IStorage::Ptr>;
	using ResourceNameToIndex = tbb::concurrent_unordered_map<ResourceNameView, uint64_t>;
	using IndexToResourceName = tbb::concurrent_unordered_map<uint64_t, ResourceName>;
	using IndexToBase = Core::MTFreeList<ResourceHandleBase, uint64_t>;
	using TypeToHandler = tbb::concurrent_unordered_map<uint32_t, std::array<ResourceHandler,MaxHandlerStages>>;
	using TypeToSavers = tbb::concurrent_unordered_map<uint32_t, std::pair<HasResourceChangedFunc,SaveResourceFunc>>;
	using ChunkHandlers = std::vector<IStorage::ChunkHandler>;

	PrefixToStorage prefixToStorage;
	TypeToHandler typeToHandler;
	TypeToSavers typeToSavers;

	IndexToResourceName indexToResourceName;
	ResourceNameToIndex nameToResourceIndex;
	IndexToBase indexToBase;
	ResourceCache resourceCache;

	std::mutex nameCacheLock;

	uint16_t managerIndex;

	static std::string_view const DeletedString;
};

template<typename T>
auto ResourceMan::placeInStorage(ResourceManager::ResourceNameView name_, T const& resource_) -> bool
{
	using namespace std::string_view_literals;

	auto storage = getStorageForPrefix(name_.getStorage());
	assert(storage);
	switch(Core::QuickHash(name_.getStorage()))
	{
		case Core::QuickHash("mem"sv):
		{
			auto memstorage = std::static_pointer_cast<ResourceManager::MemStorage>(storage);
			return memstorage->addMemory(
					std::string(name_.getName()),
					T::Id, T::MajorVersion, T::MinorVersion, &resource_, sizeof(T));
			break;
		}
		default:
			LOG_S(ERROR) << "Unknown storage type for PlaceInStore";
			return false;
	}
}

} // end namespace

#endif //WYRD_RESOURCEMAN_RESOURCEMAN_H
