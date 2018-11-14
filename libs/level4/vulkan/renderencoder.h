#pragma once
#ifndef WYRD_VULKAN_RENDERENCODER_H
#define WYRD_VULKAN_RENDERENCODER_H

#include "core/core.h"
#include "render/encoder.h"
#include "vulkan/api.h"
#include "vulkan/device.h"

namespace Vulkan {

struct EncoderPool;

struct RenderEncoder : public Render::IRenderEncoder
{
	RenderEncoder(EncoderPool& owner_, VkCommandBuffer commandBuffer_, GraphicsCBVkVTable* graphicsCBVkVTable_) : owner(
			owner_), commandBuffer(commandBuffer_), vtable(graphicsCBVkVTable_) {}

	auto clearTexture(Render::TextureConstPtr const& texture_, std::array<float_t, 4> const& floats_) -> void final;
	auto beginRenderPass(
			Render::RenderPassConstPtr const& renderPass_,
			Render::RenderTargetConstPtr const& renderTarget_
	) -> void final;
	auto endRenderPass() -> void final;
	auto blit(Render::TextureConstPtr const& src_, Render::TextureConstPtr const& dst_) -> void final;

	auto bind(Render::RenderPipelineConstPtr const& pipeline_, Render::BindingTableConstPtr const& bindingTable_) -> void final;
	auto bind(Render::RenderPipelineConstPtr const& pipeline_) -> void final;
	auto bindVertexBuffer(Render::BufferConstPtr const& buffer_, uint64_t offset_ = 0,
						  uint32_t bindingIndex = 0) -> void final;
	auto bindIndexBuffer(Render::BufferConstPtr const& buffer_,
						 uint8_t bitSize_ = 16u, uint64_t offset_ = 0) -> void final;
	auto pushConstants(Render::RenderPipelineConstPtr const& pipeline_, Render::PushConstantRange const& range,
					   void const* data_) -> void final;

	auto draw(uint32_t vertexCount_, uint32_t vertexOffset_, uint32_t instanceCount_,
			  uint32_t instanceOffset_) -> void final;
	auto drawIndexed(uint32_t indexCount_, uint32_t indexOffset_, uint32_t vertexOffset,
					 uint32_t instanceCount_, uint32_t instanceOffset_) -> void final;

	auto setDynamicViewport(uint32_t viewportIndex_, Render::ViewportDef const& viewport_) -> void final;
	auto setDynamicScissor(uint32_t viewportIndex_, Render::Scissor const& scissor_) -> void final;

#define GENERAL_CB_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(commandBuffer, args...); }
#define GENERAL_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)
#define GFXCOMP_CB_VK_FUNC(name) GENERAL_CB_VK_FUNC(name)
#define GFXCOMP_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)
#define GRAPHIC_CB_VK_FUNC(name) GENERAL_CB_VK_FUNC(name)
#define GRAPHIC_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)

#include "functionlist.inl"

	// special case for the vulkan swap chain
	auto resolveForDisplay(Render::TextureConstPtr const& src_,
						   uint32_t width_, uint32_t height_,
						   VkImage display_) -> void;

	EncoderPool& owner;
	VkCommandBuffer commandBuffer;
	GraphicsCBVkVTable* vtable;
};

}

#endif //WYRD_VULAKN_RENDERENCODER_H
