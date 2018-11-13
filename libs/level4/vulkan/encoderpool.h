#pragma once
#ifndef WYRD_VULKAN_ENCODERPOOL_H
#define WYRD_VULKAN_ENCODERPOOL_H

#include "core/core.h"
#include "render/encoder.h"
#include "vulkan/api.h"

namespace Vulkan {

struct Semaphore;
struct Encoder;
struct Device;

struct EncoderPool : public Render::EncoderPool
{
	EncoderPool(Device* device_,
				VkCommandPool commandPool_,
				CommandPoolVkVTable* commandPoolVTable_,
				GeneralCBVkVTable* generalCBVTable,
				GraphicsCBVkVTable* graphicsCBVTable_,
				ComputeCBVkVTable* computeCBVTable_);
	~EncoderPool() final;

	auto allocateEncoder(
			Render::EncoderFlag encoderFlags_ = Core::bitmask::zero<Render::EncoderFlag>()) -> Render::Encoder::Ptr final;
	auto reset() -> void final;
	auto destroyEncoder(Vulkan::Encoder* encoder_) -> void;

#define COMMANDPOOL_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable->name(device->getVkDevice(), commandPool, args...); }
#define COMMANDPOOL_VK_FUNC_EXT(name, extension) COMMANDPOOL_VK_FUNC(name)

#include "functionlist.inl"

	Device* device;
	VkCommandPool commandPool;

	GeneralCBVkVTable* generalCBVTable;
	GraphicsCBVkVTable* graphicsCBVTable;
	ComputeCBVkVTable* computeCBVTable;
	CommandPoolVkVTable* vtable;
};

}

#endif //WYRD_VULKAN_ENCODERPOOL_H
