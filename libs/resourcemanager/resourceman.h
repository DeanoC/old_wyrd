#pragma once
#ifndef WYRD_RESOURCEMAN_RESOURCEMAN_H
#define WYRD_RESOURCEMAN_RESOURCEMAN_H

#include "core/core.h"
#include "binny/bundle.h"
#include "core/freelist.h"
#include "resourcemanager/base.h"
#include "resourcemanager/resourcename.h"
#include "resourcemanager/resourcecache.h"
#include "tbb/concurrent_unordered_map.h"
#include "istorage.h"
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <functional>

namespace Binny
{
class WriteHelper;
}

namespace ResourceManager {

struct ISaver
{
	virtual auto suggestedName() -> std::string_view = 0;

	virtual auto setName(std::string_view name_) -> void = 0;
	virtual auto setMajorVersion(uint16_t majorVersion_) -> void = 0;
	virtual auto setMinorVersion(uint16_t minorVersion_) -> void = 0;
	virtual auto addDependency(uint32_t dependency_) -> void = 0;
	virtual auto setWriterFunction(std::function<void( Binny::WriteHelper& helper )>) -> void = 0;
};

using HandlerInit = std::function<bool(int stage_, uint16_t majorVersion_, uint16_t minorVersion_, ResourceBase::Ptr ptr_ )>;
using HandlerDestroy = std::function<bool(int stage_, void* ptr_)>;

using HasResourceChangedFunc = std::function<bool(ResourceBase::ConstPtr ptr_)>;
using SaveResourceFunc = std::function<bool(ResourceBase::ConstPtr ptr_, ISaver& saver_)>;

// extra memory, init, destroy
using ResourceHandler = std::tuple<int, HandlerInit, HandlerDestroy>;

class ResourceMan
{
public:
	friend class ResourceHandleBase;

	constexpr static unsigned int MaxHandlerStages = Binny::IBundle::MaxHandlerStages;

	static auto Create() -> std::shared_ptr<ResourceMan>;

	static auto GetFromIndex( uint32_t index ) -> std::shared_ptr<ResourceMan>;

	~ResourceMan();

	auto registerStorageHandler( IStorage::Ptr storage_ ) -> void;

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
		auto resourceHandleBase = idToBase[id_];
		assert( type_ == resourceHandleBase.type );
		return ResourceHandle<type_>( resourceHandleBase );
	}

	template<uint32_t type_>
	auto openResourceByName( ResourceNameView const name_ ) -> ResourceHandle<type_>
	{
		uint64_t id = getIdFromName( type_, name_ );
		return openResourceById<type_>( id );
	}

	auto getIdFromName( uint32_t type_, ResourceNameView const name_ ) -> uint64_t;

protected:
	ResourceMan();

	auto getNameFromHandleBase( ResourceHandleBase const& base_ ) -> std::string_view;

	auto openBaseResourceByTypeAndName( uint32_t type_, ResourceNameView const name_ ) -> ResourceHandleBase&;

	auto acquire( ResourceHandleBase const& base_ ) -> ResourceBase::Ptr;

	auto tryAcquire( ResourceHandleBase const& base_ ) -> ResourceBase::Ptr;

	using PrefixToStorage = std::unordered_map<std::string_view, IStorage::Ptr>;

	using ResourceNameToId = tbb::concurrent_unordered_map<ResourceNameView, uint64_t>;
	using IdToResourceName = tbb::concurrent_unordered_map<uint64_t, ResourceName>;
	using IdToBase = Core::MTFreeList<ResourceHandleBase, uint64_t>;
	using TypeToHandler = tbb::concurrent_unordered_map<uint32_t, std::array<ResourceHandler,MaxHandlerStages>>;
	using TypeToSavers = tbb::concurrent_unordered_map<uint32_t, std::pair<HasResourceChangedFunc,SaveResourceFunc>>;
	using ChunkHandlers = std::vector<IStorage::ChunkHandler>;

	PrefixToStorage prefixToStorage;
	TypeToHandler typeToHandler;
	TypeToSavers typeToSavers;

	IdToResourceName idToResourceName;
	ResourceNameToId nameToResourceId;
	IdToBase idToBase;
	ResourceCache resourceCache;

	std::mutex nameCacheLock;

	uint16_t myIndex;

	static std::string const DeletedString;
};

} // end namespace

#endif //WYRD_RESOURCEMAN_RESOURCEMAN_H
