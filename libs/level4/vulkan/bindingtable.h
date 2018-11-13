#pragma once
#ifndef WYRD_VULKAN_BINDINGTABLE_H
#define WYRD_VULKAN_BINDINGTABLE_H

#include "core/core.h"
#include "vulkan/api.h"

namespace ResourceManager { class ResourceMan; }

namespace Vulkan {
struct Device;

struct BindingTableMemoryMap
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

	VkDescriptorSetLayout layout;
};

// TODO more pools; currently we have a single pool that the device manages
struct BindingTable
{
	using Ptr = std::shared_ptr<BindingTable>;
	using ConstPtr = std::shared_ptr<BindingTable const>;
	using WeakPtr = std::weak_ptr<BindingTable>;
	using ConstWeakPtr = std::weak_ptr<BindingTable const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

#define DESCRIPTORSET_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(renderpass, args...); }
#define DESCRIPTORSET_VK_FUNC_EXT(name) DESCRIPTORSET_VK_FUNC

#include "functionlist.inl"

	VkPipeline pipeline;
	DescriptorSetVkVTable* vtable;

	VkDescriptorSet descriptorSet;
};

} // end namespace Vulkan


#endif //WYRD_VULKAN_BINDINGTABLE_H
