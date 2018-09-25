#pragma once
#ifndef WYRD_RESOURCEMAN_BASE_H
#define WYRD_RESOURCEMAN_BASE_H

#include "core/core.h"
#include "tbb/concurrent_hash_map.h"
#include "cityhash/city.h"

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

protected:
	Resource() = default;
	~Resource() = default;
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

protected:
	explicit ResourceHandle(ResourceHandleBase base_) : base(base_) {}
	ResourceHandleBase base;
};



} // end namespace
#endif //WYRD_RESOURCEMAN_BASE_H
