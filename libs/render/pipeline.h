#pragma once
#ifndef WYRD_RENDER_PIPELINE_H
#define WYRD_RENDER_PIPELINE_H

#include "core/core.h"
#include "render/resources.h"
#include "render/types.h"
#include "resourcemanager/resourcehandle.h"
#include "resourcemanager/resource.h"

namespace ResourceManager {
class ResourceMan;

struct ResourceNameView;
}

namespace Render {

enum class RenderPipelineFlags : uint8_t
{
	None = 0,
	EnablePrimitiveRestart = Core::Bit(0),
};

constexpr auto is_bitmask_enum(RenderPipelineFlags) -> bool { return true; }

struct alignas(8) RenderPipeline : public ResourceManager::Resource<RenderPipelineId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	// the shader array passed to Create should be only the shader used.
	// 0 shaders = illegal
	// 1 shader = vertex only, discard flag required
	// 2 shaders = vertex + fragment
	// 3 shaders =  vertex + geometry + fragment
	// 4 shaders = vertex + tess control + tess eval + fragment
	// 5 shaders = vertex + tess control + tess eval + geometry + fragment
	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			Topology topology_,
			RenderPipelineFlags flags_,
			std::vector<BindingTableMemoryMapHandle> const& memoryMap_,
			std::vector<SPIRVShaderHandle> const& shaders_,
			RasterisationStateHandle rasterisationState_,
			RenderPassHandle renderPass_,
			ROPBlenderHandle ropBlender_,
			ViewportHandle viewport_,
			VertexInputHandle vertexInput_) -> RenderPipelineHandle;

	auto isPrimitiveRestartEnabled() const
	{
		return Core::bitmask::test_equal(flags, RenderPipelineFlags::EnablePrimitiveRestart);
	}

	BindingTableMemoryMapHandle const* getBindingTableMemoryMaps() const
	{
		return (BindingTableMemoryMapHandle*) (this + 1);
	}


	Topology inputTopology;
	RenderPipelineFlags flags;
	uint8_t numBindingTableMemoryMaps;
	uint8_t numShaders;

	SPIRVShaderHandle shaders[5];
	RasterisationStateHandle rasterisationState;
	RenderPassHandle renderPass;
	ROPBlenderHandle ropBlender;
	ViewportHandle viewport;
	VertexInputHandle vertexInput;

};

struct alignas(8) ComputePipeline : public ResourceManager::Resource<ComputePipelineId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			SPIRVShaderHandle computeShader) -> ComputePipeline;

	SPIRVShaderHandle computeShader;
};

}

#endif //WYRD_PIPELINE_H
