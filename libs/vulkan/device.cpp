

#include "core/core.h"
#include "vulkan/device.h"
#include "vulkan/display.h"
#include "vulkan/commandqueue.h"
#include "encoder.h"
#include <unordered_set>

namespace {
auto ToQueueIndex(bool render_, bool compute_, bool blit_) -> int
{
	int index = render_ ? Render::CommandQueue::RenderFlavour : 0;
	index |= compute_ ? Render::CommandQueue::ComputeFlavour : 0;
	index |= blit_ ? Render::CommandQueue::BlitFlavour : 0;
	return index;
}
}

namespace Vulkan
{


Device::Device(
		VkPhysicalDevice physicalDevice_,
		VkDeviceCreateInfo createInfo_,
		QueueFamilies const& queueFamilies_,
		uint32_t presentQ_) : physicalDevice(physicalDevice_), deviceCreateInfo(createInfo_)
{
	CHKED(vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &device));

	std::unordered_set<char const *> extensions;
	for(auto j = 0u; j < deviceCreateInfo.enabledExtensionCount; ++j)
	{
		auto const& ext = deviceCreateInfo.ppEnabledExtensionNames[j];
		extensions.insert(ext);
	}

#define DEVICE_VULKAN_FUNC( name, level ) \
   deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; }

#define QUEUE_VULKAN_FUNC(name, level) \
   queueVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   if( queueVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; }

#define GRAPHICS_CB_VULKAN_FUNC(name, level) \
   graphicsCBVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   if( graphicsCBVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; }

#define COMPUTE_CB_VULKAN_FUNC(name, level) \
   computeCBVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   if( computeCBVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; }

#define TRANSFER_CB_VULKAN_FUNC(name, level) \
   transferCBVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   if( transferCBVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; }

#define DEVICE_VK_FUNC(name) \
    DEVICE_VULKAN_FUNC(name, device)
#define DEVICE_VK_FUNC_EXT(name, extension) \
    if( extensions.find(extension)  != extensions.end()) DEVICE_VULKAN_FUNC(name, device extension);

#define QUEUE_VK_FUNC(name) \
    QUEUE_VULKAN_FUNC(name, queue)
#define QUEUE_VK_FUNC_EXT(name, extension) \
    if( extensions.find(extension)  != extensions.end()) QUEUE_VULKAN_FUNC(name, queue extension);

#define GRAPHICS_CB_VK_FUNC(name) \
    GRAPHICS_CB_VULKAN_FUNC(name, graphics command buffer)
#define GRAPHICS_CB_VK_FUNC_EXT(name, extension) \
    if( extensions.find(extension)  != extensions.end()) GRAPHICS_CB_VULKAN_FUNC(name, graphic command buffer extension);

#define COMPUTE_CB_VK_FUNC(name) \
    COMPUTE_CB_VULKAN_FUNC(name, compute command buffer)
#define COMPUTE_CB_VK_FUNC_EXT(name, extension) \
    if( extensions.find(extension)  != extensions.end()) COMPUTE_CB_VULKAN_FUNC(name, compute command buffer extension);

#define TRANSFER_CB_VK_FUNC(name) \
    TRANSFER_CB_VULKAN_FUNC(name, transfer command buffer)
#define TRANSFER_CB_VK_FUNC_EXT(name, extension) \
    if( extensions.find(extension)  != extensions.end()) TRANSFER_VULKAN_FUNC(name, transfer commadn buffer extension);

