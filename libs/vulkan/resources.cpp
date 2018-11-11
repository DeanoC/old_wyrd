#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "vulkan/resources.h"
#include "vulkan/buffer.h"
#include "vulkan/device.h"
#include "vulkan/texture.h"
#include "vulkan/rendertarget.h"
#include "vulkan/renderpass.h"
#include "vulkan/pipeline.h"
#include "vulkan/bindingtable.h"
#include "vulkan/shadermodule.h"

namespace Vulkan {
auto RegisterResourceHandlers(ResourceManager::ResourceMan& rm_, Device::Ptr device_) -> void
{
	Buffer::RegisterResourceHandler(rm_, device_);
	Texture::RegisterResourceHandler(rm_, device_);
	RenderPass::RegisterResourceHandler(rm_, device_);
	RenderTarget::RegisterResourceHandler(rm_, device_);
	ShaderModule::RegisterResourceHandler(rm_, device_);
	BindingTable::RegisterResourceHandler(rm_, device_);
	BindingTableMemoryMap::RegisterResourceHandler(rm_, device_);
	RenderPipeline::RegisterResourceHandler(rm_, device_);
	//	ComputePipeline::RegisterResourceHandler(rm_, device_);


}
}
