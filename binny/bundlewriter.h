#pragma once
#ifndef BINNY_BUNDLE_WRITER_H
#define BINNY_BUNDLE_WRITER_H

#include <string>
#include <vector>
#include <functional>
#include "writehelper.h"

namespace Binny {

class BundleWriter
{
public:
	BundleWriter(int addressLength_ = 64);
	using ChunkWriter = std::function<void(WriteHelper& helper)>;

	/// add a raw text chunk, will be stored independently in its own chunk
	void addRawTextChunk(std::string const& name_, uint32_t id_, uint16_t majorVersion_, uint16_t minorVersion_, std::string const& text_);

	/// add a raw binary chunk, will be stored in its own chunk without any processing
	void addRawBinaryChunk(std::string const& name_, uint32_t id_, uint16_t majorVersion_, uint16_t minorVersion_, std::vector<uint8_t> const& bin_);

	/// @param ChunkWriter will be called at build time
	void addChunk(std::string const& name_, uint32_t id_, uint16_t majorVersion_, uint16_t minorVersion_, ChunkWriter writer_);

	/// @param userData_ a 64 bit in that store in the header, usually a cache / re-gen marker
	/// @param result_ where the bundle data will be put
	void build(uint64_t const userData_, std::vector<uint8_t>& result_);

	void setLogBinifyText() { logBinifyText = true; }

private:
	void addChunkInternal(std::string const& name_, uint32_t id_, std::vector<uint8_t> const& bin_);

	int addressLength;
	std::unique_ptr<WriteHelper> helper;
	WriteHelper& o;

	struct DirEntryWriter
	{
		uint32_t id;
		std::string name;
		size_t compressedSize;
		size_t uncompressedSize;
		uint32_t compressedCrc32c;
		uint32_t uncompressedCrc32c;
		std::vector<uint8_t>* chunk;
	};

	std::vector<DirEntryWriter> dirEntries;
	bool logBinifyText = false;
};

} // end namespace


#endif //BINNY_BUNDLE_WRITER_H
