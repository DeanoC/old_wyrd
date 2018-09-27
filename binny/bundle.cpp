#include "core/core.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <array>
#include "lz4/lz4.h"
#include "crc32c/crc32c.h"
#include "bundle.h"

namespace Binny {

Bundle::~Bundle()
{
	if(directory)
	{ tmpFree( directory ); }
	if(stringMemory)
	{ tmpFree( stringMemory ); }
}

std::pair<Bundle::ErrorCode, uint64_t> Bundle::read( std::string_view name_, std::vector<ChunkHandler> const& handlers_, bool forcePrefix_ )
{
	// read header
	Header header;
	auto ret = readHeader( header );
	if(ret.first != ErrorCode::Okay) return ret;

	if(directory)
	{ tmpFree( directory ); }
	if(stringMemory)
	{ tmpFree( stringMemory ); }

	static const int sizeOfPtr = sizeof( uintptr_t );
	size_t const dirMemorySize = header.chunkCount * sizeof( DirEntry );
	directory = (DirEntry *) tmpAlloc( dirMemorySize );
	stringMemory = (char *) tmpAlloc( header.stringTableSize );

	// now read the directory
	// special case 32 bit files on 64 bit systems
	if(header.flags & HeaderFlag_32Bit && sizeOfPtr == 8)
	{
		size_t const dirMemorySize32 = header.chunkCount * sizeof( DiskDirEntry32 );
		std::vector<uint8_t> readMemory( dirMemorySize32 );

		DiskDirEntry32 const *dir32 = (DiskDirEntry32 const *) readMemory.data();
		in.read((char *) dir32, dirMemorySize32 );
		if(in.fail())
		{
			return {ErrorCode::ReadError, 0ul};
		}
		// convert
		for(size_t i = 0; i < header.chunkCount; i++)
		{
			directory[i].id = dir32[i].id;
			directory[i].nameOffset = dir32[i].nameOffset;
			directory[i].storedSize = dir32[i].storedSize;
			directory[i].uncompressedSize = dir32[i].uncompressedSize;
			directory[i].storedCrc32c = dir32[i].storedCrc32c;
			directory[i].uncompressedCrc32c = dir32[i].uncompressedCrc32c;
			directory[i].storedOffset = dir32[i].storedOffset;
		}
	} else
	{
		in.read((char *) directory, dirMemorySize );
	}

	in.seekg( header.stringsMicroOffset, in.cur );
	in.read( stringMemory, header.stringTableSize );
	if(in.fail())
	{
		return {ErrorCode::ReadError, 0ul};
	}
	in.seekg( header.chunksMicroOffset, in.cur );

	size_t maxBufferSize = 0;

	// fixup string table and directory names now
	for(size_t i = 0; i < header.chunkCount; i++)
	{
		directory[i].nameOffset = (uintptr_t) stringMemory + directory[i].nameOffset;

		maxBufferSize = std::max( maxBufferSize, directory[i].uncompressedSize );
	}

	uint8_t *loadBuffer = (uint8_t *) tmpAlloc( maxBufferSize );
	uint8_t *decompBuffer = (uint8_t *) tmpAlloc( maxBufferSize );

	size_t totalExtraMem = 0;
	// TODO temp memory allocator for this map
	std::unordered_map<uint32_t, std::array<int, MaxHandlerStages>> handlerMap( handlers_.size());
	for(int j = 0; j < handlers_.size(); ++j)
	{
		uint32_t type;
		int order;
		ChunkFunc handler;
		uint32_t extramem;
		std::tie( type, order, extramem, handler ) = handlers_.at( j );
		handlerMap[type].at( order ) = j + 1; // 1 indexed so 0 can indicate no handler
		totalExtraMem += extramem;
	}

	uintptr_t lastStoredSize = 0;

	bool found = false;
	// load and decompress (if required) chunks
	for(size_t i = 0; i < header.chunkCount; i++)
	{
		DirEntry& dir = directory[i];

		// skip any unhandled chunks
		if(handlerMap.find( dir.id ) == handlerMap.end())
		{
			continue;
		}
		if(!name_.empty())
		{
			if(name_ != std::string_view( dir.getName()))
			{
				continue;
			}
		}
		found = true;

		in.seekg( dir.storedOffset - lastStoredSize, in.cur );
		lastStoredSize = dir.storedSize;

		in.read((char *) loadBuffer, dir.storedSize );
		if(in.fail())
		{
			return {ErrorCode::ReadError, 0ul};
		}
		uint32_t crc32c = crc32c_append( 0, loadBuffer, dir.storedSize );
		if(crc32c != dir.storedCrc32c) return {ErrorCode::CorruptError, 0ul};;

		uint8_t *fixupBuffer = loadBuffer;
		if(dir.uncompressedSize != dir.storedSize)
		{
			int okay = LZ4_decompress_safe((char *) loadBuffer,
										   (char *) decompBuffer,
										   (int) dir.storedSize,
										   (int) dir.uncompressedSize );

			if(okay < 0) return {ErrorCode::CompressionError, 0ul};
			if((okay) != dir.uncompressedSize) return {ErrorCode::CompressionError, 0ul};

			uint32_t ucrc32c = crc32c_append( 0, decompBuffer, dir.uncompressedSize );
			if(ucrc32c != dir.uncompressedCrc32c) return {ErrorCode::CorruptError, 0ul};
			fixupBuffer = decompBuffer;
		}

		Bundle::ChunkHeader const *cheader = (Bundle::ChunkHeader const *) fixupBuffer;

		size_t memorySize = cheader->dataSize;
		if(totalExtraMem > 0 || forcePrefix_)
		{
			memorySize += sizeof(uintptr_t) * MaxHandlerStages;
		}
		memorySize += totalExtraMem;


		// callee owns this memory!
		uint8_t *basePtr = (uint8_t *) permAlloc( memorySize );
		uint8_t *dataPtr = dataPtr;
		if(totalExtraMem)
		{
			std::memset(dataPtr, 0, sizeof(uintptr_t) * MaxHandlerStages);
			dataPtr += sizeof(uintptr_t) * MaxHandlerStages;
		}

		// copy all the data over
		std::memcpy( dataPtr, fixupBuffer + cheader->dataOffset, cheader->dataSize );

		// do the fixups
		// TODO 32 bit file on 64 bit fixup magic (need to do major runtime surgery)
		assert( !(header.flags & HeaderFlag_32Bit && sizeOfPtr == 8));

		// begining of the fixup table
		uintptr_t *fixupTable = (uintptr_t *) (fixupBuffer + cheader->fixupOffset);

		size_t numFixups = cheader->fixupSize / sizeOfPtr;
		for(size_t i = 0; i < numFixups; ++i)
		{
			uintptr_t *varAddress = (uintptr_t *) (dataPtr + fixupTable[i]);
			if((uintptr_t) (varAddress) >= (uintptr_t) (dataPtr + cheader->dataSize))
				return {ErrorCode::CorruptError, 0ul};
			if(*varAddress >= cheader->dataSize) return {ErrorCode::CorruptError, 0ul};
			*varAddress = (uintptr_t) (dataPtr + *varAddress);
			if((uintptr_t) (*varAddress) >= (uintptr_t) (dataPtr + cheader->dataSize))
				return {ErrorCode::CorruptError, 0ul};
		}

		// for debuggin mostly
		dir.storedOffset = (uintptr_t) basePtr;

		// call the callee back with memory, version etc for this chunk
		// we've already skipped any ids we don't handle
		auto localFree = permFree; // this ensure the function pointer outlives the bundle
		auto ptr = std::shared_ptr<void>((void *) basePtr,
			 [localFree]( void *ptr )
			 {
				 localFree( ptr );
			 } );

		uint8_t* extraMemPtr = dataPtr + cheader->dataSize;
		for(int j = 0; j < MaxHandlerStages; ++j)
		{
			auto const handlerIndex = handlerMap[dir.id].at( j );
			if(handlerIndex > 0)
			{
				uint32_t type;
				int order;
				ChunkFunc handler;
				uint32_t extramem;
				std::tie( type, order, extramem, handler ) = handlers_.at( handlerIndex - 1 );

				if(handler != nullptr)
				{
					if(extramem > 0)
					{
						((uintptr_t *) basePtr)[j] = (uintptr_t) extraMemPtr;
						extraMemPtr += extramem;
					}
					handler( dir.getName(), cheader->majorVersion, cheader->minorVersion, ptr );
				}
			}
		}
	}

	tmpFree( decompBuffer );
	tmpFree( loadBuffer );
	if(found == false) return {ErrorCode::NotFound, header.userData};
	else return {ErrorCode::Okay, header.userData};
}

std::pair<Bundle::ErrorCode, uint64_t> Bundle::peekAtHeader()
{
	Header header;
	auto posInStream = in.tellg();
	auto ret = readHeader( header );
	in.seekg( posInStream );
	return ret;
}

std::pair<Bundle::ErrorCode, uint64_t> Bundle::readHeader( Header& header )
{
	// read header
	in.read((char *) &header, sizeof( header ));
	if(in.fail())
	{
		return {ErrorCode::ReadError, 0ul};
	}
	assert( header.magic == "BUND"_bundle_id );
	assert( header.majorVersion == majorVersion );
	assert( header.minorVersion <= minorVersion );

	static const int sizeOfPtr = sizeof( uintptr_t );
	// we support 32 bit address on 64 bit machine and 64 on 64 but not 32 or 64 on 32
	if(sizeOfPtr < 8 && header.flags & HeaderFlag_64Bit)
	{
		return {ErrorCode::AddressLength, 0ul};
	}

	chunkCount = header.chunkCount;

	return {ErrorCode::Okay, header.userData};
}

uint32_t Bundle::getDirectoryCount()
{
	if(chunkCount == 0)
	{
		peekAtHeader();
	}
	return chunkCount;
};

std::string_view Bundle::getDirectionEntry( uint32_t const index_ )
{
	auto posInStream = in.tellg();
	read( {}, {}, false );
	in.seekg( posInStream );
	assert( index_ < chunkCount );

	return directory[index_].getName();
}

} // end namespace Core