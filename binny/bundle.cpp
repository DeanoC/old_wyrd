#include "core/core.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include "lz4/lz4.h"
#include "crc32c/crc32c.h"
#include "bundle.h"

namespace Binny {

Bundle::~Bundle()
{
	if(directory) { tmpFree(directory); }
	if(stringMemory) { tmpFree(stringMemory); }
}

std::pair<Bundle::ErrorCode, uint64_t> Bundle::read(std::istream& in_, allocFunc alloc_, size_t handlerCount, chunkHandler const* const handlers)
{
	// read header
	Header header;
	auto ret = readHeader(in_, header);
	if(ret.first != ErrorCode::Okay) return ret;

	static const int sizeOfPtr = sizeof(uintptr_t);
	size_t const dirMemorySize = header.chunkCount * sizeof(DirEntry);
	directory = (DirEntry*) tmpAlloc(dirMemorySize);
	stringMemory = (char*) tmpAlloc(header.stringTableSize);

	// now read the directory
	// special case 32 bit files on 64 bit systems
	if (header.flags & HeaderFlag_32Bit && sizeOfPtr == 8)
	{
		size_t const dirMemorySize32 = header.chunkCount * sizeof(DiskDirEntry32);
		std::vector<uint8_t> readMemory(dirMemorySize32);

		DiskDirEntry32 const* dir32 = (DiskDirEntry32 const*)readMemory.data();
		in_.read((char*)dir32, dirMemorySize32);
		if (in_.fail())
		{
			return { ErrorCode::ReadError, 0ul };
		}
		// convert
		for (size_t i = 0; i < header.chunkCount; i++)
		{
			directory[i].id = dir32[i].id;
			directory[i].nameOffset = dir32[i].nameOffset;
			directory[i].storedSize = dir32[i].storedSize;
			directory[i].uncompressedSize = dir32[i].uncompressedSize;
			directory[i].storedCrc32c = dir32[i].storedCrc32c;
			directory[i].uncompressedCrc32c = dir32[i].uncompressedCrc32c;
			directory[i].storedOffset = dir32[i].storedOffset;
		}
	}
	else
	{
		in_.read((char*)directory, dirMemorySize);
	}

	in_.seekg(header.stringsMicroOffset, in_.cur);
	in_.read(stringMemory, header.stringTableSize);
	if (in_.fail())
	{
		return { ErrorCode::ReadError, 0ul };
	}
	in_.seekg(header.chunksMicroOffset, in_.cur);

	size_t maxBufferSize = 0;

	// fixup string table and directory names now
	for (size_t i = 0; i < header.chunkCount; i++)
	{
		directory[i].nameOffset = (uintptr_t) stringMemory + directory[i].nameOffset;

		maxBufferSize = std::max(maxBufferSize, directory[i].uncompressedSize);
	}

	uint8_t* loadBuffer = (uint8_t*) tmpAlloc(maxBufferSize);
	uint8_t* decompBuffer = (uint8_t*) tmpAlloc(maxBufferSize);

	// TODO temp memory allocator for this map
	std::unordered_map<uint32_t, chunkFunc> handlerMap;
	for(int j = 0; j < handlerCount; ++j)
	{
		handlerMap[ std::get<0>(handlers[j]) ] = std::get<1>(handlers[j]);
	}

	uintptr_t lastStoredSize = 0;

	// load and decompress (if required) chunks
	for (size_t i = 0; i < header.chunkCount; i++)
	{
		DirEntry& dir = directory[i];

		// skip any unhandled chunks
		if(handlerMap.find(dir.id) == handlerMap.end())
		{
			continue;
		}

		in_.seekg(dir.storedOffset - lastStoredSize, in_.cur);
		lastStoredSize = dir.storedSize;

		in_.read((char*)loadBuffer, dir.storedSize);
		if (in_.fail())
		{
			return { ErrorCode::ReadError, 0ul };
		}
		uint32_t crc32c = crc32c_append(0, loadBuffer, dir.storedSize);
		if (crc32c != dir.storedCrc32c) return { ErrorCode::CorruptError, 0ul };;

		uint8_t* fixupBuffer = loadBuffer;
		if (dir.uncompressedSize != dir.storedSize)
		{
			int okay = LZ4_decompress_safe((char*)loadBuffer,
						(char*)decompBuffer,
						(int)dir.storedSize, 
						(int)dir.uncompressedSize);

			if (okay < 0) return { ErrorCode::CompressionError, 0ul };
			if ((okay) != dir.uncompressedSize) return { ErrorCode::CompressionError, 0ul };

			uint32_t ucrc32c = crc32c_append(0, decompBuffer, dir.uncompressedSize);
			if (ucrc32c != dir.uncompressedCrc32c) return { ErrorCode::CorruptError, 0ul };
			fixupBuffer = decompBuffer;
		}

		Bundle::ChunkHeader const * cheader = (Bundle::ChunkHeader const*) fixupBuffer;

		// callee owns this memory!
		uint8_t* dataPtr = (uint8_t*) alloc_(cheader->dataSize);
		// copy all the data over
		std::memcpy(dataPtr, fixupBuffer + cheader->dataOffset, cheader->dataSize);

		// do the fixups
		// TODO 32 bit file on 64 bit fixup magic (need to do major runtime surgery)
		assert(!(header.flags & HeaderFlag_32Bit && sizeOfPtr == 8));

		// begining of the fixup table
		uintptr_t* fixupTable = (uintptr_t*)(fixupBuffer + cheader->fixupOffset);

		size_t numFixups = cheader->fixupSize / sizeOfPtr;
		for (size_t i = 0; i < numFixups; ++i)
		{
			uintptr_t* varAddress = (uintptr_t*)(dataPtr + fixupTable[i]);
			if( (uintptr_t)(varAddress) >= (uintptr_t)(dataPtr + cheader->dataSize)) return { ErrorCode::CorruptError, 0ul };
			if(*varAddress >= cheader->dataSize) return { ErrorCode::CorruptError, 0ul };
			*varAddress = (uintptr_t)(dataPtr + *varAddress);
			if((uintptr_t)(*varAddress) >= (uintptr_t)(dataPtr + cheader->dataSize))return { ErrorCode::CorruptError, 0ul };
		}

		// for debuggin mostly
		dir.storedOffset = (uintptr_t)dataPtr;

		// call the callee back with memory, version etc for this chunk
		// we've already skipped any ids we don't handle
		handlerMap[dir.id](cheader->majorVersion, cheader->minorVersion, cheader->dataSize, (void*)dataPtr );
	}

	tmpFree(decompBuffer);
	tmpFree(loadBuffer);

	return { ErrorCode::Okay, header.userData };
}

std::pair<Bundle::ErrorCode, uint64_t> Bundle::peekAtHeader(std::istream& in_)
{
	Header header;
	auto posInStream = in_.tellg();
	auto ret = readHeader(in_, header);
	in_.seekg(posInStream);
	return ret;
}

std::pair<Bundle::ErrorCode, uint64_t> Bundle::readHeader(std::istream& in_, Header& header)
{
	// read header
	in_.read((char*)&header, sizeof(header));
	if (in_.fail())
	{
		return { ErrorCode::ReadError, 0ul };
	}
	assert(header.magic == "BUND"_bundle_id);
	assert(header.majorVersion == majorVersion);
	assert(header.minorVersion <= minorVersion);

	static const int sizeOfPtr = sizeof(uintptr_t);
	// we support 32 bit address on 64 bit machine and 64 on 64 but not 32 or 64 on 32
	if (sizeOfPtr < 8 && header.flags & HeaderFlag_64Bit)
	{
		return { ErrorCode::AddressLength, 0ul };
	}

	return { ErrorCode::Okay, header.userData };
}

} // end namespace Core