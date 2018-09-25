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
	using chunkFunc = Binny::IBundle::chunkFunc;
	using chunkHandler = Binny::IBundle::chunkHandler;

	// only every return a static string with global lifetime
	virtual auto getPrefix() -> std::string_view = 0;

	virtual auto read(ResourceNameView const resourceName_, size_t handlerCount_, chunkHandler const *const handlers_ ) -> bool = 0;
};

}

#endif //WYRD_RESOURCEMAN_STORAGE_H
