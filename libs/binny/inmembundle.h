#pragma once
#ifndef BINNY_INMEMBUNDLE_H
#define BINNY_INMEMBUNDLE_H

#include "core/core.h"
#include "core/utils.h"
#include <string>
#include <vector>
#include <functional>
#include "binny/ibundle.h"
#include <unordered_map>
#include <array>

namespace Binny {

// an in memory bundle takes a existing 'chunk' and presents it same as if
// loaded via a normal Bundle. Allows a single handler for in memory and on disk
// data
class InMemBundle : public IBundle
{
public:
	InMemBundle(AllocFunc alloc_,
				FreeFunc free_,
				uint32_t type_,
				uint16_t majorVersion_,
				uint16_t minorVersion_,
				void const* data_,
				size_t size_) :
		allocFunc(alloc_), freeFunc(free), type(type_),
		majorVersion(majorVersion_), minorVersion(minorVersion_),
		data(data_), size(size_) {}

	uint32_t getDirectoryCount() override { return 1;};
	std::string_view getDirectoryEntry(uint32_t const index_) override { return "0"; }

	std::pair<ErrorCode, uint64_t> read(std::string_view name_, std::vector<ChunkHandler> const& handlers_, bool forcePrefix_ ) override
	{
		if(!name_.empty() && name_ != "0") return { ErrorCode::NotFound, 0};

		auto orderedHandlers = Core::make_array<MaxHandlerStages>(-1);

		int totalExtraMem = 0;
		for(int j = 0; j < handlers_.size(); ++j)
		{
			uint32_t htype;
			int stage;
			int extramem;
			std::tie( htype, stage, extramem, std::ignore, std::ignore) = handlers_.at(j);
			if(type == htype)
			{
				orderedHandlers[stage] = j;
				totalExtraMem = extramem;
			}
		}

		uint8_t * basePtr;
		uint8_t * dataPtr;

		// allocate and copy to account
		size_t const prefix = (totalExtraMem == 0 && !forcePrefix_) ? 0 : sizeof(uintptr_t) * MaxHandlerStages;
		size_t const totalSize = size + totalExtraMem + prefix;
		basePtr = (uint8_t*) allocFunc(totalSize);
		if(prefix > 0)
		{
			std::memset(basePtr, 0x0DE, prefix);
		}
		dataPtr = ((uint8_t*)basePtr) + prefix;
		std::memcpy(dataPtr, data, size);

		std::vector<std::pair<int,ChunkDestroyFunc>> destroyers;
		destroyers.reserve(MaxHandlerStages);
		// reverse order for destruction
		for(int j = MaxHandlerStages-1; j >= 0; --j)
		{
			auto const handlerIndex = orderedHandlers.at( j );
			if(handlerIndex >= 0)
			{
				ChunkDestroyFunc destroyer = std::get<4>(handlers_.at(handlerIndex));
				destroyers.push_back({j, destroyer});
			}
		}
		auto localFree = freeFunc;
		auto ptr = std::shared_ptr<void>((void *) dataPtr,
				 [localFree,destroyers, prefix]( void *ptr )
				 {
					 for(auto const [stage, destroyer] : destroyers)
					 {
						 destroyer(stage, ptr);
					 }
					 localFree( ((uint8_t*)ptr)-prefix );
				 } );

		dataPtr += size;
		for(size_t i = 0u; i < handlers_.size(); ++i)
		{
			if(std::get<0>(handlers_.at(i)) == type)
			{
				bool okay = true;
				for(int j = 0; j < MaxHandlerStages; ++j)
				{
					if(orderedHandlers[j] != -1)
					{
						uint32_t htype;
						int stage;
						int extramem;
						ChunkCreateFunc func;
						std::tie( htype, stage, extramem, func, std::ignore ) = handlers_.at(orderedHandlers[j]);
						assert(stage == j);
						if(extramem > 0)
						{
							std::memset(dataPtr, 0xB0 | stage, extramem);
							((uintptr_t *) basePtr)[MaxHandlerStages - 1 - j] = (uintptr_t) dataPtr;
							dataPtr += extramem;
						}
						okay |= func(name_, stage, minorVersion, majorVersion, ptr);
					}
				}
				if(okay) return { ErrorCode::Okay, 0 };
				else return {ErrorCode ::OtherError, 0 };
			}
		}

		return {ErrorCode::OtherError, 0 };
	}
protected:
	AllocFunc allocFunc;
	FreeFunc freeFunc;
	uint32_t const type;
	uint16_t const majorVersion;
	uint16_t const minorVersion;
	void const* data;
	size_t const size;
};

}

#endif //BINNY_INMEMBUNDLE_H
