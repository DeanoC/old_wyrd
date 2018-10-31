#include "core/core.h"
#include "render/commandqueue.h"
#include "vulkan/texture.h"
#include "vulkan/encoder.h"
#include "vulkan/semaphore.h"

namespace Vulkan {

EncoderPool::EncoderPool(Device::Ptr device_,
						 VkCommandPool commandPool_,
						 CommandPoolVkVTable* commandPoolVkVTable_,
						 GraphicsCBVkVTable* graphicsCBVTable_, ComputeCBVkVTable* computeCBVTable_,
						 TransferCBVkVTable* transferCBVTable_) :
		weakDevice(device_),
		vulkanDevice(device_->getVkDevice()),
		commandPool(commandPool_),
		vtable(commandPoolVkVTable_),
		graphicsCBVTable(graphicsCBVTable_),
		computeCBVTable(computeCBVTable_),
		transferCBVTable(transferCBVTable_)
{
}

EncoderPool::~EncoderPool()
{
	auto device = weakDevice.lock();
	if(device)
	{
		device->destroyCommandPool(commandPool);
	}
}

auto EncoderPool::allocateEncoder(uint32_t encoderFlags_) -> Render::Encoder::Ptr
{
	VkCommandBufferAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = (encoderFlags_ & Encoder::CallableFlag) ?
						 VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer cb = VK_NULL_HANDLE;
	vtable->vkAllocateCommandBuffers(vulkanDevice, &allocateInfo, &cb);

	GraphicsCBVkVTable* gvt = (encoderFlags_ & Render::CommandQueue::RenderFlavour) ? graphicsCBVTable : nullptr;
	ComputeCBVkVTable* cvt = (encoderFlags_ & Render::CommandQueue::ComputeFlavour) ? computeCBVTable : nullptr;
	TransferCBVkVTable* bvt = (encoderFlags_ & Render::CommandQueue::BlitFlavour) ? transferCBVTable : nullptr;

	auto encoder = std::make_shared<Vulkan::Encoder>(*this, encoderFlags_, cb, gvt, cvt, bvt);
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

Encoder::Encoder(EncoderPool& owner_,
				 uint32_t encodeFlags_,
				 VkCommandBuffer commandBuffer_,
				 GraphicsCBVkVTable* graphicsVTable_,
				 ComputeCBVkVTable* computeVTable_,
				 TransferCBVkVTable* transferVTable_) :
		owner(owner_),
		Render::Encoder(encodeFlags_),
		commandBuffer(commandBuffer_),
		renderEncoder(owner_, commandBuffer_, graphicsVTable_),
		computeEncoder(owner_, commandBuffer_, computeVTable_),
		blitEncoder(owner_, commandBuffer_, transferVTable_)
{
	vtable = graphicsVTable_ ? (GeneralCBVkVTable*) graphicsVTable_ : (computeVTable_
																	   ? (GeneralCBVkVTable*) computeVTable_
																	   : (GeneralCBVkVTable*) transferVTable_);
	assert(vtable);

}

Encoder::~Encoder()
{
	owner.destroyEncoder(this);
}

auto Encoder::asRenderEncoder() -> Render::IRenderEncoder*
{
	if(canEncodeRenderCommands())
	{
		assert(renderEncoder.vtable != nullptr);
		return &renderEncoder;
	} else return nullptr;
};

auto Encoder::asComputeEncoder() -> Render::IComputeEncoder*
{
	if(canEncodeComputeCommands())
	{
		assert(computeEncoder.vtable != nullptr);
		return &computeEncoder;
	} else return nullptr;
}

auto Encoder::asBlitEncoder() -> Render::IBlitEncoder*
{
	if(canEncodeBlitCommands())
	{
		assert(blitEncoder.vtable != nullptr);
		return &blitEncoder;
	} else return nullptr;
}

auto Encoder::begin(std::shared_ptr<Render::Semaphore> const& semaphore_) -> void
{
	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // TODO reuseable command buffers
	beginInfo.pInheritanceInfo = nullptr; // TODO support secondary buffers
	vkBeginCommandBuffer(&beginInfo);
	beginSemaphore = std::static_pointer_cast<Semaphore>(semaphore_);
}

auto Encoder::end(std::shared_ptr<Render::Semaphore> const& semaphore_) -> void
{
	vkEndCommandBuffer();
	endSemaphore = std::static_pointer_cast<Semaphore>(semaphore_);
}

auto Encoder::reset() -> void
{
	assert(hasPooledReset() == false);
	vkResetCommandBuffer(VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}

auto RenderEncoder::clearTexture(std::shared_ptr<Render::Texture> const& texture_,
								 std::array<float_t, 4> const& floats_) -> void
{
	Texture* texture = texture_->getStage<Texture>(Texture::s_stage);

	if(texture->entireRange.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
	{
		VkClearColorValue colour;
		std::memcpy(&colour.float32, floats_.data(), floats_.size() * sizeof(float));
		vkCmdClearColorImage(texture->image, VK_IMAGE_LAYOUT_GENERAL, &colour, 1, &texture->entireRange);
	} else
	{
		VkClearDepthStencilValue ds;
		ds.depth = floats_[0];
		ds.stencil = (uint32_t) floats_[1]; // this only works upto 23 integer bits but stencil cap at 8 so..
		vkCmdClearDepthStencilImage(texture->image, VK_IMAGE_LAYOUT_GENERAL, &ds, 1, &texture->entireRange);
	}
}

auto RenderEncoder::blit(std::shared_ptr<Render::Texture> const& src_,
						 std::shared_ptr<Render::Texture> const& dst_) -> void
{
	auto src = src_->getStage<Texture>(Texture::s_stage);
	auto dst = src_->getStage<Texture>(Texture::s_stage);

	VkImageBlit blitter =
			{{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
			 {0,                         0, 0},
			 {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
			 {0,                         0, 0},};

	vkCmdBlitImage(src->image, src->imageLayout, dst->image, dst->imageLayout,
				   1, &blitter, VK_FILTER_LINEAR);
}

auto RenderEncoder::beginRenderPass() -> void
{
	VkRenderPassBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.clearValueCount = 0;
	beginInfo.pClearValues = nullptr;
	// 	beginInfo.framebuffer
}

auto RenderEncoder::endRenderPass() -> void
{

}

auto ComputeEncoder::clearTexture(std::shared_ptr<Render::Texture> const& texture_,
								  std::array<float_t, 4> const& floats_) -> void
{
	Texture::Ptr texture = std::static_pointer_cast<Texture>(texture);

	// compute encoders can't clear depth/stencil images
	assert(texture->entireRange.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT);
	VkClearColorValue colour;
	std::memcpy(&colour.float32, floats_.data(), floats_.size() * sizeof(float));
	vkCmdClearColorImage(texture->image, VK_IMAGE_LAYOUT_GENERAL, &colour, 1, &texture->entireRange);
}


auto RenderEncoder::blitDisplay(std::shared_ptr<Render::Texture> const& src_, VkImage display_) -> void
{
	auto src = src_->getStage<Texture>(Texture::s_stage);

	VkImageBlit blitter =
			{{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
			 {0,                         0, 0},
			 {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
			 {0,                         0, 0},};

	vkCmdBlitImage(src->image, src->imageLayout, display_, VK_IMAGE_LAYOUT_GENERAL,
				   1, &blitter, VK_FILTER_NEAREST);
}

}