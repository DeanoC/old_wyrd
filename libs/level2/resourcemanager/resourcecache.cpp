#include "core/core.h"
#include "resourcemanager/resourcecache.h"

namespace ResourceManager
{

auto ResourceCache::lookup(uint64_t id_) -> std::shared_ptr<ResourceBase>
{
	// look it up first without a mutex, if we found it, return it!
	auto it = cache.find(id_);
	if(it == cache.end())
	{
		std::lock_guard guard(updateMutex);
		// its possible we edge a race with the inserter, so look again
		it = cache.find(id_);
		if(it == cache.end())
		{
			// if still no luck, its can't have yet been inserted, so tell
			// the caller
			return {};
		}
	}
	return it->second;
}

void ResourceCache::insert(uint64_t id_, std::shared_ptr<ResourceBase> const& resource_)
{
	// this might be overkill but head fuzzy and better safe than sorry!
	std::lock_guard guard(updateMutex);
	if(cache.find(id_) == cache.end())
	{
		// not found in cache, put it in
		cache[id_] = resource_;
		return;
	}

	// this can occur due to timing (N things inserting same data and racing
	// now if they are the same data thats okay but if not, we have an issue
	// the mutex make this test work. first to the guard will finish, next
	// will get here after the first is done so we can check we have the same data
	assert(cache[id_] == resource_);
}

}