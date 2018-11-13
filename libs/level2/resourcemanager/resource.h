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

	static constexpr uint32_t MaxStages = Binny::IBundle::MaxHandlerStages;

	uint8_t getStageCount() const { return (sizeAndStageCount & 0x3); }

	uintptr_t getSize() const { return sizeAndStageCount & ~0x3; }

	template<typename T, bool dstChk_ = true>
	T* getStage(uint32_t stage_) const
	{
		assert(stage_ != 0);
		uintptr_t ptr = stages[stage_ - 1];
		assert(ptr != 0);

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
		static constexpr uint8_t stageChk[MaxStages]{0xB0, 0xB1, 0xB2, 0xB3};
		if(dstChk_)
		{
			// check a single byte at the destination to see if wrote currently
			assert(*(uint8_t*) ptr != stageChk[stage_]);
		} else
		{
			// check a single byte at the dest to see if its still virgin
			assert(*(uint8_t*) ptr == stageChk[stage_]);
		}
		return (T*) ptr;
	}


	// stage0 can't have extramem in the resource system so use reuse its
	// pointer space for size and stageCount
	uintptr_t sizeAndStageCount;
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
	static constexpr ResourceId Id = id_;

protected:
	Resource() = default;
	~Resource() = default;
};


} // end namespace
#endif //WYRD_RESOURCEMAN_RESOURCE_H
