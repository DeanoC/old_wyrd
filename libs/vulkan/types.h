#pragma once

#ifndef WYRD_VULKAN_TYPES_H
#define WYRD_VULKAN_TYPES_H

#include "render/types.h"
#include "vulkan/api.h"

namespace Vulkan {
constexpr auto fromMemoryAccess(Render::MemoryAccess const in_) -> uint32_t
{
	using namespace Render;
	using namespace Core::bitmask;

	uint32_t ret = 0;
	ret |= test_equal(in_, MemoryAccess::IndirectCommandRead) ? VK_ACCESS_INDIRECT_COMMAND_READ_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::IndexRead) ? VK_ACCESS_INDEX_READ_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::VertexRead) ? VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::ConstantRead) ? VK_ACCESS_UNIFORM_READ_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::InputAttachmentRead) ? VK_ACCESS_INPUT_ATTACHMENT_READ_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::ShaderRead) ? VK_ACCESS_SHADER_READ_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::ShaderWrite) ? VK_ACCESS_SHADER_WRITE_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::ColourRopRead) ? VK_ACCESS_COLOR_ATTACHMENT_READ_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::ColourRopWrite) ? VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::DepthStencilRopRead) ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::DepthStencilRopWrite) ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::DMARead) ? VK_ACCESS_TRANSFER_READ_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::DMAWrite) ? VK_ACCESS_TRANSFER_WRITE_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::CpuRead) ? VK_ACCESS_HOST_READ_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::CpuWrite) ? VK_ACCESS_HOST_WRITE_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::GeneralRead) ? VK_ACCESS_MEMORY_READ_BIT : 0;
	ret |= test_equal(in_, MemoryAccess::GeneralWrite) ? VK_ACCESS_MEMORY_WRITE_BIT : 0;

	return ret;
}

constexpr auto fromRenderPipelineStage(Render::RenderPipelineStages const in_) -> uint32_t
{
	using namespace Render;
	using namespace Core::bitmask;

	// special case all and allgfx
	if(test_equal(in_, RenderPipelineStages::AllGfx))
	{
		return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	}

	if(test_equal(in_, RenderPipelineStages::All))
	{
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	}

	uint32_t ret = 0;
	ret |= test_equal(in_, RenderPipelineStages::Begin) ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : 0;
	ret |= test_equal(in_, RenderPipelineStages::DrawIndirect) ? VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT : 0;
	ret |= test_equal(in_, RenderPipelineStages::VertexInput) ? VK_PIPELINE_STAGE_VERTEX_INPUT_BIT : 0;
	ret |= test_equal(in_, RenderPipelineStages::VertexShader) ? VK_PIPELINE_STAGE_VERTEX_SHADER_BIT : 0;
	ret |= test_equal(in_, RenderPipelineStages::TessellationControlShader)
		   ? VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT : 0;
	ret |= test_equal(in_, RenderPipelineStages::TesselationEvalShader)
		   ? VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT : 0;
	ret |= test_equal(in_, RenderPipelineStages::GeometryShader) ? VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT : 0;
	ret |= test_equal(in_, RenderPipelineStages::FragmentShader) ? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT : 0;
	ret |= test_equal(in_, RenderPipelineStages::EarlyFragmentTests) ? VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT : 0;
	ret |= test_equal(in_, RenderPipelineStages::ROPWrite) ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT : 0;
	ret |= test_equal(in_, RenderPipelineStages::LateFragmentTests) ? VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT : 0;
	ret |= test_equal(in_, RenderPipelineStages::End) ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : 0;

	return ret;
}

constexpr auto fromComputePipelineStage(Render::ComputePipelineStages const in_) -> uint32_t
{
	using namespace Render;
	using namespace Core::bitmask;

	// special case all
	if(test_equal(in_, ComputePipelineStages::All))
	{
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	}

	uint32_t ret = 0;
	ret |= test_equal(in_, ComputePipelineStages::Begin) ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : 0;
	ret |= test_equal(in_, ComputePipelineStages::ComputeShader) ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : 0;
	ret |= test_equal(in_, ComputePipelineStages::End) ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : 0;

	return ret;
}

constexpr auto fromDMAPipelineStage(Render::DMAPipelineStages const in_) -> uint32_t
{
	using namespace Render;
	using namespace Core::bitmask;

	// special case all
	if(test_equal(in_, DMAPipelineStages::All))
	{
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	}

	uint32_t ret = 0;
	ret |= test_equal(in_, DMAPipelineStages::Begin) ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : 0;
	ret |= test_equal(in_, DMAPipelineStages::DMA) ? VK_PIPELINE_STAGE_TRANSFER_BIT : 0;
	ret |= test_equal(in_, DMAPipelineStages::End) ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : 0;

	return ret;
}

constexpr auto fromHostPipelineStage(Render::HostPipelineStages const in_) -> uint32_t
{
	using namespace Render;
	using namespace Core::bitmask;

	// special case all
	if(test_equal(in_, HostPipelineStages::All))
	{
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	}

	uint32_t ret = 0;
	ret |= test_equal(in_, HostPipelineStages::Host) ? VK_PIPELINE_STAGE_HOST_BIT : 0;

	return ret;
}

}
#endif //WYRD_VULKAN_TYPES_H
