#include "core/core.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include "binify/binify.h"
#include "lz4/lz4.h"
#include "crc32c/crc32c.h"
#include "binify/binify_c_api.h"
#include "bundle.h"
#include "bundlewriter.h"
#include "writehelper.h"


namespace Binny {

BundleWriter::BundleWriter(int addressLength_) :
	addressLength(addressLength_),
	helper(std::make_unique<WriteHelper>()),
	o(*helper)
{
	assert(addressLength == 64 || addressLength == 32);
	o.set_address_length(addressLength);
}


bool BundleWriter::addRawTextChunk(std::string const& name_, uint32_t id_, uint16_t majorVersion_, uint16_t minorVersion_, std::string const& text_)
{
	bool nullTerminated = text_.back() == '\0';

	std::vector<uint8_t> bin(text_.size() + sizeof(Bundle::ChunkHeader) + (nullTerminated ? 0 : 1));
	std::memcpy(bin.data() + sizeof(Bundle::ChunkHeader), text_.data(), text_.size());
	if (nullTerminated == false)
	{
		bin[sizeof(Bundle::ChunkHeader) + text_.size()] = 0; // null terminate string
	}

	// write chunk header
	Bundle::ChunkHeader* cheader = (Bundle::ChunkHeader*) bin.data();
	cheader->dataSize = text_.size() + 1;
	cheader->fixupOffset = 0;
	cheader->fixupSize = 0;
	cheader->dataOffset = sizeof(Bundle::ChunkHeader);
	cheader->majorVersion = majorVersion_;
	cheader->minorVersion = minorVersion_;

	return addChunkInternal(name_, id_, bin);
}

bool BundleWriter::addRawBinaryChunk(std::string const& name_, uint32_t id_, uint16_t majorVersion_, uint16_t minorVersion_, std::vector<uint8_t> const& bin_)
{
	std::vector<uint8_t> bin(bin_.size() + sizeof(Bundle::ChunkHeader));
	std::memcpy(bin.data() + sizeof(Bundle::ChunkHeader), bin_.data(), bin_.size());

	// write chunk header
	Bundle::ChunkHeader* cheader = (Bundle::ChunkHeader*) bin.data();
	cheader->dataSize = bin_.size();
	cheader->fixupOffset = 0;
	cheader->fixupSize = 0;
	cheader->dataOffset = sizeof(Bundle::ChunkHeader);
	cheader->majorVersion = majorVersion_;
	cheader->minorVersion = minorVersion_;

	return addChunkInternal(name_, id_, bin);
}

bool BundleWriter::addChunkInternal(std::string const& name_, uint32_t id_, std::vector<uint8_t> const& bin_)
{
	uint32_t const uncompressedCrc32c = crc32c_append(0, bin_.data(), bin_.size());
	int const maxSize = LZ4_compressBound((int)bin_.size());

	std::vector<uint8_t>*  compressedData = new std::vector<uint8_t>(maxSize);
	{
		int okay = LZ4_compress_default((char const*)bin_.data(),
										(char *)compressedData->data(),
										(int)bin_.size(),
										(int)compressedData->size());
		if(!okay) return false;
		compressedData->resize(okay);
	}

	// if compression made this block bigger, use the uncompressed data and mark it by
	// having uncompressed size == 0 in the file
	if (compressedData->size() >= bin_.size())
	{
		*compressedData = bin_;
	}

	if (addressLength == 32)
	{
		if(compressedData->size() >= (1ull << 32)) return false;
		if(bin_.size() >= (1ull << 32)) return false;
	}

	DirEntryWriter entry =
	{
		id_,
		name_,
		compressedData->size(),
		bin_.size(),
		crc32c_append(0, compressedData->data(), compressedData->size()),
		uncompressedCrc32c,
		compressedData
	};

	dirEntries.push_back(entry);
	o.reserve_label(name_ + "chunk"s);
	return true;
}

bool BundleWriter::addChunk(std::string const& name_, uint32_t id_, uint16_t majorVersion_, uint16_t minorVersion_, ChunkWriter writer_)
{
	WriteHelper h;

	// add chunk header
	h.write_chunk_header(majorVersion_, minorVersion_);

	h.write_label("data"s);
	writer_(h);
	h.finish_string_table();
	h.align();
	h.write_label("dataEnd"s);
	h.write_label("fixups"s);
	uint32_t const numFixups = (uint32_t)h.fixups.size();
	for (auto const& label : h.fixups)
	{
		h.use_label(label, "", false, false);
	}
	h.write_label("fixupsEnd"s);

	std::string binifyText = h.ostr.str();

	std::string log;
	std::vector<uint8_t> data;
	std::stringstream binData;
	bool okay = BINIFY_StringToOStream(binifyText, &binData, log);
	if(!okay) return false;
	std::string tmp = binData.str();
	data.resize(tmp.size());
	std::memcpy(data.data(), tmp.data(), tmp.size());

	if (!log.empty() || logBinifyText)
	{
		LOG_S(INFO) << name_ << "_Chunk:" << log;
		LOG_S(INFO) << std::endl << binifyText;
	}

	return addChunkInternal(name_, id_, data);
}

bool BundleWriter::build(uint64_t const userData_, std::vector<uint8_t>& result_)
{
	using namespace std::string_literals;

	o.write_bundle_header(userData_);

	// begin the directory
	o.reserve_label("begin"s, true);
	o.write_label("begin"s);

	std::string lastName = "chunks"s;
	for (auto[id, name, storedSize, unpackedSize, scrc32c, ucrc32c, chunk] : dirEntries)
	{
		o.write(id, "id type"s);
		o.write_as<uint32_t>(scrc32c, "stored crc32c"s);
		o.write_as<uint32_t>(ucrc32c, "unpacked crc32c"s);
		o.write_as<uint32_t>(0, "flags");

		o.add_string(name);
		o.use_label(name + "chunk"s, lastName, false, false );

		o.write_size(storedSize, "stored size"s);
		o.write_size(unpackedSize, "unpacked size"s);
		
		o.increment_variable("DirEntryCount"s);
		lastName = name + "chunk"s;
	}
	o.write_label("beginEnd"s);

	// output string table
	o.finish_string_table();

	// output pointer fixups

	// output chunks
	o.align();
	o.write_label("chunks"s);
	for (auto& entry : dirEntries)
	{
		assert(entry.chunk != nullptr);
		o.align();
		o.write_label(entry.name + "chunk"s);
		o.write_byte_array(*entry.chunk);
   		delete entry.chunk;
		entry.chunk = nullptr;
	}
	o.write_label("chunksEnd"s);

	// convert
	std::string binifyText = o.ostr.str();
	std::string log;
	std::stringstream binData;
	bool okay = BINIFY_StringToOStream(binifyText, &binData, log);
	if(!okay) return false;
	std::string tmp = binData.str();
	result_.resize(tmp.size());
	std::memcpy(result_.data(), tmp.data(), tmp.size());

	if (!log.empty() || logBinifyText)
	{
		LOG_S(INFO) << "Bundle: ";
		LOG_S(INFO) << std::endl << binifyText;
	}
	return true;
}

} // end namespace