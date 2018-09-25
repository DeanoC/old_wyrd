#pragma once
#ifndef WYRD_RESOURCEMAN_MEMSTORAGE_H
#define WYRD_RESOURCEMAN_MEMSTORAGE_H

#include "core/core.h"
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
	using FilenameToMemory = tbb::concurrent_unordered_map<std::string, std::tuple<uint32_t, uint16_t, uint16_t,std::shared_ptr<void>>>;

	auto getPrefix() -> std::string_view final
	{
		using namespace std::string_view_literals;
		return "mem"sv;
	}

	auto read(ResourceNameView const resourceName_, size_t handlerCount_, chunkHandler const* const handlers_) -> bool final
	{
		std::string name = std::string(resourceName_.getNameAndSubObject());
		if(filenameToMemory.find(name) == filenameToMemory.end())
			return false;
		auto [type, majorVersion, minorVersion, ptr] = filenameToMemory[name];

		Binny::InMemBundle bundle(type, majorVersion, minorVersion, ptr.get());
		auto okay = bundle.read(resourceName_.getSubObject(), handlerCount_, handlers_);
		return okay.first == Binny::IBundle::ErrorCode::Okay;
	}

	// when released the smart ptr passed will free using its destructor
	auto addMemory(std::string name_, uint32_t type_, uint16_t majorVersion_, uint16_t minorVersion_, std::shared_ptr<void>& mem_) -> bool
	{
		if(filenameToMemory.find(name_) != filenameToMemory.end())
			return false;
		filenameToMemory[name_] = { type_, majorVersion_, minorVersion_, mem_ };
		return true;
	}

	// used if the data is really static const (never needs freeing)
	template<typename T>
	auto addStaticMemory(std::string name_, uint32_t type_, uint16_t majorVersion_, uint16_t minorVersion_, T* mem_) -> bool
	{
		std::shared_ptr<void> ptr((void*)mem_, [](void*){});
		return addMemory(name_,type_, majorVersion_, minorVersion_, ptr);
	}


	FilenameToMemory filenameToMemory;
};

}

#endif //WYRD_RESOURCEMAN_MEMSTORAGE_H
