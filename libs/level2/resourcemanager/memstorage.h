#pragma once
#ifndef WYRD_RESOURCEMANANAGER_MEMSTORAGE_H
#define WYRD_RESOURCEMANANAGER_MEMSTORAGE_H

#include "core/core.h"
#include "core/quick_hash.h"
#include "resourcemanager/istorage.h"
#include "binny/bundle.h"
#include "tbb/concurrent_unordered_map.h"
#include "binny/inmembundle.h"
#include <string_view>
#include <iostream>
#include <fstream>
#include <ios>

namespace ResourceManager {

struct MemStorage : public IStorage
{
	using FilenameToMemory = tbb::concurrent_unordered_map<std::string, std::tuple<ResourceId, uint16_t, uint16_t, std::vector<uint8_t>>>;

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

		Binny::InMemBundle bundle(alloc_, free_, (uint32_t) type, majorVersion, minorVersion, mem.data(), mem.size());
		auto okay = bundle.read(resourceName_.getSubObject(), handlers_);
		return okay.first == Binny::IBundle::ErrorCode::Okay;
	}

	// the memory added will be copied and managed
	auto addMemory(std::string name_, ResourceId id_, uint16_t majorVersion_, uint16_t minorVersion_, void const* mem_,
				   size_t size_) -> void
	{
		if(filenameToMemory.find(name_) != filenameToMemory.end())
		{
			LOG_S(ERROR) << name_ << " is already in memory storage";
			return;
		}

		std::vector<uint8_t> mem(size_);
		std::memcpy(mem.data(), mem_, size_);

		filenameToMemory[name_] = {id_, majorVersion_, minorVersion_, std::move(mem)};
	}
	auto removeMemory(std::string name_) -> void
	{
		auto const& it = filenameToMemory.find(name_);
		if(it != filenameToMemory.end())
		{
			filenameToMemory.unsafe_erase(it);
		}
	}

	FilenameToMemory filenameToMemory;
};

}

#endif //WYRD_RESOURCEMAN_MEMSTORAGE_H
