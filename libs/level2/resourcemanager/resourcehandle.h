#pragma once
#ifndef WYRD_RESOURCEMANANAGER_RESOURCEHANDLE_H
#define WYRD_RESOURCEMANANAGER_RESOURCEHANDLE_H

#include "core/core.h"

namespace ResourceManager {
class ResourceMan;

enum class ResourceId : uint32_t;

struct ResourceBase;
template<ResourceId id_>
struct Resource;

// a resource handle is an opaque handle linked to a particular resource

// the base class holds the basic elements for the typed handles that are actually used
// the linkXXX unions are whats stored on disk and resolved by the resource manager
struct ResourceHandleBase
{
	friend class ResourceMan;

	template<ResourceId id_> friend
	struct ResourceHandle;
	static constexpr uint64_t InvalidIndex = ~0;

	auto isValid() const { return index != InvalidIndex; }

	union
	{
		uint64_t index = InvalidIndex;
		char const* linkName;
	};
	ResourceId id;
	union
	{
		uint16_t managerIndex;
		uint16_t linkNameLength;
	};
	uint16_t generation;

protected:
	auto tryAcquire() const -> std::shared_ptr<ResourceBase const>;
	auto acquire() const -> std::shared_ptr<ResourceBase const>;
};

static_assert(sizeof(ResourceHandleBase) == 16);

template<ResourceId id_>
struct ResourceHandle
{
public:
	friend class ResourceMan;

	auto isValid() const { return base.isValid(); }

	auto acquire() const -> typename Resource<id_>::ConstPtr
	{
		return std::static_pointer_cast<Resource<id_> const>(base.acquire());
	}

	auto tryAcquire() const -> typename Resource<id_>::ConstPtr
	{
		return std::static_pointer_cast<Resource<id_> const>(base.tryAcquire());
	}

	template<typename T>
	auto acquire() const -> typename std::shared_ptr<T const>
	{
		static_assert(T::Id == id_, "Ptr is of different type from the handle");
		return std::static_pointer_cast<T const>(base.acquire());
	}

	template<typename T>
	auto tryAcquire() const -> typename std::shared_ptr<T const>
	{
		static_assert(T::Id == id_, "Ptr is of different type from the handle");
		return std::static_pointer_cast<T const>(base.tryAcquire());
	}

	template<typename T>
	auto mutableAcquire() const -> typename std::shared_ptr<T>
	{
		static_assert(T::Id == id_, "Ptr is of different type from the handle");
		return std::static_pointer_cast<T>(base.acquire());
	}

	template<typename T>
	auto mutableTryAcquire() const -> typename std::shared_ptr<T>
	{
		static_assert(T::Id == id_, "Ptr is of different type from the handle");
		return std::static_pointer_cast<T>(base.tryAcquire());
	}
	ResourceHandle() : base() {}

	explicit ResourceHandle(ResourceHandleBase base_) : base(base_) {}

	ResourceHandleBase base;
};


} // end namespace
#endif //WYRD_RESOURCEMAN_RESOURCE_H
