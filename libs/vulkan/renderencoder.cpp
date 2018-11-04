#include "core/core.h"
#include "render/commandqueue.h"
#include "vulkan/texture.h"
#include "vulkan/semaphore.h"
#include "vulkan/renderencoder.h"

namespace Vulkan {

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
			{{0, 0, 0},                 {(int32_t) src_->width - 1, (int32_t) src_->height - 1, 1}},
			{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
			{{0, 0, 0},                 {(int32_t) dst_->width - 1, (int32_t) dst_->height - 1, 1}},
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
				{{0, 0, 0},                 {(int32_t) src_->width - 1, (int32_t) src_->height - 1, 1}},
				{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
				{{0, 0, 0},                 {(int32_t) width_ - 1,      (int32_t) height_ - 1,      1}},
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