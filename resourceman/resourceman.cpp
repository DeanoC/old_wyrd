//
// Created by Dean Calver on 18/09/2018.
//

#include "core/core.h"
#include "resourceman.h"

namespace ResourceMan {

// resource managers are sort of singletons, we keep a static registry to save a full ptr per resource etc.
// intended usage pattern is to store the unique_ptr returned by create for the lifetime of the manager
// and it should be released last thing after all resource are finished with by reseting the unique_ptr
int const MaxResourceManagers = 4;
ResourceMan* s_resourceManagers[MaxResourceManagers];
int s_curResourceManagerCount = 0; // TODO keep a free list

ResourceMan::ResourceMan() {}
ResourceMan::~ResourceMan()
{
	assert(myIndex < s_curResourceManagerCount);
	assert(s_resourceManagers[myIndex] != nullptr);
	s_resourceManagers[s_curResourceManagerCount] = nullptr;
	// TODO reclaim the index
}

std::unique_ptr<ResourceMan> ResourceMan::Create()
{
	std::unique_ptr<ResourceMan> resourceMan(new ResourceMan());
	s_resourceManagers[s_curResourceManagerCount] = resourceMan.get();
	resourceMan->myIndex = s_curResourceManagerCount++;

	return resourceMan;
}
ResourceMan* ResourceMan::GetFromIndex(uint32_t index_)
{
	assert(index_ < s_curResourceManagerCount);
	assert(s_resourceManagers[index_] != nullptr);
	return s_resourceManagers[index_];
}

ResourceHandleBase ResourceMan::openBaseResourceByName(std::string const& name_)
{
	auto it = nameToResourceId.find(name_)

}
ResourceHandleBase ResourceMan::openBaseResourceByTypeAndId(uint32_t type_, uint64_t const id_)
{

}

} // end namespace