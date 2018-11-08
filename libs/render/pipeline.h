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

struct alignas(8) RenderPipeline : public ResourceManager::Resource<RenderPipelineId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			Topology topology_,
			uint8_t flags_,
			SPIRVShaderHandle vertexShader_,
			SPIRVShaderHandle tesselationControlShader_,
			SPIRVShaderHandle tesselationEvalShader_,
			SPIRVShaderHandle geometryShader_,
			SPIRVShaderHandle fragmentShader_) -> RenderPipelineHandle;

	static constexpr uint8_t EnablePrimitiveRestartFlag = Core::Bit(0);

	Topology inputTopology;
	uint8_t flags;
	uint8_t padd[2];

	SPIRVShaderHandle vertexShader;
	SPIRVShaderHandle tesselationControlShader;
	SPIRVShaderHandle tesselationEvalShader;
	SPIRVShaderHandle geometryShader;
	SPIRVShaderHandle fragmentShader;
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
