#pragma once
#ifndef WYRD_RENDER_BINDINGTABLE_H
#define WYRD_RENDER_BINDINGTABLE_H

#include "core/core.h"
#include "render/resources.h"
#include "render/types.h"
#include "resourcemanager/resource.h"
#include "resourcemanager/resourcehandle.h"
#include "resourcemanager/resourcename.h"

namespace ResourceManager { class ResourceMan; }

namespace Render {

struct BindingLayout
{
	BindingTableType type;
	uint8_t count;
	ShaderType shaderAccess;
};

struct Binding : public BindingLayout
{
	// use type to convert this back into the actual handle
	ResourceManager::ResourceHandleBase resource;
};

struct alignas(8) BindingTableMemoryMap : public ResourceManager::Resource<BindingTableMemoryMapId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(std::shared_ptr<ResourceManager::ResourceMan> rm_,
					   ResourceManager::ResourceNameView const& name_,
					   std::vector<BindingLayout> const& bindingLayouts) -> BindingTableMemoryMapHandle;

	BindingLayout* getBindingLayouts() { return (BindingLayout*) (this + 1); }
	uint8_t numBindings;
};

struct BindingTable : public ResourceManager::Resource<BindingTableId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(std::shared_ptr<ResourceManager::ResourceMan> rm_,
					   ResourceManager::ResourceNameView const& name_,
					   std::vector<BindingTableMemoryMapHandle> const& bindingTables) -> BindingTableHandle;

	BindingTableMemoryMapHandle* getMemoryMaps() { return (BindingTableMemoryMapHandle*) (this + 1); }

	uint8_t numMemoryMaps;
};

}

#endif //WYRD_RENDER_BINDINGTABLE_H
