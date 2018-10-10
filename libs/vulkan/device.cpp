

#include "core/core.h"
#include "vulkan/device.h"

namespace Vulkan
{
Device::Device(	VkPhysicalDevice physicalDevice_,
				VkDevice device_,
				uint32_t renderQ_,
				uint32_t computeQ_,
				uint32_t transferQ_) :
	physicalDevice(physicalDevice_),
	device(device_)
{
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

}

Device::~Device()
{
	vkDestroyDevice(device, nullptr);
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

}
