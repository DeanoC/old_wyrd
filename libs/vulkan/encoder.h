#pragma once
#ifndef WYRD_VULKAN_ENCODER_H
#define WYRD_VULKAN_ENCODER_H

#include "core/core.h"
#include "render/encoder.h"
#include "vulkan/api.h"
#include "vulkan/device.h"

namespace Vulkan {

struct EncoderPool;
struct Semaphore;
struct Fence;

struct RenderEncoder : public Render::IRenderEncoder
{
	RenderEncoder(EncoderPool& owner_, VkCommandBuffer commandBuffer_, GraphicsCBVkVTable* graphicsCBVkVTable_) : owner(
			owner_), commandBuffer(commandBuffer_), vtable(graphicsCBVkVTable_) {}

	auto clearTexture(std::shared_ptr<Render::Texture> const& texture_,
					  std::array<float_t, 4> const& floats_) -> void final;
	auto beginRenderPass() -> void final;
	auto endRenderPass() -> void final;

#define GENERAL_CB_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(commandBuffer, args...); }
#define GENERAL_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)
#define GFXCOMP_CB_VK_FUNC(name) GENERAL_CB_VK_FUNC(name)
#define GFXCOMP_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)
#define GRAPHIC_CB_VK_FUNC(name) GENERAL_CB_VK_FUNC(name)
#define GFX_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)

#include "functionlist.inl"

	EncoderPool& owner;
	VkCommandBuffer commandBuffer;
	GraphicsCBVkVTable* vtable;
};

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

struct BlitEncoder : public Render::IBlitEncoder
{
	BlitEncoder(EncoderPool& owner_, VkCommandBuffer commandBuffer_, TransferCBVkVTable* transferCBVkVTable_) : owner(
			owner_), commandBuffer(commandBuffer_), vtable(transferCBVkVTable_) {}

#define GENERAL_CB_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(commandBuffer, args...); }
#define GENERAL_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)

#include "functionlist.inl"

	EncoderPool& owner;
	VkCommandBuffer commandBuffer;
	TransferCBVkVTable* vtable;
};


struct Encoder : public Render::Encoder
{
	using Ptr = std::shared_ptr<Encoder>;
	using WeakPtr = std::weak_ptr<Encoder>;

	Encoder(EncoderPool& owner_, uint32_t encodeFlags_, VkCommandBuffer commandBuffer_,
			GraphicsCBVkVTable* graphicsVTable_, ComputeCBVkVTable* computeVTable_,
			TransferCBVkVTable* transferVTable_);

	~Encoder() final;
	auto asRenderEncoder() -> Render::IRenderEncoder* final;
	auto asComputeEncoder() -> Render::IComputeEncoder* final;
	auto asBlitEncoder() -> Render::IBlitEncoder* final;
	auto begin(std::shared_ptr<Render::Semaphore> const& semaphore_) -> void final;
	auto end(std::shared_ptr<Render::Semaphore> const& semaphore_) -> void final;
	auto reset() -> void final;

#define GENERAL_CB_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(commandBuffer, args...); }
#define GENERAL_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)

#include "functionlist.inl"

	EncoderPool& owner;
	VkCommandBuffer commandBuffer;

	GeneralCBVkVTable* vtable;
	RenderEncoder renderEncoder;
	ComputeEncoder computeEncoder;
	BlitEncoder blitEncoder;

	std::shared_ptr<Semaphore> beginSemaphore; // optional
	std::shared_ptr<Semaphore> endSemaphore; // optional
};

struct EncoderPool : public Render::EncoderPool
{
	EncoderPool(Device::Ptr device_, VkCommandPool commandPool_, CommandPoolVkVTable* commandPoolVTable_,
				GraphicsCBVkVTable* graphicsCBVTable_, ComputeCBVkVTable* computeCBVTable_,
				TransferCBVkVTable* transferCBVTable_);
	~EncoderPool() final;

	auto allocateEncoder(uint32_t encoderFlags_) -> Render::Encoder::Ptr final;
	auto reset() -> void final;
	auto destroyEncoder(Vulkan::Encoder* encoder_) -> void;

#define COMMANDPOOL_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable->name(vulkanDevice, commandPool, args...); }
#define COMMANDPOOL_VK_FUNC_EXT(name, extension) COMMANDPOOL_VK_FUNC(name)

#include "functionlist.inl"

	Device::WeakPtr weakDevice;
	VkDevice vulkanDevice;
	VkCommandPool commandPool;

	GraphicsCBVkVTable* graphicsCBVTable;
	ComputeCBVkVTable* computeCBVTable;
	TransferCBVkVTable* transferCBVTable;
	CommandPoolVkVTable* vtable;

};

}

#endif //WYRD_ENCODER_H
