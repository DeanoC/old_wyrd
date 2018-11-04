#pragma once
#ifndef WYRD_VULKAN_COMPUTEENCODER_H
#define WYRD_VULKAN_COMPUTEENCODER_H

#include "core/core.h"
#include "render/encoder.h"
#include "vulkan/api.h"
#include "vulkan/device.h"

namespace Vulkan {

struct EncoderPool;

struct ComputeEncoder : public Render::IComputeEncoder
{
	ComputeEncoder(EncoderPool& owner_, VkCommandBuffer commandBuffer_, ComputeCBVkVTable* computeCBVkVTable_) : owner(
			owner_), commandBuffer(commandBuffer_), vtable(computeCBVkVTable_) {}

	auto clearTexture(std::shared_ptr<Render::Texture> const& texture_,
					  std::array<float_t, 4> const& floats_) -> void final;

#define GENERAL_CB_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(commandBuffer, args...); }
#define GENERAL_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)
#define GFXCOMP_CB_VK_FUNC(name) GENERAL_CB_VK_FUNC(name)
#define GFXCOMP_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)
#define COMPUTE_CB_VK_FUNC(name) GENERAL_CB_VK_FUNC(name)
#define COMPUTE_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)

#include "functionlist.inl"

	EncoderPool& owner;
	VkCommandBuffer commandBuffer;
	ComputeCBVkVTable* vtable;
};

}

#endif //WYRD_VULKAN_COMPUTEENCODER_H
