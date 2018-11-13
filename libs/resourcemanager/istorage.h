#pragma once
#ifndef WYRD_RESOURCEMANANAGER_STORAGE_H
#define WYRD_RESOURCEMANANAGER_STORAGE_H

#include "binny/ibundle.h"
#include "resourcemanager/resource.h"
#include "resourcemanager/resourcename.h"
#include <string_view>
#include <iostream>

namespace ResourceManager {

struct IStorage
{
	using Ptr = std::shared_ptr<IStorage>;
	using ChunkHandler = Binny::IBundle::ChunkHandler;
	using AllocFunc = std::function<void*(size_t)>;
	using FreeFunc = std::function<void(void*)>;

	// only every return a static string with global lifetime
	virtual auto getPrefix() -> std::string_view = 0;

	virtual auto read(	ResourceNameView const resourceName_,
						AllocFunc alloc_,
						FreeFunc  free_,
						std::vector<ChunkHandler> const& handlers_ ) -> bool = 0;
};

}

#endif //WYRD_RESOURCEMAN_STORAGE_H
