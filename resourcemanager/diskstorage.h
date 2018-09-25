#pragma once
#ifndef WYRD_RESOURCEMAN_DISKSTORAGE_H
#define WYRD_RESOURCEMAN_DISKSTORAGE_H

#include "core/core.h"
#include <string_view>
#include <iostream>
#include <fstream>
#include <ios>
#include "resourcemanager/istorage.h"
#include "binny/bundle.h"

namespace ResourceManager {

struct DiskStorage : public IStorage {
	auto getPrefix() -> std::string_view final
	{
		using namespace std::string_view_literals;
		return "disk"sv;
	}

	auto read(ResourceNameView const resourceName_, size_t handlerCount_, chunkHandler const* const handlers_) -> bool final
	{
		std::string_view prefix = resourceName_.getStorage();
		assert(prefix == getPrefix());
		std::string_view name = resourceName_.getName();

		// we ignore the subobject and load them all and insert them into the cache
		// this policy can be changed here
		//std::string_view subObject = resourceName_.getSubObject();
		std::string_view subObject = {};

		auto stream = std::ifstream(static_cast<std::string>(name), std::ifstream::binary | std::ifstream::in);
		if(stream.bad()) return false;

		auto bundle = Binny::Bundle(&malloc, &free, &malloc, &free, stream);
		auto okay = bundle.read(subObject, handlerCount_, handlers_);
		return okay.first == Binny::IBundle::ErrorCode::Okay;
	}

};

}

#endif //WYRD_RESOURCEMAN_DISKSTORAGE_H
