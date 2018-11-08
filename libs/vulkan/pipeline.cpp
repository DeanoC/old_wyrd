#include "core/core.h"
#include "render/pipeline.h"
#include "render/shader.h"
#include "vulkan/pipeline.h"
#include "vulkan/device.h"
#include "resourcemanager/resourceman.h"
#include "shadermodule.h"
#include "types.h"

namespace Vulkan {
auto RenderPipeline::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	using namespace Core::bitmask;

	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto renderPipeline = std::static_pointer_cast<Render::RenderPipeline>(ptr_);
		auto vulkanRenderPipeline = renderPipeline->getStage<Vulkan::RenderPipeline>(stage_);

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
		inputCreateInfo.topology = fromTopology(renderPipeline->inputTopology);
		inputCreateInfo.primitiveRestartEnable =
				renderPipeline->flags & Render::RenderPipeline::EnablePrimitiveRestartFlag;

		VkPipelineLayoutCreateInfo layoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		//		layoutCreateInfo.setLayoutCount = ?;


		VkGraphicsPipelineCreateInfo createInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
		createInfo.stageCount = stages.size();
		createInfo.pStages = stages.data();
		createInfo.pInputAssemblyState = &inputCreateInfo;
		//		createInfo.layout = ;

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

	s_stage = rm_.registerNextResourceHandler(Render::RenderPipeline::Id,
											  {sizeof(Vulkan::RenderPipeline), registerFunc, deleteFunc});
}

}
