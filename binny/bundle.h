#pragma once
#ifndef BINNY_BUNDLE_H
#define BINNY_BUNDLE_H

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

/// A bundle is binary resource file.
/// The chunk allocs are never freed and are the callees responsbility
/// It only ever seeks forward and skips chunk that are not handled
/// It allows multiple version of the same chunk in a single resource and passes the version info to the handler
/// Chunks are compressed if its useful (currently LZ4)
/// Pointers are fixed up before handlers are called 
/// Bundle are designed to be endian specific currently and pointer size specific (however 32bit to 64bit is planned)
/// raw text and binary chunks can be stored, these will be compressed by apart from that no fixups
class Bundle
{
public:
	friend class BundleWriter;
	friend class WriteHelper;

	using allocFunc = std::function<void*(size_t)>;
	using freeFunc = std::function<void(void*)>;

	// these are for temporary allocations, alloc will be called whenever it needs some temporary
	// memory and free called when no longer needed. None will outlive the Bundle
	Bundle(allocFunc tmpAlloc_, freeFunc tmpFree_) : tmpAlloc(tmpAlloc_), tmpFree(tmpFree_) {}
	~Bundle();

	enum class ErrorCode
	{
		Okay = 0,			// no error
		AddressLength,		// address length issues probably 64 bit bundle on 32 bit system
		ReadError,			// stream issues
		CorruptError,		// failed internal crc checks
		CompressionError,	// decompression failed
		MemoryError,		// error allocating memory	
		OtherError			// generic error
	};

	// the chunkHandler hold processes the chunk once its been loaded and fixed up
	// 1 alloc call per chunk thats handled
	// the alloc memory will not be freed by the bundle, so its upto the callee to free

	// minor version, major version, sizeof chunk, pointer to data (this will be the same as returned by alloc)
	using chunkFunc = std::function<bool(uint16_t, uint16_t, size_t, void*)>;

	// id, function to handle this id
	using chunkHandler = std::tuple<uint32_t, chunkFunc>;

	std::pair<ErrorCode, uint64_t> read(std::istream& in_, allocFunc alloc_, size_t handlerCount, chunkHandler const* const handlers);

	std::pair<ErrorCode, uint64_t> peekAtHeader(std::istream& in_);
protected:
	static const uint16_t majorVersion = 0;
	static const uint16_t minorVersion = 4;

	static const uint32_t HeaderFlag_32Bit = 0x1;
	static const uint32_t HeaderFlag_64Bit = 0x2;

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

	allocFunc tmpAlloc;
	freeFunc tmpFree;

	DirEntry* directory = nullptr;
	char* stringMemory = nullptr;

	std::pair<ErrorCode, uint64_t> readHeader(std::istream& in_, Header & header);

};

} // end namespace


#endif //BINNY_BUNDLE_H
