#include "core/core.h"
#include "render/pipeline.h"
#include "render/shader.h"
#include "vulkan/shadermodule.h"
#include "render/bindingtable.h"
#include "vulkan/bindingtable.h"
#include "render/renderpass.h"
#include "render/ropblender.h"
#include "vulkan/renderpass.h"
#include "render/viewport.h"
#include "vulkan/device.h"
#include "vulkan/types.h"
#include "resourcemanager/resourceman.h"
#include "vulkan/pipeline.h"

namespace Vulkan {
auto RenderPipeline::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	using namespace Core::bitmask;

	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface resolver_, uint16_t, uint16_t,
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto renderPipeline = std::static_pointer_cast<Render::RenderPipeline>(ptr_);
		auto vulkanRenderPipeline = renderPipeline->getStage<Vulkan::RenderPipeline, false>(stage_);
		auto[getRMFunc, resolverFunc, resourceNameFunc] = resolver_;

		auto device = device_.lock();
		if(!device) return false;

		std::vector<VkPipelineShaderStageCreateInfo> stages;
		stages.reserve(16);

		std::array<Render::SPIRVShaderHandle, 5> shaderHandles{
				renderPipeline->vertexShader,
				renderPipeline->tesselationControlShader,
				renderPipeline->tesselationEvalShader,
				renderPipeline->geometryShader,
				renderPipeline->fragmentShader,
		};

