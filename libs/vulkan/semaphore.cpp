

#include "core/core.h"
#include "vulkan/semaphore.h"
#include "vulkan/device.h"

namespace Vulkan {
Semaphore::Semaphore(std::shared_ptr<Device> const& device_, VkSemaphore semaphore_) :
		weakDevice(device_),
		vulkanSemaphore(semaphore_)
{
}

Semaphore::~Semaphore()
{
	auto device = weakDevice.lock();
	if(device)
	{
		device->destroySemaphore(vulkanSemaphore);
	}
}

}