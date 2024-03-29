#include "core/core.h"

#include "render/bindingtable.h"
#include "render/buffer.h"
#include "render/commandqueue.h"
#include "render/pipeline.h"
#include "render/viewport.h"

#include "vulkan/bindingtable.h"
#include "vulkan/buffer.h"
#include "vulkan/texture.h"
#include "vulkan/semaphore.h"
#include "vulkan/renderencoder.h"
#include "vulkan/renderpass.h"
#include "vulkan/rendertarget.h"
#include "vulkan/pipeline.h"
#include "types.h"

namespace Vulkan {

auto RenderEncoder::clearTexture(Render::TextureConstPtr const& texture_,
								 std::array<float_t, 4> const& floats_) -> void
{
	Texture* texture = texture_->getStage<Texture>(Texture::s_stage);

	if(texture->entireRange.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
	{
		VkClearColorValue colour;
		std::memcpy(&colour.float32, floats_.data(), floats_.size() * sizeof(float));
		vkCmdClearColorImage(texture->image, texture->imageLayout, &colour, 1, &texture->entireRange);
	} else
	{
		VkClearDepthStencilValue ds;
		ds.depth = floats_[0];
		ds.stencil = (uint32_t) floats_[1]; // this only works upto 23 integer bits but stencil cap at 8 so..
		vkCmdClearDepthStencilImage(texture->image, texture->imageLayout, &ds, 1, &texture->entireRange);
	}
}

auto RenderEncoder::blit(Render::TextureConstPtr const& src_,
						 Render::TextureConstPtr const& dst_) -> void
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

auto RenderEncoder::beginRenderPass(
		Render::RenderPassConstPtr const& renderPass_,
		Render::RenderTargetConstPtr const& renderTarget_
) -> void
{
	auto vulkanRenderPass = renderPass_->getStage<RenderPass>(Vulkan::RenderPass::s_stage);
	auto vulkanRenderTarget = renderTarget_->getStage<RenderTarget>(Vulkan::RenderTarget::s_stage);

	VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
	beginInfo.renderPass = vulkanRenderPass->renderpass;

	// TODO pick correct clear type based on target format
	std::vector<VkClearValue> clearValues(renderPass_->numTargets);
	for(auto i = 0u; i < renderPass_->numTargets; ++i)
	{
		auto const& target = renderPass_->getTargets()[i];
		auto const& clearVal = renderPass_->getClearValues()[i];
		auto& clearValue = clearValues[i];
		if(!Render::GtfCracker::isDepthStencil(target.format))
		{
			clearValue.color.float32[0] = clearVal.x;
			clearValue.color.float32[1] = clearVal.y;
			clearValue.color.float32[2] = clearVal.z;
			clearValue.color.float32[3] = clearVal.w;
		} else
		{
			clearValue.depthStencil.depth = clearVal.x;
			clearValue.depthStencil.stencil = (uint8_t)(clearVal.y * 255.0f);
		}
	}
	beginInfo.clearValueCount = (uint32_t)clearValues.size();
	beginInfo.pClearValues = clearValues.data();

	beginInfo.framebuffer = vulkanRenderTarget->framebuffer;
	beginInfo.renderArea.offset = {renderTarget_->renderOffset[0], renderTarget_->renderOffset[1]};
	beginInfo.renderArea.extent = {renderTarget_->renderExtent[0], renderTarget_->renderExtent[1]};
	vkCmdBeginRenderPass(&beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	/*
	VkClearAttachment clearAttachment;
	clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	clearAttachment.clearValue = {1.0, 1.0f, 0.0f,1.0f};
	clearAttachment.colorAttachment = 0;

	VkClearRect clearRect;
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;
	clearRect.rect = {0,0,640,160};
	vkCmdClearAttachments(1, &clearAttachment, 1, &clearRect );
	 */
}

auto RenderEncoder::endRenderPass() -> void
{
	vkCmdEndRenderPass();
}

auto RenderEncoder::resolveForDisplay(
		Render::TextureConstPtr const& src_,
		uint32_t width_, uint32_t height_,
		VkImage display_) -> void
{
	auto src = src_->getStage<Texture>(Texture::s_stage);

	std::array<VkImageMemoryBarrier, 2> barriers;

	VkImageMemoryBarrier& sbarrier = barriers[0];
	VkImageMemoryBarrier& pbarrier = barriers[1];
	sbarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	pbarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};

	sbarrier.oldLayout = src->imageLayout;
	sbarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	src->imageLayout = sbarrier.newLayout;
	sbarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	sbarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	sbarrier.image = src->image;
	sbarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	sbarrier.subresourceRange.baseMipLevel = 0;
	sbarrier.subresourceRange.baseArrayLayer = 0;
	sbarrier.subresourceRange.levelCount = 1;
	sbarrier.subresourceRange.layerCount = 1;
	sbarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	sbarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	pbarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	pbarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	pbarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	pbarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	pbarrier.image = display_;
	pbarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	pbarrier.subresourceRange.baseMipLevel = 0;
	pbarrier.subresourceRange.baseArrayLayer = 0;
	pbarrier.subresourceRange.levelCount = 1;
	pbarrier.subresourceRange.layerCount = 1;
	pbarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	pbarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	vkCmdPipelineBarrier(
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			2, barriers.data());

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
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					   1, &blitter, VK_FILTER_LINEAR);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
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
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
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
auto RenderEncoder::bind(Render::RenderPipelineConstPtr const& pipeline_, Render::BindingTableConstPtr const& bindingTable_) -> void
{
	auto pipeline = pipeline_->getStage<RenderPipeline>(RenderPipeline::s_stage);
	auto bindingTable = bindingTable_->getStage<BindingTable>(BindingTable::s_stage);

	bind(pipeline_);

	vkCmdBindDescriptorSets(
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->layout,
			0,
			(uint32_t)bindingTable->descriptorSets.size(),
			bindingTable->descriptorSets.data(),
			0,
			nullptr);

}

auto RenderEncoder::bind(Render::RenderPipelineConstPtr const& pipeline_) -> void
{
	auto pipeline = pipeline_->getStage<RenderPipeline>(RenderPipeline::s_stage);
	vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
}

auto RenderEncoder::bindVertexBuffer(Render::BufferConstPtr const& buffer_, uint64_t offset_,
									 uint32_t bindingIndex) -> void
{
	assert(buffer_->canBeReadByVertex());
	auto buffer = buffer_->getStage<Buffer>(Buffer::s_stage);
	vkCmdBindVertexBuffers(bindingIndex, 1, &buffer->buffer, &offset_);
}

auto RenderEncoder::bindIndexBuffer(Render::BufferConstPtr const& buffer_, uint8_t bitSize_, uint64_t offset_) -> void
{
	assert(buffer_->canBeReadByIndex());
	assert(bitSize_ == 16 || bitSize_ == 32);

	auto buffer = buffer_->getStage<Buffer>(Buffer::s_stage);
	vkCmdBindIndexBuffer(buffer->buffer, offset_, VkIndexType(bitSize_ / 32));
}

auto RenderEncoder::pushConstants(Render::RenderPipelineConstPtr const& pipeline_,
								  Render::PushConstantRange const& range_, void const* data_) -> void
{
	auto pipeline = pipeline_->getStage<RenderPipeline>(RenderPipeline::s_stage);
	vkCmdPushConstants(pipeline->layout, from(range_.shaderAccess), range_.offset, range_.sizeInBytes, data_);
}

auto RenderEncoder::draw(uint32_t vertexCount_, uint32_t vertexOffset_, uint32_t instanceCount_,
						 uint32_t instanceOffset_) -> void
{
	vkCmdDraw(vertexCount_, instanceCount_, vertexOffset_, instanceOffset_);
}

auto RenderEncoder::drawIndexed(uint32_t indexCount_, uint32_t indexOffset_, uint32_t vertexOffset,
								uint32_t instanceCount_, uint32_t instanceOffset_) -> void
{
	vkCmdDrawIndexed(indexCount_, instanceCount_, indexOffset_, vertexOffset, instanceOffset_);
}

auto RenderEncoder::setDynamicViewport(uint32_t viewportIndex_, Render::ViewportDef const& viewport_) -> void
{
	VkViewport vkviewport{};
	vkviewport.x = viewport_.x;
	vkviewport.y = viewport_.y;
	vkviewport.width = viewport_.width;
	vkviewport.height = viewport_.height;
	vkviewport.minDepth = viewport_.minDepth;
	vkviewport.maxDepth = viewport_.maxDepth;
	vkCmdSetViewport(viewportIndex_, 1, &vkviewport);
}
auto RenderEncoder::setDynamicScissor(uint32_t viewportIndex_, Render::Scissor const& scissor_) -> void
{
	VkRect2D scissor{};
	scissor.offset.x = scissor_.offset[0];
	scissor.offset.y = scissor_.offset[1];
	scissor.extent.width = scissor_.extent[0];
	scissor.extent.height = scissor_.extent[1];
	vkCmdSetScissor(viewportIndex_, 1, &scissor);
}

}