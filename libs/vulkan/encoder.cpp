#include "core/core.h"
#include "render/commandqueue.h"
#include "vulkan/texture.h"
#include "vulkan/encoder.h"
#include "vulkan/semaphore.h"

namespace Vulkan {

EncoderPool::EncoderPool(Device::Ptr device_,
						 VkCommandPool commandPool_,
						 CommandPoolVkVTable* commandPoolVkVTable_,
						 GeneralCBVkVTable* generalCBVTable_,
						 GraphicsCBVkVTable* graphicsCBVTable_,
						 ComputeCBVkVTable* computeCBVTable_) :
		weakDevice(device_),
		vulkanDevice(device_->getVkDevice()),
		commandPool(commandPool_),
		vtable(commandPoolVkVTable_),
		generalCBVTable(generalCBVTable_),
		graphicsCBVTable(graphicsCBVTable_),
		computeCBVTable(computeCBVTable_)
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

auto EncoderPool::allocateEncoder(Render::EncoderFlag encoderFlags_) -> Render::Encoder::Ptr
{
	using namespace Core::bitmask;

	VkCommandBufferAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = bool(encoderFlags_ & Render::EncoderFlag::Callable) ?
						 VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer cb = VK_NULL_HANDLE;
	vtable->vkAllocateCommandBuffers(vulkanDevice, &allocateInfo, &cb);

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

Encoder::Encoder(EncoderPool& owner_,
				 Render::EncoderFlag encodeFlags_,
				 VkCommandBuffer commandBuffer_,
				 GeneralCBVkVTable* generalCBVkVTable_,
				 GraphicsCBVkVTable* graphicsVTable_,
				 ComputeCBVkVTable* computeVTable_) :
		owner(owner_),
		Render::Encoder(encodeFlags_),
		commandBuffer(commandBuffer_),
		vtable(generalCBVkVTable_),
		renderEncoder(owner_, commandBuffer_, graphicsVTable_),
		computeEncoder(owner_, commandBuffer_, computeVTable_)
{
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
	vkResetCommandBuffer(VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}

auto Encoder::copy(std::shared_ptr<Render::Texture> const& src_, std::shared_ptr<Render::Texture> const& dst_) -> void
{
	Texture* src = src_->getStage<Texture>(Texture::s_stage);

	VkImageSubresourceLayers srcLayers;
	srcLayers.aspectMask = src->entireRange.aspectMask;
	srcLayers.baseArrayLayer = src->entireRange.baseArrayLayer;
	srcLayers.layerCount = src->entireRange.layerCount;
	srcLayers.mipLevel = src_->mipLevels;

	copy(src->image, src->imageLayout, srcLayers, dst_);

}

auto Encoder::copy(VkImage srcImage_,
				   VkImageLayout srcLayout_,
				   VkImageSubresourceLayers const& srcExtents_,
				   std::shared_ptr<Render::Texture> const& dst_) -> void
{
	Texture* dst = dst_->getStage<Texture>(Texture::s_stage);
	assert(srcExtents_.aspectMask == dst->entireRange.aspectMask);
	assert(srcExtents_.layerCount == dst->entireRange.layerCount);

	uint32_t mipCount = std::min(dst_->mipLevels, srcExtents_.mipLevel);

	VkImageCopy imageCopy = {};
	imageCopy.srcSubresource = srcExtents_;
	imageCopy.dstSubresource.aspectMask = dst->entireRange.aspectMask;
	imageCopy.dstSubresource.baseArrayLayer = dst->entireRange.baseArrayLayer;
	imageCopy.dstSubresource.layerCount = dst->entireRange.layerCount;
	imageCopy.srcOffset = {0, 0, 0};
	imageCopy.dstOffset = {0, 0, 0};
	imageCopy.extent.width = dst_->width;
	imageCopy.extent.height = dst_->height;
	imageCopy.extent.depth = dst_->depth;

	VkImageCopy mipCopy[16]; // 16 mip maps would be crazy big texture
	for(auto i = 0u; i < mipCount; ++i)
	{
		mipCopy[i] = imageCopy;
		mipCopy[i].srcSubresource.mipLevel = i;
		mipCopy[i].dstSubresource.mipLevel = i;
	}

	vkCmdCopyImage(srcImage_, srcLayout_,
				   dst->image, dst->imageLayout,
				   mipCount, mipCopy);

}

auto Encoder::textureBarrier(std::shared_ptr<Render::Texture> const& texture_) -> void
{
}

auto Encoder::textureBarrier(VkPipelineStageFlagBits srcStage_, VkPipelineStageFlagBits dstStage_,
							 VkImageMemoryBarrier const& barrier_) -> void
{
	vkCmdPipelineBarrier(
			srcStage_,
			dstStage_,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier_);
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

	VkImageBlit blitter = {
			{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
			{{0, 0, 0},                 {(int32_t) src_->width, (int32_t) src_->height, 1}},
			{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
			{{0, 0, 0},                 {(int32_t) dst_->width, (int32_t) dst_->height, 1}},
	};

	vkCmdBlitImage(src->image,
				   src->imageLayout,
				   dst->image,
				   dst->imageLayout,
				   1,
				   &blitter,
				   VK_FILTER_LINEAR);
}

auto RenderEncoder::beginRenderPass() -> void
{
	VkRenderPassBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.clearValueCount = 0;
	beginInfo.pClearValues = nullptr;
	//		beginInfo.framebuffer
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


auto RenderEncoder::resolveForDisplay(
		std::shared_ptr<Render::Texture> const& src_,
		uint32_t width_, uint32_t height_,
		VkImage display_) -> void
{
	auto src = src_->getStage<Texture>(Texture::s_stage);

	if(src_->samples == 1)
	{
		VkImageBlit blitter = {
				{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
				{{0, 0, 0},                 {(int32_t) src_->width, (int32_t) src_->height, 1}},
				{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
				{{0, 0, 0},                 {(int32_t) width_,      (int32_t) height_,      1}},
		};

		vkCmdBlitImage(src->image,
					   src->imageLayout,
					   display_,
					   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					   1, &blitter, VK_FILTER_LINEAR);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = display_;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
	} else
	{
		// TODO MSAA resolve
		assert(false);
	}
}

}