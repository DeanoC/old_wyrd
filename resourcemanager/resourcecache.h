#pragma once
#ifndef WYRD_RESOURCEMAN_RESOURCECACHE_H
#define WYRD_RESOURCEMAN_RESOURCECACHE_H

#include "core/core.h"
#include "core/freelist.h"
#include "resourcemanager/base.h"
#include "tbb/concurrent_unordered_map.h"
#include <string_view>
#include <functional>

namespace ResourceManager {

class ResourceCache
{
public:
	auto lookup(uint64_t id_) -> ResourceBase::Ptr;
	void insert(uint64_t id_, ResourceBase::Ptr const& resource_);

private:
	using IdToResource = tbb::concurrent_unordered_map<uint64_t, ResourceBase::Ptr>;

	IdToResource cache;
	std::mutex updateMutex;
};

}
#endif //WYRD_RESOURCEMAN_RESOURCECACHE_H
