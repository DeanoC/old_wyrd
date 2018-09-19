#pragma once
#ifndef WYRD_RESOURCEMAN_RESOURCEMAN_H
#define WYRD_RESOURCEMAN_RESOURCEMAN_H

#include "core/core.h"
#include "binny/bundle.h"
#include "core/freelist.h"
#include "resourceman/base.h"
#include "tbb/concurrent_unordered_map.h"
#include <iostream>
#include <string_view>

namespace ResourceMan {

struct IResourceFuncs
{
	using Ptr = std::shared_ptr<IResourceFuncs>;
	using InitFunc = Binny::Bundle::chunkFunc;
	using DestroyFunc = std::function<bool(void*)>;
	using SaveFunc = std::function<bool(std::ostream)>;

	virtual uint32_t getType() const = 0;
	virtual InitFunc getInitFunc() const = 0;
	virtual DestroyFunc getDestroyFunc() const = 0;
	virtual SaveFunc getSaveFunc() const = 0;
};

class ResourceMan
{
public:
	static std::unique_ptr<ResourceMan> Create();
	static ResourceMan* GetFromIndex(uint32_t index);

	~ResourceMan();
	void registerResourceType( IResourceFuncs::Ptr funcs_ );

	template<uint32_t type_>
	ResourceHandle<type_> openResourceByName(std::string const& name_);

	template<uint32_t type_>
	ResourceHandle<type_> openResourceById(uint64_t const id_);

private:
	ResourceHandleBase openBaseResourceByName(std::string_view const name_);
	ResourceHandleBase openBaseResourceByTypeAndId(uint32_t type_, uint64_t const id_);
	using NameToResourceId = tbb::concurrent_unordered_map<std::string, uint64_t>;

	ResourceMan();

	std::vector<IResourceFuncs::Ptr> resourceTypeFuncs;
	NameToResourceId nameToResourceId;

	uint32_t myIndex;
};

} // end namespace

#endif //WYRD_RESOURCEMAN_RESOURCEMAN_H
