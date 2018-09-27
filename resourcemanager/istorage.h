#pragma once
#ifndef WYRD_RESOURCEMAN_STORAGE_H
#define WYRD_RESOURCEMAN_STORAGE_H

#include <string_view>
#include <iostream>
#include "binny/ibundle.h"
#include "resourcemanager/base.h"
#include "resourcemanager/resourcename.h"

namespace ResourceManager {

struct IStorage
{
	using Ptr = std::shared_ptr<IStorage>;
	using ChunkFunc = Binny::IBundle::ChunkFunc;
	using ChunkHandler = Binny::IBundle::ChunkHandler;

	// only every return a static string with global lifetime
	virtual auto getPrefix() -> std::string_view = 0;

	virtual auto read(ResourceNameView const resourceName_, std::vector<ChunkHandler> const& handlers_ ) -> bool = 0;
};

}

#endif //WYRD_RESOURCEMAN_STORAGE_H
