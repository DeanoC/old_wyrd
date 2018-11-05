#include "core/core.h"
#include "vulkan/renderpass.h"
#include "vulkan/device.h"
#include "vulkan/vkfcracker.h"
#include "resourcemanager/resourceman.h"

namespace Vulkan {
auto RenderPass::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	using namespace Core::bitmask;

	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto renderPass = std::static_pointer_cast<Render::RenderPass>(ptr_);
		auto vulkanRenderPass = renderPass->getStage<Vulkan::RenderPass>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		// convert between RenderPass::Load/Store and vulkans
		static constexpr VkAttachmentLoadOp LoadConvertor[3]{
				VK_ATTACHMENT_LOAD_OP_LOAD,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE
		};
		static constexpr VkAttachmentStoreOp StoreConvertor[3]{
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE
		};

		VkRenderPassCreateInfo createInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};

		std::vector<VkAttachmentDescription> attachments(renderPass->numTargets);
		std::vector<VkAttachmentReference> cReferences;
		cReferences.reserve(renderPass->numTargets);
		VkAttachmentReference dsReference = {~0u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		for(auto i = 0u; i < renderPass->numTargets; ++i)
		{
			auto& attach = attachments[i];
			auto const& target = renderPass->targets[i];

			attach.flags = 0;
			attach.samples = VK_SAMPLE_COUNT_1_BIT; // TODO
			if(target.load == Render::LoadOp::Load)
			{
				// TODO need to know what the layour was we are loading from?
				attach.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
			} else
			{
				// we don't care, but we can't load from undefined
				attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			}
			attach.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			attach.loadOp = LoadConvertor[(int) target.load];
			attach.storeOp = StoreConvertor[(int) target.store];
			if(Render::GtfCracker::isStencil(target.format))
			{
				attach.stencilLoadOp = LoadConvertor[(int) target.stencilLoad];
				attach.stencilStoreOp = StoreConvertor[(int) target.stencilStore];
			}
			attach.format = VkfCracker::fromGeneric(target.format);

			if(Render::GtfCracker::isDepth(target.format) ||
			   Render::GtfCracker::isStencil(target.format))
			{
				dsReference = {i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
			} else
			{
				cReferences.emplace_back(
						VkAttachmentReference{i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
			}
		}

		// TODO subpasses, for now assume 1 and it matches the entire render target
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = cReferences.size();
		subpass.pColorAttachments = cReferences.data();
		subpass.pDepthStencilAttachment =
				(dsReference.attachment != ~0) ? &dsReference : nullptr;

		createInfo.attachmentCount = attachments.size();
		createInfo.pAttachments = attachments.data();
		createInfo.flags = 0;
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;
		createInfo.dependencyCount = 0;
		createInfo.pDependencies = nullptr;

		vulkanRenderPass->renderpass = device->createRenderPass(createInfo);
		vulkanRenderPass->vtable = &device->renderPassVkVTable;

		return true;
	};

	auto deleteFunc = [device_](int stage_, void* ptr_) -> bool
	{
		Render::RenderPass* renderPass = (Render::RenderPass*) ptr_;
		Vulkan::RenderPass* vulkanRenderPass = renderPass->getStage<Vulkan::RenderPass>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		device->destroyRenderPass(vulkanRenderPass->renderpass);

		return true;
	};

	s_stage = rm_.registerNextResourceHandler(Render::RenderPass::Id,
											  {sizeof(Vulkan::RenderPass), registerFunc, deleteFunc});
}

}
