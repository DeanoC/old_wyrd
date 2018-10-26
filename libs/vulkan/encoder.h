#pragma once
#ifndef WYRD_VULKAN_ENCODER_H
#define WYRD_VULKAN_ENCODER_H

#include "core/core.h"
#include "render/encoder.h"
#include "vulkan/api.h"
#include "device.h"

namespace Vulkan {

struct EncoderPool;

struct RenderEncoder : public Render::IRenderEncoder
{
	RenderEncoder(EncoderPool& owner_, VkCommandBuffer commandBuffer_, GraphicsCBVkVTable *graphicsCBVkVTable_) : owner(
			owner_), commandBuffer(commandBuffer_), vtable(graphicsCBVkVTable_) {}

	EncoderPool& owner;
	VkCommandBuffer commandBuffer;
	GraphicsCBVkVTable *vtable;
};

struct ComputeEncoder : public Render::IComputeEncoder
{
	ComputeEncoder(EncoderPool& owner_, VkCommandBuffer commandBuffer_, ComputeCBVkVTable *computeCBVkVTable_) : owner(
			owner_), commandBuffer(commandBuffer_), vtable(computeCBVkVTable_) {}

	EncoderPool& owner;
	VkCommandBuffer commandBuffer;
	ComputeCBVkVTable *vtable;
};

struct BlitEncoder : public Render::IBlitEncoder
{
	BlitEncoder(EncoderPool& owner_, VkCommandBuffer commandBuffer_, TransferCBVkVTable *transferCBVkVTable_) : owner(
			owner_), commandBuffer(commandBuffer_), vtable(transferCBVkVTable_) {}

	EncoderPool& owner;
	VkCommandBuffer commandBuffer;
	TransferCBVkVTable *vtable;
};


struct Encoder : public Render::Encoder
{
	Encoder(EncoderPool& owner_, uint32_t encodeFlags_, VkCommandBuffer commandBuffer_,
			GraphicsCBVkVTable *graphicsVTable_, ComputeCBVkVTable *computeVTable_,
			TransferCBVkVTable *transferVTable_);
	~Encoder() override;

	Render::IRenderEncoder *asRenderEncoder() final
	{
		if(canEncodeRenderCommands())
		{
			assert(renderEncoder.vtable != nullptr);
			return &renderEncoder;
		} else return nullptr;
	};

	Render::IComputeEncoder *asComputeEncoder() final
	{
		if(canEncodeComputeCommands())
		{
			assert(computeEncoder.vtable != nullptr);
			return &computeEncoder;
		} else return nullptr;
	}

	Render::IBlitEncoder *asBlitEncoder() final
	{
		if(canEncodeBlitCommands())
		{
			assert(blitEncoder.vtable != nullptr);
			return &blitEncoder;
		} else return nullptr;
	}

	auto reset() -> void final;

#define GENERAL_CB_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(commandBuffer, args...); }
#define GENERAL_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)

#include "functionlist.inl"

	EncoderPool& owner;
	VkCommandBuffer commandBuffer;
	GeneralCBVkVTable *vtable;

	RenderEncoder renderEncoder;
	ComputeEncoder computeEncoder;
	BlitEncoder blitEncoder;
};

struct EncoderPool : public Render::EncoderPool
{
	EncoderPool(VkDevice device_, VkCommandPool commandPool_, CommandPoolVkVTable *commandPoolVTable_,
				GraphicsCBVkVTable *graphicsCBVTable_, ComputeCBVkVTable *computeCBVTable_,
				TransferCBVkVTable *transferCBVTable_);

	auto allocateEncoder(uint32_t encoderFlags_) -> Render::Encoder::Ptr final;
	auto reset() -> void final;
	auto destroyEncoder(Vulkan::Encoder *encoder_) -> void;

#define COMMANDPOOL_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable->name(device, commandPool, args...); }
#define COMMANDPOOL_VK_FUNC_EXT(name, extension) COMMANDPOOL_VK_FUNC(name)

#include "functionlist.inl"

	VkDevice device;
	VkCommandPool commandPool;

	GraphicsCBVkVTable *graphicsCBVTable;
	ComputeCBVkVTable *computeCBVTable;
	TransferCBVkVTable *transferCBVTable;
	CommandPoolVkVTable *vtable;

};

}

#endif //WYRD_ENCODER_H
