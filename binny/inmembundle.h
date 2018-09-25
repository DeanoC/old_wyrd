#pragma once
#ifndef BINNY_INMEMBUNDLE_H
#define BINNY_INMEMBUNDLE_H

#include "core/core.h"
#include <string>
#include <vector>
#include <functional>
#include "binny/ibundle.h"

namespace Binny {

// an in memory bundle takes a existing 'chunk' and presents it same as if
// loaded via a normal Bundle. Allows a single handler for in memory and on disk
// data
// note: doesn't take ownership of the memory!
class InMemBundle : public IBundle
{
public:
	InMemBundle(uint32_t type_, uint16_t majorVersion_, uint16_t minorVersion_, void* data_) :
		type(type_), majorVersion(majorVersion_), minorVersion(minorVersion_), data(data_) {}

	uint32_t getDirectoryCount() override { return 1;};
	std::string_view getDirectionEntry(uint32_t const index_) override { return "0"; }

	std::pair<ErrorCode, uint64_t> read(std::string_view name_, size_t handlerCount, chunkHandler const *const handlers ) override
	{
		if(!name_.empty() && name_ != "0") return { ErrorCode::NotFound, 0};

		for(size_t i = 0u; i < handlerCount; ++i)
		{
			if(std::get<0>(handlers[i]) == type)
			{
				auto func = std::get<1>(handlers[i]);
				auto ptr = std::shared_ptr<void>(data, [](void* ptr) {});

				bool okay = func(name_, minorVersion, majorVersion, ptr);
				if(okay) return { ErrorCode::Okay, 0 };
				else return {ErrorCode ::OtherError, 0 };
			}
		}
		return {ErrorCode::OtherError, 0 };
	}
protected:
	uint32_t type;
	uint16_t majorVersion;
	uint16_t minorVersion;
	void* data;
};

}

#endif //BINNY_INMEMBUNDLE_H
