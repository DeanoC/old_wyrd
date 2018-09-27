#pragma once
#ifndef BINNY_IBUNDLE_H
#define BINNY_IBUNDLE_H

#include "core/core.h"
#include <string>
#include <vector>
#include <functional>

namespace Binny {

/// used to identify chunks, each should be unique to a project etc. IFF like
constexpr uint32_t operator"" _bundle_id(char const* s, size_t count)
{
	assert(count==4);
	return s[3] << 24 | s[2] << 16 | s[1] << 8 | s[0] << 0;
}

struct IBundle {
	using Ptr = std::shared_ptr<IBundle>;

	enum class ErrorCode
	{
		Okay = 0,			// no error
		NotFound,			// nothing wanted was found
		AddressLength,		// address length issues probably 64 bit bundle on 32 bit system
		ReadError,			// stream issues
		CorruptError,		// failed internal crc checks
		CompressionError,	// decompression failed
		MemoryError,		// error allocating memory
		OtherError			// generic error
	};
	using AllocFunc = std::function<void*(size_t)>;
	using FreeFunc = std::function<void(void*)>;

	constexpr static unsigned int MaxHandlerStages = 4;

	// chunk name, minor version, major version, smart pointer to the data
	using ChunkFunc = std::function<bool(std::string_view, uint16_t, uint16_t, std::shared_ptr<void>)>;

	/// id, order, extra mem, function to handle this type
	using ChunkHandler = std::tuple<uint32_t, int, uint32_t, ChunkFunc>;

	// the extra mem parameter needs some explaination. Each stage can request
	// some extra meme just for itself, if any do, there will be a pointer block to
	// this amount of extra memory somewhere *BEFORE* the ptr returned to the handlers
	// if any stage request extra mem there will be exactly MaxHandlerStages pointers
	// before the ptr and each stages pointer will be ((uintptr_t*)ptr)[-stage] in
	// location
	// the actual allocation will be one large block and its this block that will be
	// passed the free function

	virtual uint32_t getDirectoryCount() = 0;
	virtual std::string_view getDirectionEntry(uint32_t const index_) = 0;

	/// @param name_ name of chunk wanted, empty to load all given handler types
	/// @param handlers vector of handler to process the chunk of a given type
	/// @param forcePrefix when true even if no extramem is allocated the prefix will be added anyway
	virtual std::pair<ErrorCode, uint64_t> read(std::string_view name_,
												std::vector<ChunkHandler> const& handlers_, bool forcePrefix_ = false) = 0;

};

}
#endif //WYRD_IBUNDLE_H
