#pragma once
#ifndef WYRD_VULKAN_DEVICE_H
#define WYRD_VULKAN_DEVICE_H

#include "core/core.h"
#include "core/freelist.h"
#include "render/device.h"
#include "render/commandqueue.h"

#include "vulkan/api.h"
#include "vulkan/vk_mem_alloc.h"
#include <array>

namespace Vulkan {
struct CommandQueue;
struct Display;
struct EncoderPool;
struct Texture;
struct Buffer;

struct Device : public Render::Device, std::enable_shared_from_this<Device>
{
public:
	friend class System;

	friend struct Display;

	using Ptr = std::shared_ptr<Device>;
	using WeakPtr = std::weak_ptr<Device>;
	using QueueFamilies = std::vector<VkQueueFamilyProperties>;

	Device(std::shared_ptr<ResourceManager::ResourceMan> resourceMan_,
		   bool renderCapable_,
		   VkPhysicalDevice physicalDevice_,
		   VkDeviceCreateInfo createInfo_,
		   QueueFamilies const& queueFamilies_,
		   uint32_t presentQ_ = ~0);
	~Device();

	// Render::Device interface
	auto getDisplay() const -> std::weak_ptr<Render::Display> final;
	auto houseKeepTick() -> void final;

	auto makeEncoderPool(bool frameLifetime_,
						 Render::CommandQueueFlavour flavour_) -> std::shared_ptr<Render::EncoderPool> final;
	auto makeFence() -> std::shared_ptr<Render::Fence> final;
	auto makeSemaphore() -> std::shared_ptr<Render::Semaphore> final;

	auto getGeneralQueue() -> Render::CommandQueue::Ptr final;
	auto getRenderSpecificQueue() -> Render::CommandQueue::Ptr final;
	auto getComputeSpecificQueue() -> Render::CommandQueue::Ptr final;
	auto getDMASpecificQueue() -> Render::CommandQueue::Ptr final;
	auto getPresentQueue() -> Render::CommandQueue::Ptr final;

	// Vulkan specific functions
	auto getVkDevice() -> VkDevice { return device; }

	auto upload(uint8_t const* data_, uint64_t size_, Texture const* dst_) -> void;
	auto fill(uint32_t value_, Texture const* dst_) -> void;

	auto upload(uint8_t const* data_, uint64_t size_, Buffer const* dst_) -> void;
	auto fill(uint32_t value_, Buffer const* dst_) -> void;

	auto mapMemory(VmaAllocation const& alloc_) -> void*;
	auto unmapMemory(VmaAllocation const& alloc_) -> void;
	auto flushMemory(VmaAllocation const& alloc_) -> void;

	auto getPhysicalDevice() const -> VkPhysicalDevice { return physicalDevice; }

	auto destroyQueue(VkQueue const& queue_) -> void;

	auto createImage(VkImageCreateInfo const& createInfo_, VmaAllocationCreateInfo const& allocInfo_,
					 VmaAllocationInfo& outInfo_) -> std::pair<VkImage, VmaAllocation>;
	auto destroyImage(std::pair<VkImage, VmaAllocation> const& image_) -> void;

	auto createBuffer(VkBufferCreateInfo const& createInfo_, VmaAllocationCreateInfo const& allocInfo_,
					  VmaAllocationInfo& outInfo_) -> std::pair<VkBuffer, VmaAllocation>;
	auto destroyBuffer(std::pair<VkBuffer, VmaAllocation> const& buffer_) -> void;

	auto createImageView(VkImageViewCreateInfo const& createInfo_) -> VkImageView;
	auto destroyImageView(VkImageView const& texture_) -> void;

	auto createSwapchain(VkSwapchainCreateInfoKHR const& createInfo_) -> VkSwapchainKHR;
	auto destroySwapchain(VkSwapchainKHR swapchain_) -> void;

	auto createSemaphore(VkSemaphoreCreateInfo const& createInfo_) -> VkSemaphore;
	auto destroySemaphore(VkSemaphore semaphore_) -> void;

