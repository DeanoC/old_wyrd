

#include "core/core.h"
#include "vulkan/fence.h"
#include "vulkan/device.h"

namespace Vulkan {

Fence::Fence(std::shared_ptr<Device> device_, VkFence fence_) :
		weakDevice(device_),
		vulkanFence(fence_)
{

}

Fence::~Fence()
{
	auto device = weakDevice.lock();
	if(device)
	{
		device->destroyFence(vulkanFence);
	}
}

}