#include "functionlist.inl"

	vmaVulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vmaVulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vmaVulkanFunctions.vkAllocateMemory = deviceVkVTable.vkAllocateMemory;
	vmaVulkanFunctions.vkFreeMemory = deviceVkVTable.vkFreeMemory;
	vmaVulkanFunctions.vkMapMemory = deviceVkVTable.vkMapMemory;
	vmaVulkanFunctions.vkUnmapMemory = deviceVkVTable.vkUnmapMemory;
	vmaVulkanFunctions.vkFlushMappedMemoryRanges = deviceVkVTable.vkFlushMappedMemoryRanges;
	vmaVulkanFunctions.vkInvalidateMappedMemoryRanges = deviceVkVTable.vkInvalidateMappedMemoryRanges;
	vmaVulkanFunctions.vkBindBufferMemory = deviceVkVTable.vkBindBufferMemory;
	vmaVulkanFunctions.vkBindImageMemory = deviceVkVTable.vkBindImageMemory;
	vmaVulkanFunctions.vkGetBufferMemoryRequirements = deviceVkVTable.vkGetBufferMemoryRequirements;
	vmaVulkanFunctions.vkGetImageMemoryRequirements = deviceVkVTable.vkGetImageMemoryRequirements;
	vmaVulkanFunctions.vkCreateBuffer = deviceVkVTable.vkCreateBuffer;
	vmaVulkanFunctions.vkDestroyBuffer = deviceVkVTable.vkDestroyBuffer;
	vmaVulkanFunctions.vkCreateImage = deviceVkVTable.vkCreateImage;
	vmaVulkanFunctions.vkDestroyImage = deviceVkVTable.vkDestroyImage;

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

	std::vector<CommandQueue::Ptr> cqs;

	for(auto i = 0u; i < createInfo_.queueCreateInfoCount; ++i)
	{
		uint32_t flavour = 0;
		flavour |= queueFamilies_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ? Render::CommandQueue::RenderFlavour : 0;
		flavour |= queueFamilies_[i].queueFlags & VK_QUEUE_COMPUTE_BIT ? Render::CommandQueue::ComputeFlavour : 0;
		flavour |= queueFamilies_[i].queueFlags & VK_QUEUE_TRANSFER_BIT ? Render::CommandQueue::BlitFlavour : 0;
		flavour |= i == presentQ_ ? Render::CommandQueue::PresentFlavour : 0;

		VkQueue q;
		vkGetDeviceQueue(i, 0, &q);
		auto que = cqs.emplace_back(std::make_shared<CommandQueue>(&queueVkVTable, q, i, flavour));
	}

	for(auto que : cqs)
	{
		int index = que->getFlavour() & 0x7;
		queues[index] = que;
		if(que->isPresentFlavour() && mainPresentQueue.expired())
		{
			mainPresentQueue = que;
		}
	}

	// now spread multi-capable queues to any holes in the array
	// easy case is if we have a fully flexible queue
	for(auto i = 0u; i < 6; ++i)
	{
		if(queues[i] != nullptr) continue;

		if(queues[0x7] != nullptr) queues[i] = queues[0x7];
		else
		{
			if(queues[0x6] != nullptr && ((i & 0x2) || (i & 0x4))) queues[i] = queues[0x6];
			if(queues[0x5] != nullptr && ((i & 0x1) || (i & 0x4))) queues[i] = queues[0x5];
			if(queues[0x3] != nullptr && ((i & 0x1) || (i & 0x2))) queues[i] = queues[0x3];
		}
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

auto Device::createEncoderPool(bool frameLifetime_, uint32_t queueFlavour_) -> std::shared_ptr<Render::EncoderPool>
{
	assert(queueFlavour_ != 0);

	VkCommandPoolCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.pNext = nullptr;

	createInfo.flags = 0;
	createInfo.flags |= frameLifetime_ ? VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0;

	auto queue = queues[queueFlavour_ & 0x7];
	assert(queue != nullptr);
	createInfo.queueFamilyIndex = queue->getFamilyIndex();
	VkCommandPool commandPool;
	CHKED(vkCreateCommandPool(&createInfo, nullptr, &commandPool));

	auto encoderPool = std::make_shared<EncoderPool>(device,
													 commandPool,
													 &deviceVkVTable,
													 &graphicsCBVkVTable,
													 &computeCBVkVTable,
													 &transferCBVkVTable);
	return encoderPool;
}

auto Device::destroyEncoderPool() -> void
{

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
	return queues[ToQueueIndex(true, false, false)];
}

auto Device::getMainComputeQueue() ->Render::CommandQueue::Ptr
{
	return queues[ToQueueIndex(false, true, false)];
}
auto Device::getMainBlitQueue() -> Render::CommandQueue::Ptr
{
	return queues[ToQueueIndex(false, false, true)];
}
auto Device::getMainPresentQueue() -> Render::CommandQueue::Ptr
{
	return mainPresentQueue.lock();
}

}
