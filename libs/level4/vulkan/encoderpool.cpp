#include "core/core.h"
#include "render/commandqueue.h"
#include "vulkan/texture.h"
#include "vulkan/semaphore.h"
#include "vulkan/encoder.h"
#include "vulkan/renderencoder.h"
#include "vulkan/computeencoder.h"
#include "vulkan/encoderpool.h"

namespace Vulkan {

EncoderPool::EncoderPool(Device* device_,
						 VkCommandPool commandPool_,
						 CommandPoolVkVTable* commandPoolVkVTable_,
						 GeneralCBVkVTable* generalCBVTable_,
						 GraphicsCBVkVTable* graphicsCBVTable_,
						 ComputeCBVkVTable* computeCBVTable_) :
		device(device_),
		commandPool(commandPool_),
		vtable(commandPoolVkVTable_),
		generalCBVTable(generalCBVTable_),
		graphicsCBVTable(graphicsCBVTable_),
		computeCBVTable(computeCBVTable_)
{
}

EncoderPool::~EncoderPool()
{
	device->destroyCommandPool(commandPool);
}

auto EncoderPool::allocateEncoder(Render::EncoderFlag encoderFlags_) -> Render::Encoder::Ptr
{
	using namespace Core;

	VkCommandBufferAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = bool(encoderFlags_ & Render::EncoderFlag::Callable) ?
						 VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer cb = VK_NULL_HANDLE;
	vtable->vkAllocateCommandBuffers(device->getVkDevice(), &allocateInfo, &cb);

	GraphicsCBVkVTable* gvt = bool(encoderFlags_ & Render::EncoderFlag::RenderEncoder) ? graphicsCBVTable : nullptr;
	ComputeCBVkVTable* cvt = bool(encoderFlags_ & Render::EncoderFlag::ComputeEncoder) ? computeCBVTable : nullptr;

	auto encoder = std::make_shared<Vulkan::Encoder>(*this, encoderFlags_, cb, generalCBVTable, gvt, cvt);
	return std::static_pointer_cast<Render::Encoder>(encoder);
}

auto EncoderPool::destroyEncoder(Vulkan::Encoder* encoder_) -> void
{
	if(encoder_ == nullptr) return;
	vkFreeCommandBuffers(1, &encoder_->commandBuffer);
}

void EncoderPool::reset()
{
	vkResetCommandPool(VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}

}