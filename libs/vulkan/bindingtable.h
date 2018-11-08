#pragma once
#ifndef WYRD_VULKAN_BINDINGTABLE_H
#define WYRD_VULKAN_BINDINGTABLE_H

#include "core/core.h"
#include "vulkan/api.h"

namespace ResourceManager { class ResourceMan; }

namespace Vulkan {
struct Device;

// currently we have a policy of 1 pool per BindingTable
// probably not optimal TODO get fancy
struct BindingTable
{
	using Ptr = std::shared_ptr<BindingTable>;
	using ConstPtr = std::shared_ptr<BindingTable const>;
	using WeakPtr = std::weak_ptr<BindingTable>;
	using ConstWeakPtr = std::weak_ptr<BindingTable const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

	VkDescriptorPool pool;
};

}


#endif //WYRD_VULKAN_BINDINGTABLE_H
