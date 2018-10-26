#include "core/core.h"
#include "vulkan/encoder.h"
#include "render/commandqueue.h"

namespace Vulkan {

EncoderPool::EncoderPool(VkDevice device_, VkCommandPool commandPool_, CommandPoolVkVTable *commandPoolVkVTable_,
						 GraphicsCBVkVTable *graphicsCBVTable_, ComputeCBVkVTable *computeCBVTable_,
						 TransferCBVkVTable *transferCBVTable_)
		: device(device_), commandPool(commandPool_), vtable(commandPoolVkVTable_), graphicsCBVTable(graphicsCBVTable_),
		  computeCBVTable(computeCBVTable_), transferCBVTable(transferCBVTable_)
{
}

auto EncoderPool::allocateEncoder(uint32_t encoderFlags_) -> Render::Encoder::Ptr
{
	VkCommandBufferAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = (encoderFlags_ & Encoder::PrimaryFlag) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY
																: VK_COMMAND_BUFFER_LEVEL_SECONDARY;

	VkCommandBuffer cb = VK_NULL_HANDLE;
	vtable->vkAllocateCommandBuffers(device, &allocateInfo, &cb);

	GraphicsCBVkVTable *gvt = (encoderFlags_ & Render::CommandQueue::RenderFlavour) ? graphicsCBVTable : nullptr;
	ComputeCBVkVTable *cvt = (encoderFlags_ & Render::CommandQueue::ComputeFlavour) ? computeCBVTable : nullptr;
	TransferCBVkVTable *bvt = (encoderFlags_ & Render::CommandQueue::BlitFlavour) ? transferCBVTable : nullptr;

	auto encoder = std::make_shared<Vulkan::Encoder>(*this, encoderFlags_, cb, gvt, cvt, bvt);
	return std::static_pointer_cast<Render::Encoder>(encoder);

}

auto EncoderPool::destroyEncoder(Vulkan::Encoder *encoder_) -> void
{
	if(encoder_ == nullptr) return;
	vkFreeCommandBuffers(1, &encoder_->commandBuffer);
}

void EncoderPool::reset()
{
	vkResetCommandPool(VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}

Encoder::Encoder(EncoderPool& owner_, uint32_t encodeFlags_, VkCommandBuffer commandBuffer_,
				 GraphicsCBVkVTable *graphicsVTable_, ComputeCBVkVTable *computeVTable_,
				 TransferCBVkVTable *transferVTable_)
		: owner(owner_), Render::Encoder(encodeFlags_), commandBuffer(commandBuffer_),
		  renderEncoder(owner_, commandBuffer_, graphicsVTable_),
		  computeEncoder(owner_, commandBuffer_, computeVTable_), blitEncoder(owner_, commandBuffer_, transferVTable_)
{
	vtable = graphicsVTable_ ? (GeneralCBVkVTable *) graphicsVTable_ : (computeVTable_
																		? (GeneralCBVkVTable *) computeVTable_
																		: (GeneralCBVkVTable *) transferVTable_);
	assert(vtable);

}

Encoder::~Encoder()
{
	owner.destroyEncoder(this);
}

auto Encoder::reset() -> void
{
	assert(hasPooledReset() == false);
	vkResetCommandBuffer(VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}

}