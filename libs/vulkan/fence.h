#pragma once
#ifndef WYRD_VULKAN_FENCE_H
#define WYRD_VULKAN_FENCE_H

#include "core/core.h"
#include "render/fence.h"
#include "vulkan/api.h"

namespace Vulkan {
struct Device;

struct Fence : public Render::Fence
{
	using Ptr = std::shared_ptr<Fence>;
	using WeakPtr = std::shared_ptr<Fence>;

	Fence(std::shared_ptr<Device> device_, VkFence fence_);
	~Fence() final;

	std::weak_ptr<Device> weakDevice;
	VkFence vulkanFence;
};

}
#endif //WYRD_FENCE_H
