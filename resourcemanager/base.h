#pragma once
#ifndef WYRD_RESOURCEMAN_BASE_H
#define WYRD_RESOURCEMAN_BASE_H

#include "core/core.h"
#include "tbb/concurrent_hash_map.h"
#include "cityhash/city.h"

/// used to identify resources, each should be unique to a project etc. IFF like
constexpr uint32_t operator"" _resource_id(char const* s, size_t count)
{
	assert(count==4);
	return s[3] << 24 | s[2] << 16 | s[1] << 8 | s[0] << 0;
}

namespace ResourceManager
{

struct ResourceBase
{
	using Ptr = std::shared_ptr<ResourceBase>;
	using ConstPtr = std::shared_ptr<ResourceBase const>;
	using WeakPtr = std::weak_ptr<ResourceBase>;
	using ConstWeakPtr = std::weak_ptr<ResourceBase>;
};

// resources and handles have a type along with there name to determine how they
// should be used and processed

// a resource is the actual memory associated with a particular named resource
template<uint32_t type_>
class Resource : public ResourceBase
{
public:
	using Ptr = std::shared_ptr<Resource<type_>>;
	using ConstPtr = std::shared_ptr<Resource<type_> const>;
	using WeakPtr = std::weak_ptr<Resource<type_>>;
	using ConstWeakPtr = std::weak_ptr<Resource<type_> const>;

	// note: this memory only exist if the resource handler asked for it
	// so only use this if you know what the resource handler did
	void* getExtraMemPtr(uint32_t stage_) const { return (void*) *(((uintptr_t*)this)-stage_); }

protected:
	Resource() = default;
	~Resource() = default;
	// Implementation note: we assume that the compiler is doing the empty object
	// optimization, as we treat Resource as using no allocation space when classes
	// derive from it.
};

// a resource handle is an opaque handle linked to a particular resource

// the base class holds the basic elements for the typed handles that are actually used
struct ResourceHandleBase
{

	friend class ResourceMan;
	template<uint32_t type_> friend class ResourceHandle;

	uint64_t id;
	uint32_t type;
	uint16_t managerIndex;
	uint16_t generation;

protected:
	auto acquire() -> ResourceBase::Ptr;
	auto tryAcquire() -> ResourceBase::Ptr;
};

template<uint32_t type_>
class ResourceHandle
{
public:
	friend class ResourceMan;
	virtual ~ResourceHandle() = default;

	auto acquire() -> typename Resource<type_>::Ptr {
		return std::static_pointer_cast<Resource<type_>>(base.acquire());
	}

	auto tryAcquire() -> typename Resource<type_>::Ptr {
		return std::static_pointer_cast<Resource<type_>>(base.tryAcquire());
	}

	template<typename T> auto acquire() -> typename std::shared_ptr<T> {
		static_assert(T::Id == type_);
		return std::static_pointer_cast<T>(base.acquire());
	}

	template<typename T> auto tryAcquire() -> typename std::shared_ptr<T> {
		static_assert(T::Id == type_);
		return std::static_pointer_cast<T>(base.tryAcquire());
	}

protected:
	explicit ResourceHandle(ResourceHandleBase base_) : base(base_) {}
	ResourceHandleBase base;
};



} // end namespace
#endif //WYRD_RESOURCEMAN_BASE_H
