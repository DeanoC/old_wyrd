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
				uint32_t id_,
				uint16_t majorVersion_,
				uint16_t minorVersion_,
				void const* data_,
				size_t size_) :
			allocFunc(alloc_), freeFunc(free), id(id_),
			majorVersion(majorVersion_), minorVersion(minorVersion_),
			data(data_), size(size_) {}

	uint32_t getDirectoryCount() override { return 1;};
	std::string_view getDirectoryEntry(uint32_t const index_) override { return "0"; }

	auto read(std::string_view name_, std::vector<ChunkHandler> const& handlers_) -> ReadReturn final
	{
		if(!name_.empty() && name_ != "0") return { ErrorCode::NotFound, 0};

		auto orderedHandlers = Core::make_array<MaxHandlerStages>(-1);

		bool writePrefix = false;
		bool allocatePrefix = false;
		uint32_t totalExtraMem = 0;
		for(auto j = 0u; j < handlers_.size(); ++j)
		{
			ChunkHandler const& handler = handlers_.at(j);
			if(id == handler.id)
			{
				orderedHandlers[handler.stage] = j;
				totalExtraMem = handler.extraMem;
				if(handler.stage == 0)
				{
					writePrefix = handler.writePrefix;
					allocatePrefix = handler.allocatePrefix;
				}
			}
		}

		// allocate and copy to account
		size_t const prefixBlockSize = sizeof(uintptr_t) * MaxHandlerStages;
		size_t const prefix = (allocatePrefix) ? prefixBlockSize : 0;
		size_t const totalSize = Core::alignTo(prefix + size + totalExtraMem, 8);

		uint8_t* const basePtr = (uint8_t*) allocFunc(totalSize);

		std::memcpy(basePtr + prefix, data, size);
		if(writePrefix)
		{
			std::memset(basePtr, 0x0DE, prefixBlockSize);
		}

		std::vector<std::pair<int,ChunkDestroyFunc>> destroyers;
		destroyers.reserve(MaxHandlerStages);
		// reverse order for destruction
		for(int j = MaxHandlerStages-1; j >= 0; --j)
		{
			auto const handlerIndex = orderedHandlers.at( j );
			if(handlerIndex >= 0)
			{
				ChunkHandler const& handler = handlers_.at(handlerIndex);
				destroyers.push_back({j, handler.destroyFunc});
			}
		}

		auto localFree = freeFunc;
		auto ptr = std::shared_ptr<void>((void*) basePtr,
										 [localFree, destroyers](void* ptr)
				 {
					 for(auto const [stage, destroyer] : destroyers)
					 {
						 destroyer(stage, ptr);
					 }
					 localFree(ptr);
				 } );

		uint8_t* extraPtr = basePtr + prefix + size;
		for(size_t i = 0u; i < handlers_.size(); ++i)
		{
			ChunkHandler const& handler = handlers_.at(i);
			if(handler.id == id)
			{
				bool okay = true;
				for(int j = 0; j < MaxHandlerStages; ++j)
				{
					if(orderedHandlers[j] != -1)
					{
						ChunkHandler const& handler = handlers_.at(orderedHandlers[j]);
						assert(handler.stage == j);
						if(writePrefix)
						{
							std::memset(extraPtr, 0xB0 | handler.stage, handler.extraMem);
							((uintptr_t*) basePtr)[j] = (uintptr_t) extraPtr;
						}
						extraPtr += handler.extraMem;
						okay |= handler.createFunc(name_, handler.stage, majorVersion, minorVersion, totalSize, ptr);
						if(okay == false) break;
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
	uint32_t const id;
	uint16_t const majorVersion;
	uint16_t const minorVersion;
	void const* data;
	size_t const size;
};

}

#endif //BINNY_INMEMBUNDLE_H
