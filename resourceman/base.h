#pragma once
#ifndef WYRD_RESOURCEMAN_BASE_H
#define WYRD_RESOURCEMAN_BASE_H

#include "core/core.h"

namespace ResourceMan
{

struct ResourceBase {};

// a resource is the actual memory associated with a particular named resource
template<uint32_t type_>
class Resource : ResourceBase
{
public:
	using Ptr = std::shared_ptr<Resource<type_>>;
	using ConstPtr = std::shared_ptr<Resource<type_> const>;
	using WeakPtr = std::weak_ptr<Resource<type_>>;
	using ConstWeakPtr = std::weak_ptr<Resource<type_> const>;

protected:
	Resource(){}
	~Resource(){}
};

// a resource handle is an opaque handle linked to a particular resource

// the base class holds the basic elements for the typed handles that are actually used
class ResourceHandleBase {
public:
	friend class ResourceMan;
	template<uint32_t type_> friend class ResourceHandle;

	uint64_t getId() const { return id; }
	uint32_t getType() const { return type; }

protected:
	template<uint32_t type_> std::shared_ptr<Resource<type_> const> acquire() const;
	template<uint32_t type_> std::shared_ptr<Resource<type_> const> tryAcquire() const;

	uint64_t id;
	uint32_t type;
	uint16_t managerIndex;
	uint16_t generation;
};

template<uint32_t type_>
class ResourceHandle
{
public:
	friend class ResourceMan;

	typename Resource<type_>::Ptr acquire() {
		return std::static_pointer_cast<type_>(base.acquire<type_>());
	}

	typename Resource<type_>::Ptr tryAcquire() {
		return std::static_pointer_cast<type_>(base.tryAcquire<type_>());
	}

protected:
	ResourceHandle() { base.type = type_; }
	~ResourceHandle() {};
	ResourceHandleBase base;
};



}

#endif //WYRD_RESOURCEMAN_BASE_H
