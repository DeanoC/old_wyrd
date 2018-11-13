#include "core/core.h"
#include "render/pipeline.h"
#include "render/shader.h"
#include "render/bindingtable.h"
#include "render/renderpass.h"
#include "render/ropblender.h"
#include "render/vertexinput.h"
#include "render/viewport.h"
#include "render/rasterisationstate.h"
#include "vulkan/shadermodule.h"
#include "vulkan/renderpass.h"
#include "vulkan/device.h"
#include "vulkan/types.h"
#include "vulkan/bindingtable.h"
#include "vulkan/pipeline.h"
#include "resourcemanager/resourceman.h"
namespace Vulkan {
namespace {

// convert shader type to a single vulkan shader bit
constexpr auto fromSingle(Render::ShaderType const in_) -> VkShaderStageFlagBits
{
	using namespace Render;
	switch(in_)
	{
		case ShaderType::Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderType::TesselationControl:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case ShaderType::TesselationEval:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case ShaderType::Geometry:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case ShaderType::Fragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderType::Compute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		default:
			assert(false);
			return VK_SHADER_STAGE_COMPUTE_BIT;
	}
};

constexpr auto fromSingle(Render::SampleCounts const in_) -> VkSampleCountFlagBits
{
	using namespace Render;
	switch(in_)
	{
		case SampleCounts::One:
			return VK_SAMPLE_COUNT_1_BIT;
		case SampleCounts::Two:
			return VK_SAMPLE_COUNT_2_BIT;
		case SampleCounts::Four:
			return VK_SAMPLE_COUNT_4_BIT;
		case SampleCounts::Eight:
			return VK_SAMPLE_COUNT_8_BIT;
		case SampleCounts::Sixteen:
			return VK_SAMPLE_COUNT_16_BIT;
		case SampleCounts::ThirtyTwo:
			return VK_SAMPLE_COUNT_32_BIT;
		case SampleCounts::SixtyFour:
			return VK_SAMPLE_COUNT_64_BIT;
		default:
			assert(false);
			return VK_SAMPLE_COUNT_1_BIT;
	}
};

constexpr auto from(Render::VertexInputType const in_) -> VkFormat
{
	using namespace Render;
	switch(in_)
	{
		case VertexInputType::Float1:
			return VK_FORMAT_R32_SFLOAT;
		case VertexInputType::Float2:
			return VK_FORMAT_R32G32_SFLOAT;
		case VertexInputType::Float3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case VertexInputType::Float4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case VertexInputType::Byte4 :
			return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
		default:
			assert(false);
			return VK_FORMAT_R32_SFLOAT;

	}
};

}

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

		// TODO pipeline inheritance
		VkGraphicsPipelineCreateInfo createInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
		VkPipelineLayoutCreateInfo layoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

		layoutCreateInfo.setLayoutCount = renderPipeline->numBindingTableMemoryMaps;
		std::vector<VkDescriptorSetLayout> layouts(renderPipeline->numBindingTableMemoryMaps);
		for(auto i = 0u; i < renderPipeline->numBindingTableMemoryMaps; ++i)
		{
			auto memmap = renderPipeline->getBindingTableMemoryMapHandles()[i].acquire<Render::BindingTableMemoryMap>();
			auto bindingLayout = memmap->getStage<Vulkan::BindingTableMemoryMap>(BindingTableMemoryMap::s_stage);
			layouts[i] = bindingLayout->layout;
		}
		layoutCreateInfo.pSetLayouts = layouts.data();
		layoutCreateInfo.pushConstantRangeCount = renderPipeline->numPushConstantRanges;
		std::vector<VkPushConstantRange> pushConstants(renderPipeline->numPushConstantRanges);
		for(auto i = 0u; i < renderPipeline->numPushConstantRanges; ++i)
		{
			auto const& pushC = renderPipeline->getPushConstantRanges()[i];
			pushConstants[i].stageFlags = from(pushC.shaderAccess);
			pushConstants[i].offset = pushC.offset;
			pushConstants[i].size = pushC.sizeInBytes;
		}
		layoutCreateInfo.pPushConstantRanges = pushConstants.data();

