#pragma once
#ifndef WYRD_VULKAN_DEVICE_H
#define WYRD_VULKAN_DEVICE_H

#include "core/core.h"
#include "render/device.h"
#include "vulkan/display.h"
#include "vulkan/api.h"
#include "vulkan/vk_mem_alloc.h"

namespace Vulkan {

class Device : public Render::Device
{
public:
	friend class System;
	using Ptr = std::shared_ptr<Device>;
	using WeakPtr = std::weak_ptr<Device>;
	using Image = std::pair<VkImage, VmaAllocation>;

	Device(VkPhysicalDevice physicalDevice_, VkDevice device_, uint32_t renderQ_, uint32_t computeQ_, uint32_t transferQ_);
	~Device();
	auto getDisplay() const -> std::shared_ptr<Render::Display> final { return std::static_pointer_cast<Render::Display>(display); };

	auto createImage(VkImageCreateInfo const& createInfo_, VmaAllocationCreateInfo const& allocInfo_, VmaAllocationInfo& outInfo_) -> Image;
	auto destroyImage(Image const& image_) -> void;

private:
#define DEVICE_VK_FUNC( name ) PFN_##name _##name; \
	template<typename... Args> auto name(Args... args) { return _##name(device, args...); }
#define DEVICE_VK_FUNC_EXT( name, extension ) PFN_##name _##name; \
	template<typename... Args> auto name(Args... args) { return _##name(device, args...); }

#include "functionlist.inl"

	std::shared_ptr<Display> display; // can be null for headless

	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue renderQueue;
	VkQueue computeQueue;
	VkQueue transferQueue;
	VmaVulkanFunctions vmaVulkanFunctions;
	VmaAllocator allocator;
	VmaAllocationCreateInfo cpuBufferCreateInfo;
	VmaAllocationCreateInfo gpuBufferCreateInfo;
	VmaAllocationCreateInfo imageCreateInfo;

};

}

#endif //WYRD_DEVICE_H
