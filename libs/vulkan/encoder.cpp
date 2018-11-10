#include "core/core.h"
#include "render/buffer.h"
#include "render/commandqueue.h"
#include "vulkan/buffer.h"
#include "vulkan/texture.h"
#include "vulkan/semaphore.h"
#include "vulkan/encoderpool.h"
#include "vulkan/encoder.h"
#include "vulkan/types.h"

namespace Vulkan {

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
	VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
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

auto Encoder::copy(Render::TextureConstPtr const& src_, Render::TextureConstPtr const& dst_) -> void
{
	Texture* src = src_->getStage<Texture>(Texture::s_stage);

	VkImageSubresourceLayers srcLayers;
	srcLayers.aspectMask = src->entireRange.aspectMask;
	srcLayers.baseArrayLayer = src->entireRange.baseArrayLayer;
	srcLayers.layerCount = src->entireRange.layerCount;
	srcLayers.mipLevel = src_->mipLevels;

	copy(src->image, src->imageLayout, srcLayers, dst_);

}

auto Encoder::fill(uint32_t fill_, Render::BufferPtr const& dst_) -> void
{
	Buffer* dst = dst_->getStage<Buffer>(Buffer::s_stage);
	vkCmdFillBuffer(dst->buffer, 0, VK_WHOLE_SIZE, fill_);
}

auto Encoder::copy(VkImage srcImage_,
				   VkImageLayout srcLayout_,
				   VkImageSubresourceLayers const& srcExtents_,
				   Render::TextureConstPtr const& dst_) -> void
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

auto Encoder::textureBarrier(
		Render::MemoryAccess waitAccess_,
		Render::MemoryAccess stallAccess_,
		Render::TextureConstPtr const& texture_) -> void
{
	Texture* texture = texture_->getStage<Texture>(Texture::s_stage);

	uint32_t srcMask = from(waitAccess_);
	uint32_t dstMask = from(stallAccess_);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = texture->imageLayout;
	barrier.newLayout = texture->imageLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = texture->image;
	barrier.subresourceRange = texture->entireRange;
	barrier.srcAccessMask = srcMask;
	barrier.dstAccessMask = dstMask;

	vkCmdPipelineBarrier(
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

}

auto Encoder::textureBarrier(Render::TextureConstPtr const& texture_) -> void
{
	using namespace Render;
	textureBarrier(MemoryAccess::GeneralWrite, MemoryAccess::GeneralRead, texture_);
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

}