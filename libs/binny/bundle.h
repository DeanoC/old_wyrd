#pragma once
#ifndef BINNY_BUNDLE_H
#define BINNY_BUNDLE_H

#include "core/core.h"
#include "core/utils.h"
#include <string>
#include <vector>
#include <functional>
#include "binny/ibundle.h"

namespace Binny {

/// A bundle is binary resource file.
/// The chunk allocs are never freed and are the callees responsbility
/// It only ever seeks forward and skips chunk that are not handled
/// It allows multiple version of the same chunk in a single resource and passes the version info to the handler
/// Chunks are compressed if its useful (currently LZ4)
/// Pointers are fixed up before handlers are called 
/// Bundle are designed to be endian specific currently and pointer size specific (however 32bit to 64bit is planned)
/// raw text and binary chunks can be stored, these will be compressed by apart from that no fixups
class Bundle : public IBundle
{
public:
	friend class BundleWriter;
	friend class WriteHelper;

	// if this flag is set, the memory will come out of the temp pool
	// and will be freed
	static constexpr uint32_t ChunkFlag_TempAlloc = Core::Bit(0u);

	// temporary allocations, will be called whenever it needs some temporary
	// memory and free called when no longer needed. None will outlive the Bundle
	// alloc_ and free_ will be called for the real data and free called when the
	// smart pointer passed to handler dies
	Bundle(	AllocFunc alloc_,
			FreeFunc free_,
			AllocFunc tmpAlloc_,
			FreeFunc tmpFree_,
			std::istream& in_) :
				permAlloc(alloc_), permFree(free_),
				tmpAlloc(tmpAlloc_), tmpFree(tmpFree_),
				in(in_) {}
	~Bundle();

	// the ChunkHandler hold processes the chunk once its been loaded and fixed up
	std::pair<ErrorCode, uint64_t> read(std::string_view name_,
			std::vector<ChunkHandler> const& handlers_, bool forcePrefix_) override;
	uint32_t getDirectoryCount() override;
	std::string_view getDirectoryEntry(uint32_t const index_) override;


	std::pair<ErrorCode, uint64_t> peekAtHeader();
protected:
	static const uint16_t majorVersion = 1;
	static const uint16_t minorVersion = 0;

	static constexpr uint32_t HeaderFlag_32Bit = Core::Bit(0u);
	static constexpr uint32_t HeaderFlag_64Bit = Core::Bit(1u);


	//  32 bytes
	struct Header
	{
		uint32_t magic;
		uint32_t flags;

		uint16_t majorVersion;
		uint16_t minorVersion;
		// this allow padding between the directory, the strings and the start of the chunks
		uint16_t stringsMicroOffset;
		uint16_t chunksMicroOffset;

		// this is a 64 bit user data item, usually used as a cache/regen id
		uint64_t userData;

		// string table is limited to 4GB I hope this limit never gets hit!
		uint32_t stringTableSize;
		uint32_t chunkCount;

	};
	static const int sizeOfHeader = sizeof(Header);
	static_assert(sizeOfHeader == 32, "Bundle::Header is not 32 bytes long");

	struct DiskDirEntry32
	{
		uint32_t id;
		uint32_t storedCrc32c;
		uint32_t uncompressedCrc32c;
		uint32_t flags;

		uint32_t nameOffset;
		uint32_t storedOffset;
		uint32_t storedSize;
		uint32_t uncompressedSize;
	};

	struct ChunkHeader32
	{
		uint32_t fixupSize;
		uint32_t dataSize;
		uint32_t fixupOffset;
		uint32_t dataOffset;
		uint16_t majorVersion;
		uint16_t minorVersion;
	};

	struct DirEntry
	{
		uint32_t id;
		uint32_t storedCrc32c;
		uint32_t uncompressedCrc32c;
		uint32_t flags;

		uintptr_t nameOffset;	// this is replaced by the real name pointer 
		uintptr_t storedOffset; // this is never used after fixup but is set to the ptr for debugging
		uintptr_t storedSize;
		uintptr_t uncompressedSize;

		char const * getName() const {
			return *reinterpret_cast<char const* const*>(&nameOffset);
		}

	};

	struct ChunkHeader
	{
		uintptr_t fixupSize;
		uintptr_t dataSize;
		uintptr_t fixupOffset;
		uintptr_t dataOffset;
		uint16_t majorVersion;
		uint16_t minorVersion;
	};

	AllocFunc permAlloc;
	FreeFunc permFree;
	AllocFunc tmpAlloc;
	FreeFunc tmpFree;
	std::istream& in;

	uint32_t chunkCount = 0;
	DirEntry* directory = nullptr;
	char* stringMemory = nullptr;

	std::pair<ErrorCode, uint64_t> readHeader(Header & header);

};

} // end namespace


#endif //BINNY_BUNDLE_H