	auto createFence(VkFenceCreateInfo const& createInfo_) -> VkFence;
	auto destroyFence(VkFence semaphore_) -> void;

	auto createCommandPool(VkCommandPoolCreateInfo const& createInfo_) -> VkCommandPool;
	auto destroyCommandPool(VkCommandPool const& commandPool_) -> void;

	auto createRenderPass(VkRenderPassCreateInfo const& createInfo_) -> VkRenderPass;
	auto destroyRenderPass(VkRenderPass renderPass_) -> void;

	auto createFramebuffer(VkFramebufferCreateInfo const& createInfo_) -> VkFramebuffer;
	auto destroyFramebuffer(VkFramebuffer frameBuffer_) -> void;

	auto createGraphicsPipeline(VkGraphicsPipelineCreateInfo const& createInfo_) -> VkPipeline;
	auto createComputePipeline(VkComputePipelineCreateInfo const& createInfo_) -> VkPipeline;
	auto destroyPipeline(VkPipeline pipeline_) -> void;

	auto createShaderModule(VkShaderModuleCreateInfo const& createInfo_) -> VkShaderModule;
	auto destroyShaderModule(VkShaderModule shaderModule_) -> void;

	auto createDescriptorPool(VkDescriptorPoolCreateInfo const& createInfo_) -> VkDescriptorPool;
	auto destroyDescriptorPool(VkDescriptorPool descriptorPool_) -> void;

	auto createDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo const& createInfo_) -> VkDescriptorSetLayout;
	auto destroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout_) -> void;

	auto allocDescriptorSet(VkDescriptorSetAllocateInfo const& allocInfo, std::vector<VkDescriptorSet>& out_) -> void;
	auto freeDescriptorSet(std::vector<VkDescriptorSet> const& descriptorSets_) -> void;

	auto createPipelineLayout(VkPipelineLayoutCreateInfo const& createInfo) -> VkPipelineLayout;
	auto destroyPipelineLayout(VkPipelineLayout pipelineLayout_) -> void;

	auto createSampler(VkSamplerCreateInfo const& createInfo) -> VkSampler;
	auto destroySampler(VkSampler sampler_) -> void;

	auto getDescriptorPool() -> VkDescriptorPool { return descriptorPool; }

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
	GeneralCBVkVTable generalCBVkVTable;

	auto debugNameVkObject(uint64_t object_, VkDebugReportObjectTypeEXT type_, char const* name_) -> void;

private:
	void upload(VkImage cpuImage, Texture const* dst_);
	void upload(VkBuffer cpuImage, Buffer const* dst_);

	DeviceVkVTable vtable;

#define DEVICE_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable. name(device, args...); }
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

	// hold a reference to the resource manager we were created with
	std::shared_ptr<ResourceManager::ResourceMan> resourceMan;

	bool renderCapable;
	std::shared_ptr<Display> display; // can be null for headless

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	std::weak_ptr<CommandQueue> presentQueue;
	std::shared_ptr<CommandQueue> renderSpecificQueue;
	std::shared_ptr<CommandQueue> computeSpecificQueue;
	std::shared_ptr<CommandQueue> dmaOnlyQueue;
	std::shared_ptr<CommandQueue> allQueue;

	std::shared_ptr<EncoderPool> dmaEncoderPool;

	VmaVulkanFunctions vmaVulkanFunctions;
	VmaAllocator allocator;
	VkDeviceCreateInfo deviceCreateInfo;

	VmaAllocationCreateInfo cpuBufferCreateInfo;
	VmaAllocationCreateInfo gpuBufferCreateInfo;
	VmaAllocationCreateInfo imageCreateInfo;

	VkAllocationCallbacks allocationCallbacks;

	// for now we have a single pool
	VkDescriptorPool descriptorPool;
	Core::MTFreeList<VkFence, uint32_t> fenceFreeList;
};

}

#endif //WYRD_DEVICE_H
