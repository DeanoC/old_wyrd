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

struct alignas(8)    BindingTableMemoryMap : public ResourceManager::Resource<BindingTableMemoryMapId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(std::shared_ptr<ResourceManager::ResourceMan> rm_,
					   ResourceManager::ResourceNameView const& name_,
					   std::vector<BindingLayout> const& bindingLayouts_
	) -> BindingTableMemoryMapHandle;

	BindingLayout const* getBindingLayouts() const { return (BindingLayout*) (this + 1); }

	uint8_t numBindingLayouts;
};

// resource stages of BindingTable have to implement IGpuBindingTable
struct IGpuBindingTable
{
	virtual ~IGpuBindingTable() = default;

	virtual auto update(uint8_t memoryMapIndex_,
						uint32_t bindingIndex_,
						Render::TextureHandle const& texture_) -> void = 0;

	virtual auto update(uint8_t memoryMapIndex_,
						uint32_t bindingIndex_,
						Render::SamplerHandle const& sampler_) -> void = 0;

	virtual auto update(uint8_t memoryMapIndex_,
						uint32_t bindingIndex_,
						Render::TextureHandle const& texture_,
						Render::SamplerHandle const& sampler_) -> void = 0;

	virtual auto update(uint8_t memoryMapIndex_,
						uint32_t bindingIndex_,
						std::vector<Render::TextureHandle> const& textures_) -> void = 0;
};

struct BindingTable : public ResourceManager::Resource<BindingTableId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(std::shared_ptr<ResourceManager::ResourceMan> rm_,
					   ResourceManager::ResourceNameView const& name_,
					   std::vector<BindingTableMemoryMapHandle> const& bindingTables_) -> BindingTableHandle;

	BindingTableMemoryMapHandle* getMemoryMaps() { return (BindingTableMemoryMapHandle*) (this + 1); }

	#define INTERFACE_THUNK(name) \
    template<typename... Args> auto name(Args... args) const { \
        for(auto i = 0u; i < getStageCount(); ++i) \
        { auto iptr = getStage<IGpuBindingTable>(i+1); \
            assert(iptr != nullptr); return iptr->name(args...);  } }

	INTERFACE_THUNK(update);

#undef INTERFACE_THUNK

	uint8_t numMemoryMaps;
};

}

#endif //WYRD_RENDER_BINDINGTABLE_H
