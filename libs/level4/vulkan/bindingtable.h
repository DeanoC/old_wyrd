#pragma once
#ifndef WYRD_VULKAN_BINDINGTABLE_H
#define WYRD_VULKAN_BINDINGTABLE_H

#include "core/core.h"
#include "vulkan/api.h"
#include "render/bindingtable.h"

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
struct BindingTable : public Render::IGpuBindingTable
{
	using Ptr = std::shared_ptr<BindingTable>;
	using ConstPtr = std::shared_ptr<BindingTable const>;
	using WeakPtr = std::weak_ptr<BindingTable>;
	using ConstWeakPtr = std::weak_ptr<BindingTable const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

	auto update(uint8_t memoryMapIndex_,
						uint32_t bindingIndex_,
						std::vector<Render::TextureHandle> const& textures_) -> void final;


#define DESCRIPTORSET_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(device, args...); }
#define DESCRIPTORSET_VK_FUNC_EXT(name, extension) DESCRIPTORSET_VK_FUNC(name)

#include "functionlist.inl"

	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<Render::BindingTableMemoryMap::ConstPtr> memoryMaps;

	VkDevice device;

	DescriptorSetVkVTable* vtable;

};

} // end namespace Vulkan


#endif //WYRD_VULKAN_BINDINGTABLE_H
