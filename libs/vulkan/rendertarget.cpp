#include "core/core.h"
#include "vulkan/rendertarget.h"
#include "vulkan/device.h"
#include "resourcemanager/resourceman.h"

namespace Vulkan {

auto RenderTarget::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	using namespace Core::bitmask;

	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto renderTarget = std::static_pointer_cast<Render::RenderTarget>(ptr_);
		Vulkan::RenderTarget* vulkanRenderTarget = renderTarget->getStage<Vulkan::RenderTarget>(stage_);

		auto device = device_.lock();
		if(!device) return false;
		/*
				std::vector<VkImageView> images(targets_.size());

				uint32_t const width = targets_[0]->width;
				uint32_t const height = targets_[0]->height;
				for(auto i = 0u; i < targets_.size(); ++i)
				{
					auto& rtarget = targets_[i];
					assert(rtarget->width == width);
					assert(rtarget->height == height);

					auto target = rtarget->getStage<Texture>(Texture::s_stage);
					images[i] = target->imageView;
					if(Render::GtfCracker::isDepth(rtarget->format) ||
					   Render::GtfCracker::isStencil(rtarget->format))
					{
						target->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					} else
					{
						target->imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
				}
				auto renderPass = std::static_pointer_cast<Vulkan::RenderPass>(pass_);

				VkFramebufferCreateInfo createInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
				createInfo.flags = 0;
				createInfo.renderPass = renderPass->renderpass;
				createInfo.width = width;
				createInfo.height = height;
				createInfo.attachmentCount = images.size();
				createInfo.pAttachments = images.data();
				createInfo.layers = 0;
				vulkanRenderTarget->framebuffer = device->createFramebuffer(createInfo);
		*/
		vulkanRenderTarget->vtable = &device->framebufferVkVTable;

		return true;
	};

	auto deleteFunc = [device_](int stage_, void* ptr_) -> bool
	{
		Render::RenderTarget* renderTarget = (Render::RenderTarget*) ptr_;
		Vulkan::RenderTarget* vulkanRenderTarget = renderTarget->getStage<Vulkan::RenderTarget>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		device->destroyFramebuffer(vulkanRenderTarget->framebuffer);

		return true;
	};

	s_stage = rm_.registerNextResourceHandler(Render::RenderTarget::Id,
											  {sizeof(Vulkan::RenderTarget), registerFunc, deleteFunc});
}

}