#pragma once
#ifndef WYRD_RENDER_PIPELINE_H
#define WYRD_RENDER_PIPELINE_H

#include "core/core.h"
#include "render/resources.h"
#include "render/types.h"
#include "resourcemanager/resourcehandle.h"
#include "resourcemanager/resource.h"
#include "render/rasterisationstate.h"

namespace ResourceManager {
class ResourceMan;

struct ResourceNameView;
}

namespace Render {

struct alignas(8) RenderPipeline : public ResourceManager::Resource<RenderPipelineId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			RasterisationState const rasterisationState_,
			Topology topology_,
			uint8_t flags_,
			std::vector<BindingTableMemoryMapHandle> const& memoryMap_,
			SPIRVShaderHandle vertexShader_,
			SPIRVShaderHandle tesselationControlShader_,
			SPIRVShaderHandle tesselationEvalShader_,
			SPIRVShaderHandle geometryShader_,
			SPIRVShaderHandle fragmentShader_,
			RenderPassHandle renderPass_,
			ROPBlenderHandle ropBlender_,
			ViewportHandle viewport_,
			VertexInputHandle vertexInput_) -> RenderPipelineHandle;

	static constexpr uint8_t EnablePrimitiveRestartFlag = Core::Bit(0);

	BindingTableMemoryMapHandle* getBindingTableMemoryMaps()
	{
		return (BindingTableMemoryMapHandle*) ((uint8_t*) (this + 1) + bindingTableMemoryMapsOffset);
	}

	RasterisationState rasterisationState;

	Topology inputTopology;
	uint8_t flags;
	uint8_t numBindingTableMemoryMaps;
	uint8_t padd;
	uint32_t bindingTableMemoryMapsOffset;

	SPIRVShaderHandle vertexShader;
	SPIRVShaderHandle tesselationControlShader;
	SPIRVShaderHandle tesselationEvalShader;
	SPIRVShaderHandle geometryShader;
	SPIRVShaderHandle fragmentShader;
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
