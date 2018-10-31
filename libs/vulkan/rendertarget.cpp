#include "core/core.h"
#include "vulkan/rendertarget.h"
#include "vulkan/device.h"

namespace Vulkan {

RenderTarget::RenderTarget(std::shared_ptr<Device> const& device_) :
		weakDevice(device_)
{

}

RenderTarget::~RenderTarget()
{
	auto device = weakDevice.lock();
	if(device)
	{
		device->destroyFramebuffer(framebuffer);
	}
}

}