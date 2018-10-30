#pragma once
#ifndef WYRD_VULKAN_SEMAPHORE_H
#define WYRD_VULKAN_SEMAPHORE_H

#include "core/core.h"
#include "render/semaphore.h"
#include "vulkan/api.h"

namespace Vulkan {
struct Device;

struct Semaphore : public Render::Semaphore
{
	using Ptr = std::shared_ptr<Semaphore>;
	using WeakPtr = std::shared_ptr<Semaphore>;

	Semaphore(std::shared_ptr<Device> const& device_, VkSemaphore semaphore_);
	~Semaphore() final;

	std::weak_ptr<Device> weakDevice;
	VkSemaphore vulkanSemaphore;
};

}
#endif //WYRD_SEMAPHORE_H
