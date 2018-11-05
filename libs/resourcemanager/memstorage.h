#pragma once
#ifndef WYRD_RESOURCEMANANAGER_MEMSTORAGE_H
#define WYRD_RESOURCEMANANAGER_MEMSTORAGE_H

#include "core/core.h"
#include "core/quick_hash.h"
#include <string_view>
#include <iostream>
#include <fstream>
#include <ios>
#include "resourcemanager/istorage.h"
#include "binny/bundle.h"
#include "tbb/concurrent_unordered_map.h"
#include "binny/inmembundle.h"

namespace ResourceManager {

struct MemStorage : public IStorage
{
	using FilenameToMemory = tbb::concurrent_unordered_map<std::string, std::tuple<uint32_t, uint16_t, uint16_t, std::vector<uint8_t>>>;

	auto getPrefix() -> std::string_view final
	{
		using namespace std::string_view_literals;
		return "mem"sv;
	}

	auto read(	ResourceNameView const resourceName_,
				  AllocFunc alloc_,
				  FreeFunc  free_,
				  std::vector<ChunkHandler> const& handlers_) -> bool final
	{
		std::string name = std::string(resourceName_.getNameAndSubObject());
		if(filenameToMemory.find(name) == filenameToMemory.end())
			return false;
		auto const[type, majorVersion, minorVersion, mem] = filenameToMemory[name];

		Binny::InMemBundle bundle(alloc_, free_, type, majorVersion, minorVersion, mem.data(), mem.size());
		auto okay = bundle.read(resourceName_.getSubObject(), handlers_);
		return okay.first == Binny::IBundle::ErrorCode::Okay;
	}

	// the memory added will be copied and managed
	auto addMemory(std::string name_, uint32_t type_, uint16_t majorVersion_, uint16_t minorVersion_, void const* mem_, size_t size_) -> bool
	{
		if(filenameToMemory.find(name_) != filenameToMemory.end())
			return false;

		std::vector<uint8_t> mem(size_);
		std::memcpy(mem.data(), mem_, size_);

		filenameToMemory[name_] = {type_, majorVersion_, minorVersion_, std::move(mem)};
		return true;
	}

	FilenameToMemory filenameToMemory;
};

}

#endif //WYRD_RESOURCEMAN_MEMSTORAGE_H
