#pragma once
#ifndef BINNY_IBUNDLE_H
#define BINNY_IBUNDLE_H

#include "core/core.h"
#include <string>
#include <vector>
#include <functional>

namespace Binny {

/// used to identify chunks, each should be unique to a project etc. IFF like
constexpr uint32_t operator "" _bundle_id(char const* s, size_t count)
{
	assert(count == 4);
	return s[3] << 24 | s[2] << 16 | s[1] << 8 | s[0] << 0;
}

struct IBundle
{
	using Ptr = std::shared_ptr<IBundle>;

	enum class ErrorCode
	{
		Okay = 0,            // no error
		NotFound,            // nothing wanted was found
		AddressLength,        // address length issues probably 64 bit bundle on 32 bit system
		ReadError,            // stream issues
		CorruptError,        // failed internal crc checks
		CompressionError,    // decompression failed
		MemoryError,        // error allocating memory
		OtherError            // generic error
	};
	using AllocFunc = std::function<void*(size_t)>;
	using FreeFunc = std::function<void(void*)>;
	using ReadReturn = std::pair<ErrorCode, uint64_t>;

	// Stages allow additionaly handlers to hook into resource allocations.
	// For example a texture could have additional stages for the various API (Vulkan etc.)
	// to process it. Each stage can have additional memory associated with it for its own
	// purposes but this isn't serialized (yet).
	// Stage 0 is the main stage and should represent the actual data. Whilst it can specify
	// extra memory i'm not sure thats useful.
	// All memory in the chunk is a single continous block.
	// Stage extra memory is currently physically located at the end of the data, but as
	// stage 0 may be variable sized, finding the location of the extra memory can
	// be done via a prefix. Here the first MaxHandlerStages ptrs of memory returned are filled
	// pointers to the extra memory allocated for each stage.
	// The prefix is optional and you can choose to have this allocated already embeded in
	// the data or as an extra sized part.
	// the base of memory passed to most functions it the actual start of the allocated block
	// if there is a prefix this means the stage 0 data will be uintptr_t * MaxHandlerStages from this
	// address

	constexpr static unsigned int MaxHandlerStages = 4;

	// chunk name, stage, major version, minor version, memory size, smart pointer to the base of memory
	using ChunkCreateFunc = std::function<bool(std::string_view, int, uint16_t, uint16_t, size_t,
											   std::shared_ptr<void>)>;

	// the pointer passed to destroy is a unsmart pointer to the base memory
	using ChunkDestroyFunc = std::function<auto(int, void*)->void>;

	using ChunkHandler = struct
	{
		uint32_t id;
		int stage;
		uint32_t extraMem;
		ChunkCreateFunc createFunc;
		ChunkDestroyFunc destroyFunc;
		bool writePrefix = false; // stage 0 only
		bool allocatePrefix = false; // stage 0 only
	};

	// the extra mem parameter needs some explaination. Each stage can request
	// some extra memory just for itself. This will be add the end of the stage 0 memory
	// as stage 0 may be variable sized, when a read is called it can specify to put pointers
	// in the MaxHandlerStages first locations of the returned memory (known as the prefix).
	// Each of these will point to each stages extra memory (note that yes stage 0 can have extra
	// memory but not sure its every useful)

	// the actual allocation will be one large block and its this block that will be
	// passed the free function, the chunk funcs will be pointers to the actual extra memory
	virtual auto getDirectoryCount() -> uint32_t = 0;
	virtual auto getDirectoryEntry(uint32_t const index_) -> std::string_view = 0;

	/// @param name_ name of chunk wanted, empty to load all given handler types
	/// @param handlers vector of handler to process the chunk of a given type
	/// @param writePrefix will write pointers into the begining of memory
	/// @param allocatePrefix_ whether should add the size of the prefix onto the data size
	virtual auto read(std::string_view name_,
					  std::vector<ChunkHandler> const& handlers_) -> ReadReturn = 0;

};

}
#endif //WYRD_IBUNDLE_H