		// convert shader type to vulkan shader bit
		static constexpr VkShaderStageFlagBits TypeConverter[5]{
				VK_SHADER_STAGE_VERTEX_BIT,
				VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
				VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
				VK_SHADER_STAGE_GEOMETRY_BIT,
				VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		for(auto& handle : shaderHandles)
		{
			auto shader = handle.acquire<Render::SPIRVShader>();
			auto vulkanShader = shader->getStage<ShaderModule>(ShaderModule::s_stage);
			VkPipelineShaderStageCreateInfo stage;
			stage.module = vulkanShader->shaderModule;
			stage.stage = TypeConverter[(uint8_t) shader->shaderType];
			stage.pName = "main"; // TODO
			stage.pSpecializationInfo = nullptr; // TODO
			stages.push_back(stage);
		}

		VkPipelineInputAssemblyStateCreateInfo inputCreateInfo{
				VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
		inputCreateInfo.topology = from(renderPipeline->inputTopology);
		inputCreateInfo.primitiveRestartEnable =
				renderPipeline->flags & Render::RenderPipeline::EnablePrimitiveRestartFlag;

		VkPipelineLayoutCreateInfo layoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		layoutCreateInfo.setLayoutCount = renderPipeline->numBindingTableMemoryMaps;
		std::vector<VkDescriptorSetLayout> layouts(renderPipeline->numBindingTableMemoryMaps);
		for(auto i = 0u; i < renderPipeline->numBindingTableMemoryMaps; ++i)
		{
			auto memmap = renderPipeline->getBindingTableMemoryMaps()[i].acquire<Render::BindingTableMemoryMap>();
			auto bindingLayout = memmap->getStage<Vulkan::BindingTableMemoryMap>(BindingTableMemoryMap::s_stage);
			layouts[i] = bindingLayout->layout;
		}
		layoutCreateInfo.pSetLayouts = layouts.data();
		layoutCreateInfo.pushConstantRangeCount = 0; // TODO push constant API
		vulkanRenderPipeline->layout = device->createPipelineLayout(layoutCreateInfo);

		// TODO pipeline inheritance
		VkGraphicsPipelineCreateInfo createInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
		createInfo.stageCount = (uint32_t) stages.size();
		createInfo.pStages = stages.data();
		createInfo.pInputAssemblyState = &inputCreateInfo;
		createInfo.layout = vulkanRenderPipeline->layout;

		auto renderPass = renderPipeline->renderPass.acquire<Render::RenderPass>();
		auto vulkanRenderPass = renderPass->getStage<RenderPass>(RenderPass::s_stage);
		createInfo.renderPass = vulkanRenderPass->renderpass;

		auto ropBlender = renderPipeline->ropBlender.acquire<Render::ROPBlender>();
		VkPipelineColorBlendStateCreateInfo colourBlendCreateInfo{
				VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
		colourBlendCreateInfo.logicOpEnable = ropBlender->isLogicBlender();
		colourBlendCreateInfo.logicOp = from(ropBlender->logicOp);
		colourBlendCreateInfo.attachmentCount = ropBlender->numTargets;
		std::memcpy(colourBlendCreateInfo.blendConstants, ropBlender->constants, sizeof(float) * 4);
		std::vector<VkPipelineColorBlendAttachmentState> vulkanBlenders(ropBlender->numTargets);

		for(auto j = 0u; j < ropBlender->numTargets; ++j)
		{
			auto const& blender = ropBlender->targetBlenders()[j];
			auto& vkblender = vulkanBlenders[j];
			vkblender.blendEnable = blender.isEnabled();
			vkblender.alphaBlendOp = from(blender.alphaBlendOp);
			vkblender.colorBlendOp = from(blender.colourBlendOp);
			vkblender.srcColorBlendFactor = from(blender.srcColourFactor);
			vkblender.srcAlphaBlendFactor = from(blender.srcAlphaFactor);
			vkblender.dstColorBlendFactor = from(blender.dstColourFactor);
			vkblender.dstAlphaBlendFactor = from(blender.dstAlphaFactor);
			vkblender.colorWriteMask = from(blender.writeMask);
		}
		createInfo.pColorBlendState = &colourBlendCreateInfo;

		auto viewport = renderPipeline->viewport.acquire<Render::Viewport>();
		VkPipelineViewportStateCreateInfo viewportCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
		viewportCreateInfo.scissorCount = viewport->numViewports;
		viewportCreateInfo.viewportCount = viewport->numViewports;
		std::vector<VkViewport> vkviewports(viewport->numViewports);
		std::vector<VkRect2D> vkscissors(viewport->numViewports);

		for(auto k = 0u; k < viewport->numViewports; ++k)
		{
			auto const& viewportAndScissor = viewport->getViewports()[k];
			auto& vkviewport = vkviewports[k];
			auto& vkscissor = vkscissors[k];
			vkviewport.x = viewportAndScissor.x;
			vkviewport.y = viewportAndScissor.y;
			vkviewport.width = viewportAndScissor.width;
			vkviewport.height = viewportAndScissor.height;
			vkviewport.minDepth = viewportAndScissor.minDepth;
			vkviewport.maxDepth = viewportAndScissor.maxDepth;
			vkscissor.offset.x = viewportAndScissor.scissorOffset[0];
			vkscissor.offset.y = viewportAndScissor.scissorOffset[1];
			vkscissor.extent.width = viewportAndScissor.scissorExtent[0];
			vkscissor.extent.height = viewportAndScissor.scissorExtent[1];
		}
		viewportCreateInfo.pViewports = vkviewports.data();
		viewportCreateInfo.pScissors = vkscissors.data();
		createInfo.pViewportState = &viewportCreateInfo;

		vulkanRenderPipeline->pipeline = device->createGraphicsPipeline(createInfo);
		vulkanRenderPipeline->vtable = &device->pipelineVkVTable;

		return true;
	};

	auto deleteFunc = [device_](int stage_, void* ptr_) -> bool
	{
		Render::RenderPipeline* renderPipeline = (Render::RenderPipeline*) ptr_;
		Vulkan::RenderPipeline* vulkanRenderPipeline = renderPipeline->getStage<Vulkan::RenderPipeline>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		device->destroyPipeline(vulkanRenderPipeline->pipeline);

		return true;
	};

	s_stage = rm_.registerNextHandler(Render::RenderPipeline::Id,
									  {sizeof(Vulkan::RenderPipeline), registerFunc, deleteFunc});
}

}
