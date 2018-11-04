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

	auto clearTexture(std::shared_ptr<Render::Texture> const& texture_,
					  std::array<float_t, 4> const& floats_) -> void final;
	auto beginRenderPass() -> void final;
	auto endRenderPass() -> void final;
	auto blit(std::shared_ptr<Render::Texture> const& src_, std::shared_ptr<Render::Texture> const& dst_) -> void final;

#define GENERAL_CB_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(commandBuffer, args...); }
#define GENERAL_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)
#define GFXCOMP_CB_VK_FUNC(name) GENERAL_CB_VK_FUNC(name)
#define GFXCOMP_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)
#define GRAPHIC_CB_VK_FUNC(name) GENERAL_CB_VK_FUNC(name)
#define GFX_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)

#include "functionlist.inl"

	// special case for the vulkan swap chain
	auto resolveForDisplay(std::shared_ptr<Render::Texture> const& src_,
						   uint32_t width_, uint32_t height_,
						   VkImage display_) -> void;

	EncoderPool& owner;
	VkCommandBuffer commandBuffer;
	GraphicsCBVkVTable* vtable;
};

}

#endif //WYRD_VULAKN_RENDERENCODER_H
