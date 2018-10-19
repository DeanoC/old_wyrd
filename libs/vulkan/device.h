#pragma once
#ifndef WYRD_VULKAN_DEVICE_H
#define WYRD_VULKAN_DEVICE_H

#include "core/core.h"
#include "render/device.h"
#include "vulkan/api.h"
#include "vulkan/vk_mem_alloc.h"
#include "vulkan/queue.h"
#include <vector>

namespace Vulkan {
class Display;

inline auto AsSemaphore(Render::Semaphore semaphore_) ->VkSemaphore
{
	return reinterpret_cast<VkSemaphore >(semaphore_);
}
inline auto AsRenderSemaphore(VkSemaphore semaphore_) ->Render::Semaphore
{
	return reinterpret_cast<Render::Semaphore >(semaphore_);
}

class Device : public Render::Device
{
public:
	friend class System;
	friend class Display;
	using Ptr = std::shared_ptr<Device>;
	using WeakPtr = std::weak_ptr<Device>;
	using Image = std::pair<VkImage, VmaAllocation>;
	using QueueFamilies = std::vector<VkQueueFamilyProperties>;

	Device( VkPhysicalDevice physicalDevice_,
			VkDeviceCreateInfo createInfo_,
			QueueFamilies const& queueFamilies_,
			uint32_t presentQ_ = ~0);
	~Device();

	auto getDisplay() const -> std::shared_ptr<Render::Display> final;
	auto createSemaphore() -> Render::Semaphore final;
	auto destroySemaphore(Render::Semaphore semaphore_) -> void final;
	virtual auto getMainRenderQueue() -> Render::Queue::Ptr final;
	virtual auto getMainComputeQueue() ->Render::Queue::Ptr final;
	virtual auto getMainBlitQueue() -> Render::Queue::Ptr final;
	virtual auto getMainPresentQueue() -> Render::Queue::Ptr final;

	auto getPhysicalDevice() const -> VkPhysicalDevice { return physicalDevice; }

	auto createImage(VkImageCreateInfo const& createInfo_, VmaAllocationCreateInfo const& allocInfo_, VmaAllocationInfo& outInfo_) -> Image;
	auto destroyImage(Image const& image_) -> void;
	auto createSwapchain(VkSwapchainCreateInfoKHR const& createInfo_) -> VkSwapchainKHR;
	auto destroySwapchain(VkSwapchainKHR swapchain_) -> void;
	auto createSemaphore(VkSemaphoreCreateInfo const& createInfo_) -> VkSemaphore;
	auto destroySemaphore(VkSemaphore semaphore_) -> void;

	auto getSwapchainImages(VkSwapchainKHR swapchain_, std::vector<VkImage >& images_) -> void;
	auto acquireNextImageInSwapchain(VkSwapchainKHR swapchain_) -> uint32_t;

	auto queuePresent(VkPresentInfoKHR const& presentInfo_) -> void;

private:
#define DEVICE_VK_FUNC( name ) \
	PFN_##name _##name; \
	template<typename... Args> auto name(Args... args) { return _##name(device, args...); }
#define DEVICE_VK_FUNC_EXT( name, extension ) \
	PFN_##name _##name; \
	template<typename... Args> auto name(Args... args) { return _##name(device, args...); }

#include "functionlist.inl"

	std::shared_ptr<Display> display; // can be null for headless

	VkPhysicalDevice physicalDevice;
	VkDevice device;
	std::vector<Queue::Ptr> queues;
	Queue::WeakPtr mainPresentQueue;
	Queue::WeakPtr mainRenderQueue;
	Queue::WeakPtr mainComputeQueue;
	Queue::WeakPtr mainBlitQueue;

	VmaVulkanFunctions vmaVulkanFunctions;
	VmaAllocator allocator;
	VmaAllocationCreateInfo cpuBufferCreateInfo;
	VmaAllocationCreateInfo gpuBufferCreateInfo;
	VmaAllocationCreateInfo imageCreateInfo;

};

}

#endif //WYRD_DEVICE_H
