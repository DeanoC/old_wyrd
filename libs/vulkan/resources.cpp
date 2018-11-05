#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "vulkan/resources.h"
#include "vulkan/device.h"
#include "vulkan/texture.h"
#include "vulkan/rendertarget.h"
#include "vulkan/renderpass.h"

namespace Vulkan {
auto RegisterResourceHandlers(ResourceManager::ResourceMan& rm_, Device::Ptr device_) -> void
{
	Texture::RegisterResourceHandler(rm_, device_);
	RenderPass::RegisterResourceHandler(rm_, device_);
	RenderTarget::RegisterResourceHandler(rm_, device_);

}
}
