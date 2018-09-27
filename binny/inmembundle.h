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
	InMemBundle(uint32_t type_, uint16_t majorVersion_, uint16_t minorVersion_, void const* data_, size_t size_) :
		type(type_), majorVersion(majorVersion_), minorVersion(minorVersion_), data(data_), size(size_) {}

	uint32_t getDirectoryCount() override { return 1;};
	std::string_view getDirectionEntry(uint32_t const index_) override { return "0"; }

	std::pair<ErrorCode, uint64_t> read(std::string_view name_, std::vector<ChunkHandler> const& handlers_, bool forcePrefix_ ) override
	{
		if(!name_.empty() && name_ != "0") return { ErrorCode::NotFound, 0};

		auto orderedHandlers = Core::make_array<MaxHandlerStages>(-1);

		int totalExtraMem = 0;
		for(int j = 0; j < handlers_.size(); ++j)
		{
			uint32_t htype;
			int order;
			int extramem;
			ChunkFunc handler;
			std::tie( htype, order, extramem, handler ) = handlers_.at(j);
			if(type == htype)
			{
				orderedHandlers[order] = j;
				totalExtraMem = extramem;
			}
		}

		std::shared_ptr<void> ptr;
		uint8_t * basePtr;
		uint8_t * dataPtr;


		// allocate and copy to account
		size_t const prefix = (totalExtraMem == 0 && !forcePrefix_) ? 0 : sizeof(uintptr_t) * MaxHandlerStages;
		size_t const totalSize = size + totalExtraMem + prefix;
		basePtr = (uint8_t*) malloc(totalSize);
		std::memset(basePtr, 0, prefix);
		dataPtr = ((uint8_t*)basePtr) + prefix;
		std::memcpy(dataPtr, data, size);

		ptr = std::shared_ptr<void>(dataPtr, [prefix](void* ptr) {
			free(((uint8_t*)ptr) - prefix);
		});

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
						int order;
						int extramem;
						ChunkFunc func;
						std::tie( htype, order, extramem, func ) = handlers_.at(orderedHandlers[j]);
						if(extramem > 0)
						{
							((uintptr_t *) basePtr)[j] = (uintptr_t) dataPtr;
							dataPtr += extramem;
						}
						okay |= func(name_, minorVersion, majorVersion, ptr);
					}
				}
				if(okay) return { ErrorCode::Okay, 0 };
				else return {ErrorCode ::OtherError, 0 };
			}
		}

		return {ErrorCode::OtherError, 0 };
	}
protected:
	uint32_t const type;
	uint16_t const majorVersion;
	uint16_t const minorVersion;
	void const* data;
	size_t const size;
};

}

#endif //BINNY_INMEMBUNDLE_H
