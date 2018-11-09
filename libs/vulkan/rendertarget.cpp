#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "render/resources.h"
#include "render/texture.h"
#include "vulkan/rendertarget.h"
#include "vulkan/device.h"
#include "vulkan/renderpass.h"

namespace Vulkan {

auto RenderTarget::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	using namespace Core::bitmask;

	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto renderTarget = std::static_pointer_cast<Render::RenderTarget>(ptr_);
		Vulkan::RenderTarget* vulkanRenderTarget = renderTarget->getStage<Vulkan::RenderTarget, false>(stage_);

		auto renderPass = renderTarget->renderPassHandle.acquire<Render::RenderPass>();

		std::vector<VkImageView> images(renderPass->numTargets);
		std::vector<Render::TexturePtr> targets(renderPass->numTargets);
		for(auto j = 0u; j < renderPass->numTargets; ++j)
		{
			targets[j] = renderTarget->getTargetTextures()[j].acquire<Render::Texture>();
		}

		uint32_t const width = targets[0]->width;
		uint32_t const height = targets[0]->height;
		for(auto i = 0u; i < renderPass->numTargets; ++i)
		{
			auto& rtarget = targets[i];
			assert(rtarget->width == width);
			assert(rtarget->height == height);

			auto target = rtarget->getStage<Texture>(Texture::s_stage);
			images[i] = target->imageView;
		}
		auto vulkanRenderPass = renderPass->getStage<Vulkan::RenderPass>(stage_);

		VkFramebufferCreateInfo createInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		createInfo.flags = 0;
		createInfo.renderPass = vulkanRenderPass->renderpass;
		createInfo.width = width;
		createInfo.height = height;
		createInfo.attachmentCount = (uint32_t) images.size();
		createInfo.pAttachments = images.data();
		createInfo.layers = 1;

		auto device = device_.lock();
		if(!device) return false;
		vulkanRenderTarget->framebuffer = device->createFramebuffer(createInfo);
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

	s_stage = rm_.registerNextHandler(Render::RenderTarget::Id,
									  {sizeof(Vulkan::RenderTarget), registerFunc, deleteFunc});
}

}