#pragma once
#ifndef WYRD_VULKAN_ENCODER_H
#define WYRD_VULKAN_ENCODER_H

#include "core/core.h"
#include "render/encoder.h"
#include "vulkan/api.h"
#include "vulkan/device.h"
#include "vulkan/renderencoder.h"
#include "vulkan/computeencoder.h"

namespace Vulkan {

struct EncoderPool;
struct Semaphore;
struct Fence;

struct Encoder : public Render::Encoder
{
	using Ptr = std::shared_ptr<Encoder>;
	using WeakPtr = std::weak_ptr<Encoder>;

	Encoder(EncoderPool& owner_,
			Render::EncoderFlag encodeFlags_,
			VkCommandBuffer commandBuffer_,
			GeneralCBVkVTable* generalVTable_,
			GraphicsCBVkVTable* graphicsVTable_,
			ComputeCBVkVTable* computeVTable_);

	~Encoder() final;
	auto asRenderEncoder() -> Render::IRenderEncoder* final;
	auto asComputeEncoder() -> Render::IComputeEncoder* final;
	auto begin(std::shared_ptr<Render::Semaphore> const& semaphore_ = {}) -> void final;
	auto end(std::shared_ptr<Render::Semaphore> const& semaphore_ = {}) -> void final;
	auto reset() -> void final;
	auto copy(std::shared_ptr<Render::Texture> const& src_, std::shared_ptr<Render::Texture> const& dst_) -> void final;

	auto textureBarrier(std::shared_ptr<Render::Texture> const& texture_) -> void final;
	auto textureBarrier(
			Render::MemoryAccess waitAccess_,
			Render::MemoryAccess stallAccess_,
			std::shared_ptr<Render::Texture> const& texture_) -> void final;


	auto copy(VkImage srcImage_,
			  VkImageLayout srcLayout_,
			  VkImageSubresourceLayers const& srcExtents_,
			  std::shared_ptr<Render::Texture> const& dst_) -> void;
	auto textureBarrier(VkPipelineStageFlagBits srcStage_, VkPipelineStageFlagBits dstStage_,
						VkImageMemoryBarrier const& barrier_) -> void;


#define GENERAL_CB_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(commandBuffer, args...); }
#define GENERAL_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)
#include "functionlist.inl"

	EncoderPool& owner;
	VkCommandBuffer commandBuffer;

	GeneralCBVkVTable* vtable;
	RenderEncoder renderEncoder;
	ComputeEncoder computeEncoder;

	std::shared_ptr<Semaphore> beginSemaphore; // optional
	std::shared_ptr<Semaphore> endSemaphore; // optional
};

}

#endif //WYRD_ENCODER_H
