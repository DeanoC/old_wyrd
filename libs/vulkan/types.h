#pragma once

#ifndef WYRD_VULKAN_TYPES_H
#define WYRD_VULKAN_TYPES_H


#include "render/types.h"
#include "vulkan/api.h"

constexpr auto is_bitmask_enum(::VkColorComponentFlagBits) -> bool { return true; }

namespace Vulkan {
constexpr auto from(Render::MemoryAccess const in_) -> uint32_t
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

constexpr auto from(Render::RenderPipelineStages const in_) -> uint32_t
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

constexpr auto from(Render::ComputePipelineStages const in_) -> uint32_t
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

constexpr auto from(Render::DMAPipelineStages const in_) -> uint32_t
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

constexpr auto from(Render::HostPipelineStages const in_) -> uint32_t
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

constexpr auto from(Render::Topology const in_)
{
	using namespace Render;
	switch(in_)
	{
		case Topology::Points:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case Topology::Lines:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case Topology::Triangles:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case Topology::Patches:
			return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		case Topology::LinesWithAdjacecy:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
		case Topology::TrianglesWithAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		case Topology::LineStrips:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case Topology::TriangleStrips:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case Topology::LineStripsWithAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
		case Topology::TriangleStripsWithAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		case Topology::TriangleFans:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		default:
			assert(false);
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
}

constexpr auto from(Render::BindingTableType const& in_)
{
	using namespace Render;
	switch(in_)
	{
		case BindingTableType::Texture:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case BindingTableType::RWTexture:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case BindingTableType::Buffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case BindingTableType::RWBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case BindingTableType::TextureBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case BindingTableType::RWTextureBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case BindingTableType::DynamicBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		case BindingTableType::DynamicRWBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		case BindingTableType::Sampler:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		default:
			assert(false);
			return VK_DESCRIPTOR_TYPE_SAMPLER;
	}
}

constexpr auto from(Render::ROPLogicOps const in_) -> VkLogicOp
{
	using namespace Render;

	switch(in_)
	{
		case ROPLogicOps::Dest:
			return VK_LOGIC_OP_NO_OP;
		case ROPLogicOps::Src:
			return VK_LOGIC_OP_COPY;
		case ROPLogicOps::NSrc:
			return VK_LOGIC_OP_COPY_INVERTED;
		case ROPLogicOps::Zero:
			return VK_LOGIC_OP_CLEAR;
		case ROPLogicOps::NZero:
			return VK_LOGIC_OP_SET;
		case ROPLogicOps::Or:
			return VK_LOGIC_OP_OR;
		case ROPLogicOps::OrNSrc:
			return VK_LOGIC_OP_OR_INVERTED;
		case ROPLogicOps::OrNDest:
			return VK_LOGIC_OP_OR_REVERSE;
		case ROPLogicOps::And:
			return VK_LOGIC_OP_AND;
		case ROPLogicOps::AndNSrc:
			return VK_LOGIC_OP_AND_INVERTED;
		case ROPLogicOps::AndNDest:
			return VK_LOGIC_OP_AND_REVERSE;
		case ROPLogicOps::Xor:
			return VK_LOGIC_OP_XOR;
		case ROPLogicOps::Not:
			return VK_LOGIC_OP_INVERT;
		case ROPLogicOps::Nor:
			return VK_LOGIC_OP_NOR;
		case ROPLogicOps::Nand:
			return VK_LOGIC_OP_NAND;
		case ROPLogicOps::NXor:
			return VK_LOGIC_OP_EQUIVALENT;
		default:
			assert(false);
			return VK_LOGIC_OP_AND;

	}
}

constexpr auto from(Render::ROPBlendOps const in_) -> VkBlendOp
{
	using namespace Render;

	switch(in_)
	{
		case ROPBlendOps::Add:
			return VK_BLEND_OP_ADD;
		case ROPBlendOps::Sub:
			return VK_BLEND_OP_SUBTRACT;
		case ROPBlendOps::ReverseSub:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case ROPBlendOps::Min:
			return VK_BLEND_OP_MIN;
		case ROPBlendOps::Max:
			return VK_BLEND_OP_MAX;
		default:
			assert(false);
			return VK_BLEND_OP_ADD;
	}
}

constexpr auto from(Render::ROPBlendFactor const in_) -> VkBlendFactor
{
	using namespace Render;

	bool invert = false;

	if(uint8_t(in_) & uint8_t(ROPBlendFactor::InvertedBit))
	{
		assert(in_ != ROPBlendFactor::SrcAlphaSaturate);
		invert = true;
	}

	switch(in_)
	{
		case ROPBlendFactor::Zero:
			return invert ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ZERO;
		case ROPBlendFactor::One:
			return invert ? VK_BLEND_FACTOR_ZERO : VK_BLEND_FACTOR_ONE;
		case ROPBlendFactor::SrcColour:
			return VkBlendFactor(VK_BLEND_FACTOR_SRC_COLOR + invert);
		case ROPBlendFactor::DstColour:
			return VkBlendFactor(VK_BLEND_FACTOR_DST_COLOR + invert);
		case ROPBlendFactor::SrcAlpha:
			return VkBlendFactor(VK_BLEND_FACTOR_SRC_ALPHA + invert);
		case ROPBlendFactor::DstAlpha:
			return VkBlendFactor(VK_BLEND_FACTOR_DST_COLOR + invert);
		case ROPBlendFactor::ConstantColour:
			return VkBlendFactor(VK_BLEND_FACTOR_CONSTANT_COLOR + invert);
		case ROPBlendFactor::ConstantAlpha:
			return VkBlendFactor(VK_BLEND_FACTOR_CONSTANT_ALPHA + invert);
		case ROPBlendFactor::Src1Colour:
			return VkBlendFactor(VK_BLEND_FACTOR_SRC1_COLOR + invert);
		case ROPBlendFactor::Src1Alpha:
			return VkBlendFactor(VK_BLEND_FACTOR_SRC1_COLOR + invert);
		case ROPBlendFactor::SrcAlphaSaturate:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		default:
			assert(false);
			return VK_BLEND_FACTOR_ONE;
	}
}

constexpr auto from(Render::ColourComponents const in_) -> uint32_t
{
	using namespace Render;
	using namespace Core::bitmask;

	if(in_ == ColourComponents::Red)
	{
		return (VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT);
	}

	uint32_t ret = 0;
	ret |= test_equal(in_, ColourComponents::Red) ? VK_COLOR_COMPONENT_R_BIT : 0;
	ret |= test_equal(in_, ColourComponents::Green) ? VK_COLOR_COMPONENT_G_BIT : 0;
	ret |= test_equal(in_, ColourComponents::Blue) ? VK_COLOR_COMPONENT_B_BIT : 0;
	ret |= test_equal(in_, ColourComponents::Alpha) ? VK_COLOR_COMPONENT_A_BIT : 0;

	return ret;
}

constexpr auto from(Render::ShaderType const in_) -> uint32_t
{
	using namespace Render;
	using namespace Core::bitmask;

	uint32_t ret = 0;
	ret |= test_equal(in_, ShaderType::Vertex) ? VK_SHADER_STAGE_VERTEX_BIT : 0;
	ret |= test_equal(in_, ShaderType::TesselationControl) ? VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT : 0;
	ret |= test_equal(in_, ShaderType::TesselationEval) ? VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT : 0;
	ret |= test_equal(in_, ShaderType::Geometry) ? VK_SHADER_STAGE_GEOMETRY_BIT : 0;
	ret |= test_equal(in_, ShaderType::Fragment) ? VK_SHADER_STAGE_FRAGMENT_BIT : 0;
	ret |= test_equal(in_, ShaderType::Compute) ? VK_SHADER_STAGE_COMPUTE_BIT : 0;

	return ret;
}


} // end namespace Vulkan
#endif //WYRD_VULKAN_TYPES_H
