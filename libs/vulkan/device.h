#pragma once
#ifndef WYRD_VULKAN_DEVICE_H
#define WYRD_VULKAN_DEVICE_H

#include "core/core.h"
#include "render/device.h"
#include "render/commandqueue.h"

#include "vulkan/api.h"
#include "vulkan/vk_mem_alloc.h"
#include <array>

namespace Vulkan {
class Display;

class CommandQueue;

inline auto AsSemaphore(Render::Semaphore semaphore_) ->VkSemaphore
{
	return reinterpret_cast<VkSemaphore >(semaphore_);
}
inline auto AsRenderSemaphore(VkSemaphore semaphore_) ->Render::Semaphore
{
	return reinterpret_cast<Render::Semaphore >(semaphore_);
}

class Device : public Render::Device, std::enable_shared_from_this<Device>
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

	// Render::Device interface
	auto getDisplay() const -> std::shared_ptr<Render::Display> final;

	auto createSemaphore() -> Render::Semaphore final;
	auto destroySemaphore(Render::Semaphore semaphore_) -> void final;

	auto createEncoderPool(bool frameLifetime_, uint32_t queueType_) -> std::shared_ptr<Render::EncoderPool> final;
	auto destroyEncoderPool() -> void final;

	auto getMainRenderQueue() -> Render::CommandQueue::Ptr final;

	auto getMainComputeQueue() -> Render::CommandQueue::Ptr final;
	auto getMainBlitQueue() -> Render::CommandQueue::Ptr final;
	auto getMainPresentQueue() -> Render::CommandQueue::Ptr final;

	// Vulkan specific functions
	auto getPhysicalDevice() const -> VkPhysicalDevice { return physicalDevice; }

	auto createImage(VkImageCreateInfo const& createInfo_, VmaAllocationCreateInfo const& allocInfo_, VmaAllocationInfo& outInfo_) -> Image;
	auto destroyImage(Image const& image_) -> void;

	auto createSwapchain(VkSwapchainCreateInfoKHR const& createInfo_) -> VkSwapchainKHR;
	auto destroySwapchain(VkSwapchainKHR swapchain_) -> void;

	auto createSemaphore(VkSemaphoreCreateInfo const& createInfo_) -> VkSemaphore;
	auto destroySemaphore(VkSemaphore semaphore_) -> void;

	auto createCommandBuffer(VkCommandBufferAllocateInfo const& createInfo_) -> VkCommandBuffer;
	auto destroyCommandBuffer(VkCommandBuffer commandBuffer_) -> void;

	auto getSwapchainImages(VkSwapchainKHR swapchain_, std::vector<VkImage >& images_) -> void;
	auto acquireNextImageInSwapchain(VkSwapchainKHR swapchain_) -> uint32_t;

private:

	DeviceVkVTable deviceVkVTable;

	//--
	// functions also in device table
	FenceVkVTable fenceVkVTable;
	SemaphoreVkVTable semaphoreVkVTable;
	EventVkVTable eventVkVTable;
	QueryPoolVkVTable queryPoolVkVTable;
	BufferVkVTable bufferVkVTable;
	BufferViewVkVTable bufferViewVkVTable;
	ImageVkVTable imageVkVTable;
	ImageViewVkVTable imageViewVkVTable;
	ShaderModuleVkVTable shaderModuleVkVTable;
	PipelineVkVTable pipelineVkVTable;
	SamplerVkVTable samplerVkVTable;
	DescriptorSetVkVTable descriptorSetVkVTable;
	FramebufferVkVTable framebufferVkVTable;
	RenderPassVkVTable renderPassVkVTable;
	CommandPoolVkVTable commandPoolVkVTable;

	//--
	QueueVkVTable queueVkVTable;

	GraphicsCBVkVTable graphicsCBVkVTable;
	ComputeCBVkVTable computeCBVkVTable;
	TransferCBVkVTable transferCBVkVTable;

#define DEVICE_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return deviceVkVTable. name(device, args...); }
#define DEVICE_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define FENCE_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define FENCE_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define SEMAPHORE_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define SEMAPHORE_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define EVENT_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define EVENT_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define QUERYPOOL_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define QUERYPOOL_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define BUFFER_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define BUFFER_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define BUFFERVIEW_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define BUFFERVIEW_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define IMAGE_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define IMAGE_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define IMAGEVIEW_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define IMAGEVIEW_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define SHADERMODULE_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define SHADERMODULE_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define PIPELINE_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define PIPELINE_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define SAMPLER_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define SAMPLER_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define DESCRIPTORSET_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define DESCRIPTORSET_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define FRAMEBUFFER_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define FRAMEBUFFER_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define RENDERPASS_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define RENDERPASS_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#define COMMANDPOOL_VK_FUNC(name) DEVICE_VK_FUNC(name)
#define COMMANDPOOL_VK_FUNC_EXT(name, extension) DEVICE_VK_FUNC(name)
#include "functionlist.inl"

	std::shared_ptr<Display> display; // can be null for headless

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	std::weak_ptr<CommandQueue> mainPresentQueue;
	std::array<std::shared_ptr<CommandQueue>, 8> queues;

	VmaVulkanFunctions vmaVulkanFunctions;
	VmaAllocator allocator;
	VkDeviceCreateInfo deviceCreateInfo;

	VmaAllocationCreateInfo cpuBufferCreateInfo;
	VmaAllocationCreateInfo gpuBufferCreateInfo;
	VmaAllocationCreateInfo imageCreateInfo;

};

}

#endif //WYRD_DEVICE_H
