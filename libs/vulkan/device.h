#pragma once
#ifndef WYRD_VULKAN_DEVICE_H
#define WYRD_VULKAN_DEVICE_H

#include "core/core.h"
#include "render/device.h"
#include "render/commandqueue.h"

#include "vulkan/api.h"
#include "vulkan/vk_mem_alloc.h"
#include "texture.h"
#include <array>

namespace Vulkan {
struct CommandQueue;
struct Display;

struct Device : public Render::Device, std::enable_shared_from_this<Device>
{
public:
	friend class System;

	friend struct Display;

	using Ptr = std::shared_ptr<Device>;
	using WeakPtr = std::weak_ptr<Device>;
	using QueueFamilies = std::vector<VkQueueFamilyProperties>;

	Device( VkPhysicalDevice physicalDevice_,
			VkDeviceCreateInfo createInfo_,
			QueueFamilies const& queueFamilies_,
			uint32_t presentQ_ = ~0);
	~Device();

	auto getVkDevice() -> VkDevice { return device; }

	// Render::Device interface
	auto getDisplay() const -> std::shared_ptr<Render::Display> final;

	auto makeEncoderPool(bool frameLifetime_, uint32_t queueType_) -> std::shared_ptr<Render::EncoderPool> final;
	auto makeFence() -> std::shared_ptr<Render::Fence> final;
	auto makeSemaphore() -> std::shared_ptr<Render::Semaphore> final;

	auto getMainRenderQueue() -> Render::CommandQueue::Ptr final;
	auto getMainComputeQueue() -> Render::CommandQueue::Ptr final;
	auto getMainBlitQueue() -> Render::CommandQueue::Ptr final;
	auto getMainPresentQueue() -> Render::CommandQueue::Ptr final;

	// Vulkan specific functions
	auto getPhysicalDevice() const -> VkPhysicalDevice { return physicalDevice; }

	auto destroyQueue(VkQueue const& queue_) -> void;

	auto createImage(VkImageCreateInfo const& createInfo_, VmaAllocationCreateInfo const& allocInfo_,
					 VmaAllocationInfo& outInfo_) -> std::pair<VkImage, VmaAllocation>;
	auto destroyImage(std::pair<VkImage, VmaAllocation> const& image_) -> void;

	auto createImageView(VkImageViewCreateInfo const& createInfo_) -> VkImageView;
	auto destroyImageView(VkImageView const& texture_) -> void;

	auto createSwapchain(VkSwapchainCreateInfoKHR const& createInfo_) -> VkSwapchainKHR;
	auto destroySwapchain(VkSwapchainKHR swapchain_) -> void;

	auto createSemaphore(VkSemaphoreCreateInfo const& createInfo_) -> VkSemaphore;
	auto destroySemaphore(VkSemaphore semaphore_) -> void;

	auto createFence(VkFenceCreateInfo const& createInfo_) -> VkFence;
	auto destroyFence(VkFence semaphore_) -> void;

	auto createEncoderPool(VkCommandPoolCreateInfo const& createInfo_) -> VkCommandPool;
	auto destroyEncoderPool(VkCommandPool const& commandPool_) -> void;

	auto createRenderPass(VkRenderPassCreateInfo const& createInfo_) -> VkRenderPass;
	auto destroyRenderPass(VkRenderPass renderPass_) -> void;

	auto createRenderTarget(VkFramebufferCreateInfo const& createInfo_) -> VkFramebuffer;
	auto destroyRenderTarget(VkFramebuffer frameBuffer_) -> void;

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

	VkAllocationCallbacks allocationCallbacks;

};

}

#endif //WYRD_DEVICE_H
