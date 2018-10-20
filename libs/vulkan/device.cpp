

#include "core/core.h"
#include "vulkan/device.h"
#include "vulkan/display.h"
#include "vulkan/commandqueue.h"

namespace Vulkan
{
Device::Device(
		VkPhysicalDevice physicalDevice_,
		VkDeviceCreateInfo createInfo_,
		QueueFamilies const& queueFamilies_,
		uint32_t presentQ_) :
	physicalDevice(physicalDevice_)
{
	CHKED(vkCreateDevice(physicalDevice_, &createInfo_, nullptr, &device));
#define DEVICE_VULKAN_FUNC( name, level ) \
   _##name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   if( _##name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; }

#define DEVICE_VK_FUNC( name ) DEVICE_VULKAN_FUNC(name, device)

#define DEVICE_VK_FUNC_EXT( name, extension )            \
	for(auto j = 0u; j < createInfo_.enabledExtensionCount; ++j) \
	{ \
		auto const& ext = createInfo_.ppEnabledExtensionNames[j]; \
		if( std::string( ext ) == std::string( extension ) ) {\
			DEVICE_VULKAN_FUNC(name, device extension) \
		} \
	}
#include "functionlist.inl"

	vmaVulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vmaVulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vmaVulkanFunctions.vkAllocateMemory = _vkAllocateMemory;
	vmaVulkanFunctions.vkFreeMemory = _vkFreeMemory;
	vmaVulkanFunctions.vkMapMemory = _vkMapMemory;
	vmaVulkanFunctions.vkUnmapMemory = _vkUnmapMemory;
	vmaVulkanFunctions.vkFlushMappedMemoryRanges = _vkFlushMappedMemoryRanges;
	vmaVulkanFunctions.vkInvalidateMappedMemoryRanges = _vkInvalidateMappedMemoryRanges;
	vmaVulkanFunctions.vkBindBufferMemory = _vkBindBufferMemory;
	vmaVulkanFunctions.vkBindImageMemory = _vkBindImageMemory;
	vmaVulkanFunctions.vkGetBufferMemoryRequirements = _vkGetBufferMemoryRequirements;
	vmaVulkanFunctions.vkGetImageMemoryRequirements = _vkGetImageMemoryRequirements;
	vmaVulkanFunctions.vkCreateBuffer = _vkCreateBuffer;
	vmaVulkanFunctions.vkDestroyBuffer = _vkDestroyBuffer;
	vmaVulkanFunctions.vkCreateImage = _vkCreateImage;
	vmaVulkanFunctions.vkDestroyImage = _vkDestroyImage;

	VmaAllocatorCreateInfo create = {
		0,
		physicalDevice,
		device,
		0, 			// preferred block size default 256MB
		nullptr,	// allocation callbacks
		nullptr,	// memory callbacks
		1, 			// double buffer lost capable allocations
		nullptr,	// device size limits
		&vmaVulkanFunctions, 	// api pointer
		nullptr 	// recorder
	};

	CHKED(vmaCreateAllocator(&create, &allocator));

	for(auto i = 0u; i < createInfo_.queueCreateInfoCount; ++i)
	{
		uint32_t flavour = 0;
		flavour |= queueFamilies_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ? Render::CommandQueue::RenderFlavour : 0;
		flavour |= queueFamilies_[i].queueFlags & VK_QUEUE_COMPUTE_BIT ? Render::CommandQueue::ComputeFlavour : 0;
		flavour |= queueFamilies_[i].queueFlags & VK_QUEUE_TRANSFER_BIT ? Render::CommandQueue::BlitFlavour : 0;
		flavour |= i == presentQ_ ? Render::CommandQueue::PresentFlavour : 0;

		VkQueue q;
		vkGetDeviceQueue(i, 0, &q);
		auto que = queues.emplace_back(std::make_shared<CommandQueue>(q, flavour));
		if(que->isPresentFlavour() && mainPresentQueue.expired()) mainPresentQueue = que;
		if(que->isRenderFlavour() && mainRenderQueue.expired()) mainRenderQueue = que;
		if(que->isComputeFlavour() && mainComputeQueue.expired()) mainComputeQueue = que;
		if(que->isBlitFlavour() && mainBlitQueue.expired()) mainBlitQueue = que;
	}

}

Device::~Device()
{
	vkDestroyDevice(device, nullptr);
}

auto Device::getDisplay() const -> std::shared_ptr<Render::Display>
{
	return std::static_pointer_cast<Render::Display>(display);
};

auto Device::createSemaphore() -> Render::Semaphore
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;

	VkSemaphore semaphore{};
	CHKED(vkCreateSemaphore(&semaphoreCreateInfo, nullptr, &semaphore));
	return AsRenderSemaphore(semaphore);
}

auto Device::destroySemaphore(Render::Semaphore semaphore_) -> void
{
	vkDestroySemaphore(AsSemaphore(semaphore_), nullptr);
}

auto Device::createImage(VkImageCreateInfo const& createInfo_, VmaAllocationCreateInfo const& allocInfo_, VmaAllocationInfo& outInfo_) -> Image
{
	Image result{VK_NULL_HANDLE, VK_NULL_HANDLE };
	CHKED(vmaCreateImage(allocator, &createInfo_, &allocInfo_, &result.first, &result.second, &outInfo_));

	return result;
}
auto Device::destroyImage(Image const& image_) -> void
{
	vmaDestroyImage(allocator, image_.first, image_.second);
}

auto Device::createSwapchain(VkSwapchainCreateInfoKHR const& createInfo_) -> VkSwapchainKHR
{
	VkSwapchainKHR swapchainKHR;
	CHKED(vkCreateSwapchainKHR(&createInfo_, nullptr, &swapchainKHR));
	return swapchainKHR;
}

auto Device::destroySwapchain(VkSwapchainKHR swapchain_) -> void
{
	vkDestroySwapchainKHR(swapchain_, nullptr);
}

auto Device::createSemaphore(VkSemaphoreCreateInfo const& createInfo_) -> VkSemaphore {
	VkSemaphore semaphore;
	CHKED(vkCreateSemaphore(&createInfo_, nullptr, &semaphore));
	return semaphore;
}

auto Device::destroySemaphore(VkSemaphore semaphore_) -> void {
	vkDestroySemaphore(semaphore_, nullptr);
}

auto Device::getMainRenderQueue() -> Render::CommandQueue::Ptr
{
	return std::static_pointer_cast<Render::CommandQueue>(mainRenderQueue.lock());
}
auto Device::getMainComputeQueue() ->Render::CommandQueue::Ptr
{
	return std::static_pointer_cast<Render::CommandQueue>(mainComputeQueue.lock());

}
auto Device::getMainBlitQueue() -> Render::CommandQueue::Ptr
{
	return std::static_pointer_cast<Render::CommandQueue>(mainBlitQueue.lock());
}
auto Device::getMainPresentQueue() -> Render::CommandQueue::Ptr
{
	return std::static_pointer_cast<Render::CommandQueue>(mainPresentQueue.lock());
}



}
