#pragma once
#ifndef WYRD_RESOURCEMANANAGER_RESOURCE_H
#define WYRD_RESOURCEMANANAGER_RESOURCE_H

#include "core/core.h"
#include "binny/ibundle.h"

namespace ResourceManager
{
class ResourceMan;

enum class ResourceId : uint32_t;

struct ResourceBase
{
	using Ptr = std::shared_ptr<ResourceBase>;
	using ConstPtr = std::shared_ptr<ResourceBase const>;
	using WeakPtr = std::weak_ptr<ResourceBase>;
	using ConstWeakPtr = std::weak_ptr<ResourceBase>;

	static constexpr uint32_t MaxStages = Binny::IBundle::MaxHandlerStages;

	uint8_t getStageCount() const { return stage0 & 0x3; }

	uintptr_t getSize() const { return stage0 & ~0x3; }

	// note: this memory only exist if the resource handler asked for it
	// so only use this if you know what the resource handler did
	template<typename T>
	T* getStage(uint32_t stage_) const
	{
		assert(stage_ != 0);
		uintptr_t ptr = stages[stage_ - 1];
		if constexpr (sizeof(uintptr_t) == 4)
		{
			assert(ptr != 0xDEDEDEDE);
		} else if constexpr (sizeof(uintptr_t) == 8)
		{
			assert(ptr != 0xDEDEDEDEDEDEDEDE);
		} else
		{
			uint8_t* bytePtr = (uint8_t*) &ptr;
			uintptr_t count = 0;
			for(auto i = 0u; i < sizeof( uintptr_t ); ++i)
			{
				count += (*bytePtr == 0xDE);
			}
			assert(count != sizeof(uintptr_t));
		}
		return (T*) ptr;
	}


	// this is a misnomer, stage0 can't have extramem so use reuse its
	// pointer space for size and stageCount
	uintptr_t stage0;
	uintptr_t stages[MaxStages - 1];
};

// resources and handles have a type along with there name to determine how they
// should be used and processed

// a resource is the actual memory associated with a particular named resource
// this relys on the 0 size class optimization, as its just a helper for
// ResourceBase and must not have any storage
template<ResourceId id_>
struct Resource : public ResourceBase
{
public:
	using Ptr = std::shared_ptr<Resource<id_>>;
	using ConstPtr = std::shared_ptr<Resource<id_> const>;
	using WeakPtr = std::weak_ptr<Resource<id_>>;
	using ConstWeakPtr = std::weak_ptr<Resource<id_> const>;
	static constexpr ResourceId Id = id_;

protected:
	Resource() = default;
	~Resource() = default;
};


} // end namespace
#endif //WYRD_RESOURCEMAN_RESOURCE_H
