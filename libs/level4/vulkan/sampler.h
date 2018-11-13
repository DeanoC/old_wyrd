#pragma once
#ifndef WYRD_VULKAN_SAMPLER_H
#define WYRD_VULKAN_SAMPLER_H

#include "core/core.h"
#include "render/sampler.h"
#include "vulkan/api.h"

namespace Vulkan {
struct Device;

struct Sampler
{
	using Ptr = std::shared_ptr<Sampler>;
	using ConstPtr = std::shared_ptr<Sampler const>;
	using WeakPtr = std::weak_ptr<Sampler>;
	using ConstWeakPtr = std::weak_ptr<Sampler const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

	VkSampler sampler;
};

}

#endif //WYRD_VULKAN_SAMPLER_H
