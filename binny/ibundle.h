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
	using allocFunc = std::function<void*(size_t)>;
	using freeFunc = std::function<void(void*)>;

	// chunk name, minor version, major version, smart pointer to the data
	using chunkFunc = std::function<bool(std::string_view, uint16_t, uint16_t, std::shared_ptr<void>)>;
	using chunkHandler = std::tuple<uint32_t, chunkFunc>; ///< id, function to handle this id

	virtual uint32_t getDirectoryCount() = 0;
	virtual std::string_view getDirectionEntry(uint32_t const index_) = 0;

	/// @param name_ name of chunk wanted, empty to load all given handler types
	/// @param handlers list of handler to process the chunk of a given type
	/// @param handlerCount_ number of handlers in handlers list
	virtual std::pair<ErrorCode, uint64_t> read(std::string_view name_, size_t handlerCount_, chunkHandler const* const handlers_) = 0;

};

}
#endif //WYRD_IBUNDLE_H
