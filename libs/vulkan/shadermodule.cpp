#include "core/core.h"
#include "render/shader.h"
#include "vulkan/shadermodule.h"
#include "vulkan/device.h"
#include "resourcemanager/resourceman.h"

namespace Vulkan {
auto ShaderModule::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	using namespace Core::bitmask;

	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto shader = std::static_pointer_cast<Render::SPIRVShader>(ptr_);
		auto vulkanShaderModule = shader->getStage<Vulkan::ShaderModule, false>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		VkShaderModuleCreateInfo moduleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
		moduleCreateInfo.pCode = shader->getByteCode();
		moduleCreateInfo.codeSize = shader->codeCount * sizeof(uint32_t);

		vulkanShaderModule->shaderModule = device->createShaderModule(moduleCreateInfo);
		vulkanShaderModule->vtable = &device->shaderModuleVkVTable;

		return true;
	};

	auto deleteFunc = [device_](int stage_, void* ptr_) -> bool
	{
		Render::SPIRVShader* shader = (Render::SPIRVShader*) ptr_;
		ShaderModule* vulkanShader = shader->getStage<Vulkan::ShaderModule>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		device->destroyShaderModule(vulkanShader->shaderModule);

		return true;
	};

	s_stage = rm_.registerNextHandler(
			Render::SPIRVShaderId, {sizeof(Vulkan::ShaderModule), registerFunc, deleteFunc});
}

}