		std::vector<VkPipelineShaderStageCreateInfo> stages;
		stages.reserve(renderPipeline->numShaders);
		for(auto m = 0u; m < renderPipeline->numShaders; ++m)
		{
			auto const& handle = renderPipeline->getSPIRVShaderHandles()[m];
			if(handle.isValid())
			{
				auto shader = handle.acquire<Render::SPIRVShader>();
				auto vulkanShader = shader->getStage<ShaderModule>(ShaderModule::s_stage);
				VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
				stage.module = vulkanShader->shaderModule;
				stage.stage = fromSingle(shader->shaderType);
				stage.pName = "main"; // TODO
				stage.pSpecializationInfo = nullptr; // TODO 
				stages.push_back(stage);
			}
		}
		createInfo.stageCount = (uint32_t) stages.size();
		createInfo.pStages = stages.data();

		VkPipelineInputAssemblyStateCreateInfo inputCreateInfo{
				VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
		inputCreateInfo.topology = from(renderPipeline->inputTopology);
		inputCreateInfo.primitiveRestartEnable = renderPipeline->isPrimitiveRestartEnabled();
		createInfo.pInputAssemblyState = &inputCreateInfo;

		auto vertexInput = renderPipeline->vertexInput.acquire<Render::VertexInput>();

		VkPipelineVertexInputStateCreateInfo vertexCreateInfo = {
				VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
		vertexCreateInfo.vertexAttributeDescriptionCount = vertexInput->numVertexInputs;
		vertexCreateInfo.vertexBindingDescriptionCount = vertexInput->numVertexInputs;

		std::vector<VkVertexInputBindingDescription> vinputBinding(vertexInput->numVertexInputs);
		std::vector<VkVertexInputAttributeDescription> vinputAttr(vertexInput->numVertexInputs);
		for(auto l = 0u; l < vertexInput->numVertexInputs; ++l)
		{
			auto const& input = vertexInput->getInputs()[l];
			auto& inputBinding = vinputBinding[l];
			auto& inputAttr = vinputAttr[l];
			inputAttr.format = from(input.type);
			inputAttr.location = input.shaderIndex;
			inputAttr.binding = input.binding;
			inputAttr.offset = vertexInput->getElementOffset(l);
			inputBinding.binding = input.binding;
			inputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			inputBinding.stride = vertexInput->getStride();
		}
		vertexCreateInfo.pVertexAttributeDescriptions = vinputAttr.data();
		vertexCreateInfo.pVertexBindingDescriptions = vinputBinding.data();
		createInfo.pVertexInputState = &vertexCreateInfo;

		auto rasterState = renderPipeline->rasterisationState.acquire<Render::RasterisationState>();

		VkPipelineRasterizationStateCreateInfo rasterStateCreateInfo{
				VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
		VkPipelineTessellationStateCreateInfo tessStateCreateInfo{
				VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
				VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
				VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

		rasterStateCreateInfo.rasterizerDiscardEnable = rasterState->isDiscardEnabled();

		depthStencilStateCreateInfo.depthTestEnable = rasterState->isDepthTestEnabled();
		depthStencilStateCreateInfo.depthWriteEnable = rasterState->isDepthWriteEnabled();
		depthStencilStateCreateInfo.depthBoundsTestEnable = rasterState->isDepthBoundsEnabled();
		depthStencilStateCreateInfo.stencilTestEnable = rasterState->isStencilTestEnabled();
		rasterStateCreateInfo.depthClampEnable = rasterState->isDepthClampEnabled();

		depthStencilStateCreateInfo.depthCompareOp = from(rasterState->depthCompare);
		depthStencilStateCreateInfo.front.compareOp = from(rasterState->frontStencil.compareOp);
		depthStencilStateCreateInfo.front.depthFailOp = from(rasterState->frontStencil.depthFailOp);
		depthStencilStateCreateInfo.front.failOp = from(rasterState->frontStencil.failOp);
		depthStencilStateCreateInfo.front.passOp = from(rasterState->frontStencil.passOp);
		depthStencilStateCreateInfo.front.writeMask = rasterState->frontStencil.writeMask;
		depthStencilStateCreateInfo.front.compareMask = rasterState->frontStencil.compareMask;
		depthStencilStateCreateInfo.front.reference = rasterState->frontStencil.reference;
		depthStencilStateCreateInfo.back.compareOp = from(rasterState->backStencil.compareOp);
		depthStencilStateCreateInfo.back.depthFailOp = from(rasterState->backStencil.depthFailOp);
		depthStencilStateCreateInfo.back.failOp = from(rasterState->backStencil.failOp);
		depthStencilStateCreateInfo.back.passOp = from(rasterState->backStencil.passOp);
		depthStencilStateCreateInfo.back.writeMask = rasterState->backStencil.writeMask;
		depthStencilStateCreateInfo.back.compareMask = rasterState->backStencil.compareMask;
		depthStencilStateCreateInfo.back.reference = rasterState->backStencil.reference;
		depthStencilStateCreateInfo.minDepthBounds = rasterState->minDepthBounds;
		depthStencilStateCreateInfo.maxDepthBounds = rasterState->maxDepthBounds;

		rasterStateCreateInfo.cullMode = from(rasterState->cullMode);
		rasterStateCreateInfo.frontFace = from(rasterState->frontFace);
		rasterStateCreateInfo.polygonMode = from(rasterState->fillMode);
		rasterStateCreateInfo.lineWidth = rasterState->lineWidth;
		rasterStateCreateInfo.depthBiasEnable = rasterState->isDepthBiasEnabled();
		rasterStateCreateInfo.depthBiasClamp = rasterState->depthBias.clamp;
		rasterStateCreateInfo.depthBiasConstantFactor = rasterState->depthBias.constantFactor;
		rasterStateCreateInfo.depthBiasSlopeFactor = rasterState->depthBias.slopeFactor;
		tessStateCreateInfo.patchControlPoints = rasterState->patchControlPointsCount;
		multisampleStateCreateInfo.rasterizationSamples = fromSingle(rasterState->sampleCount);
		multisampleStateCreateInfo.alphaToCoverageEnable = rasterState->isAlphaToCoverageEnabled();
		multisampleStateCreateInfo.alphaToOneEnable = rasterState->isAlphaToOneEnabled();
		multisampleStateCreateInfo.minSampleShading = rasterState->minSampleShadingRate;
		multisampleStateCreateInfo.pSampleMask = (VkSampleMask*) &rasterState->sampleMask;

		createInfo.pRasterizationState = &rasterStateCreateInfo;
		createInfo.pTessellationState = &tessStateCreateInfo;
		createInfo.pDepthStencilState = &depthStencilStateCreateInfo;
		createInfo.pMultisampleState = &multisampleStateCreateInfo;

		auto ropBlender = renderPipeline->ropBlender.acquire<Render::ROPBlender>();
		VkPipelineColorBlendStateCreateInfo colourBlendCreateInfo{
				VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
		colourBlendCreateInfo.logicOpEnable = ropBlender->isLogicBlender();
		colourBlendCreateInfo.logicOp = from(ropBlender->logicOp);
		std::memcpy(colourBlendCreateInfo.blendConstants, ropBlender->constants, sizeof(float) * 4);

		colourBlendCreateInfo.attachmentCount = ropBlender->numTargets;
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
		colourBlendCreateInfo.pAttachments = vulkanBlenders.data();
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

		vulkanRenderPipeline->layout = device->createPipelineLayout(layoutCreateInfo);
		createInfo.layout = vulkanRenderPipeline->layout;

		auto renderPass = renderPipeline->renderPass.acquire<Render::RenderPass>();
		auto vulkanRenderPass = renderPass->getStage<RenderPass>(RenderPass::s_stage);
		createInfo.renderPass = vulkanRenderPass->renderpass;

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
