#include "core/core.h"
#include "vulkan/encoder.h"
#include "render/commandqueue.h"

namespace Vulkan {

EncoderPool::EncoderPool(VkDevice device_, VkCommandPool commandPool_, DeviceVkVTable *deviceCBVTable_,
						 GraphicsCBVkVTable *graphicsCBVTable_, ComputeCBVkVTable *computeCBVTable_,
						 TransferCBVkVTable *transferCBVTable_)
		: device(device_), commandPool(commandPool_), deviceVkVTable(deviceCBVTable_),
		  graphicsCBVTable(graphicsCBVTable_), computeCBVTable(computeCBVTable_), transferCBVTable(transferCBVTable_)
{
}

auto EncoderPool::allocatePrimaryEncoder(uint32_t encoderFlags_) -> Render::Encoder::Ptr
{
	VkCommandBufferAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer cb = VK_NULL_HANDLE;
	deviceVkVTable->vkAllocateCommandBuffers(device, &allocateInfo, &cb);

	GraphicsCBVkVTable *gvt = (encoderFlags_ & Render::CommandQueue::RenderFlavour) ? graphicsCBVTable : nullptr;
	ComputeCBVkVTable *cvt = (encoderFlags_ & Render::CommandQueue::ComputeFlavour) ? computeCBVTable : nullptr;
	TransferCBVkVTable *bvt = (encoderFlags_ & Render::CommandQueue::BlitFlavour) ? transferCBVTable : nullptr;

	auto encoder = std::make_shared<Vulkan::Encoder>(*this, cb, gvt, cvt, bvt);
	return std::static_pointer_cast<Render::Encoder>(encoder);

}

Encoder::Encoder(EncoderPool& owner_, VkCommandBuffer commandBuffer_, GraphicsCBVkVTable *graphicsVTable,
				 ComputeCBVkVTable *computeVTable, TransferCBVkVTable *transferVTable)
		: owner(owner_), commandBuffer(commandBuffer_), renderEncoder(owner_, commandBuffer_, graphicsVTable),
		  computeEncoder(owner_, commandBuffer_, computeVTable), blitEncoder(owner_, commandBuffer_, transferVTable)
{
};

}