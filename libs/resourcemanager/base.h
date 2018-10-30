#pragma once
#ifndef WYRD_RESOURCEMANANAGER_BASE_H
#define WYRD_RESOURCEMANANAGER_BASE_H

#include "core/core.h"
#include "tbb/concurrent_hash_map.h"
#include "cityhash/city.h"

/// used to identify resources, each should be unique to a project etc. IFF like
constexpr uint32_t operator"" _resource_id(char const* s, size_t count)
{
	assert(count<=4 && count > 0);
	uint32_t res =
			(s[0] << 0) |
			((count > 1 ? s[1] : uint32_t('_')) << 8) |
			((count > 2 ? s[2] : uint32_t('_')) << 16) |
			((count > 3 ? s[3] : uint32_t('_')) << 24);
	return res;
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
template<uint32_t id_>
class Resource : public ResourceBase
{
public:
	using Ptr = std::shared_ptr<Resource<id_>>;
	using ConstPtr = std::shared_ptr<Resource<id_> const>;
	using WeakPtr = std::weak_ptr<Resource<id_>>;
	using ConstWeakPtr = std::weak_ptr<Resource<id_> const>;
	static constexpr uint32_t Id = id_;

	// note: this memory only exist if the resource handler asked for it
	// so only use this if you know what the resource handler did
	template<typename T>
	T* getStage(uint32_t stage_) const
	{
		uintptr_t* ptr = (((uintptr_t*)this)-(stage_+1));
		if constexpr (sizeof(uintptr_t) == 4)
		{
			assert(*ptr != 0xDEDEDEDE);
		} else if constexpr (sizeof(uintptr_t) == 8)
		{
			assert(*ptr != 0xDEDEDEDEDEDEDEDE);
		} else
		{
			uint8_t* bytePtr = (uint8_t*)ptr;
			uintptr_t count = 0;
			for(auto i = 0u; i < sizeof( uintptr_t ); ++i)
			{
				count += (*bytePtr == 0xDE);
			}
			assert(count != sizeof(uintptr_t));
		}
		return (T*) *ptr;
	}

protected:
	Resource() = default;
	~Resource() = default;
	// Implementation note: we assume that the compiler is doing the empty object
	// optimization, as we treat Resource as using no allocation space when classes
	// derive from it.
};

// a resource handle is an opaque handle linked to a particular resource

// the base class holds the basic elements for the typed handles that are actually used
// the linkXXX unions are whats stored on disk and resolved by the resource manager
struct ResourceHandleBase
{

	friend class ResourceMan;
	template<uint32_t id_> friend class ResourceHandle;
	static constexpr uint64_t InvalidIndex = ~0;

	union
	{
		uint64_t index = InvalidIndex;
		char const* linkName;
	};
	uint32_t type;
	union
	{
		uint16_t managerIndex;
		uint16_t linkNameLength;
	};
	uint16_t generation;

protected:
	auto acquire() -> ResourceBase::Ptr;
	auto tryAcquire() -> ResourceBase::Ptr;
};
static_assert(sizeof(ResourceHandleBase)== 16);

template<uint32_t id_>
class ResourceHandle
{
public:
	friend class ResourceMan;
	virtual ~ResourceHandle() = default;

	auto acquire() -> typename Resource<id_>::Ptr {
		return std::static_pointer_cast<Resource<id_>>(base.acquire());
	}

	auto tryAcquire() -> typename Resource<id_>::Ptr {
		return std::static_pointer_cast<Resource<id_>>(base.tryAcquire());
	}

	template<typename T> auto acquire() -> typename std::shared_ptr<T> {
		static_assert(T::Id == id_);
		return std::static_pointer_cast<T>(base.acquire());
	}

	template<typename T> auto tryAcquire() -> typename std::shared_ptr<T> {
		static_assert(T::Id == id_);
		return std::static_pointer_cast<T>(base.tryAcquire());
	}

	explicit ResourceHandle(ResourceHandleBase base_) : base(base_) {}
	ResourceHandleBase base;
};


} // end namespace
#endif //WYRD_RESOURCEMAN_BASE_H
