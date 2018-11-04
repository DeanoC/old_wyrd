#include "core/core.h"
#include "render/types.h"
#include "render/gtfcracker.h"
#include "vulkan/device.h"
#include "vulkan/commandqueue.h"
#include "vulkan/display.h"
#include "vulkan/encoder.h"
#include "vulkan/fence.h"
#include "vulkan/semaphore.h"
#include "vulkan/renderpass.h"
#include "vulkan/rendertarget.h"
#include "vulkan/vkfcracker.h"
#include <unordered_set>
#include <stdlib.h>

namespace {
auto VKAPI_PTR VulkanAlloc(void* pUserData, size_t size, size_t alignment,
						   VkSystemAllocationScope allocationScope) -> void*
{
	assert(alignment <= alignof(max_align_t));
	return std::malloc(size); //
}

auto VKAPI_PTR VulkanFree(void* pUserData, void* pMemory) -> void
{
	std::free(pMemory);
}

auto VKAPI_PTR VulkanRealloc(void* pUserData, void* pOriginal, size_t size, size_t alignment,
							 VkSystemAllocationScope allocationScope) -> void*
{
	assert(alignment <= alignof(max_align_t));
	return std::realloc(pOriginal, size);
}

auto VKAPI_PTR VulkanInternalAllocNotify(void* pUserData, size_t size, VkInternalAllocationType allocationType,
										 VkSystemAllocationScope allocationScope) -> void
{
}

auto VKAPI_PTR VulkanInternalFreeNotify(void* pUserData, size_t size, VkInternalAllocationType allocationType,
										VkSystemAllocationScope allocationScope) -> void
{
}

}


namespace Vulkan {

Device::Device(std::shared_ptr<ResourceManager::ResourceMan> resourceMan_,
			   bool renderCapable_,
			   VkPhysicalDevice physicalDevice_,
			   VkDeviceCreateInfo createInfo_,
			   QueueFamilies const& queueFamilies_,
			   uint32_t presentQ_) :
		resourceMan(resourceMan_),
		renderCapable(renderCapable_),
		physicalDevice(physicalDevice_),
		deviceCreateInfo(createInfo_),
		deviceVkVTable{},
		fenceVkVTable{},
		semaphoreVkVTable{},
		eventVkVTable{},
		queryPoolVkVTable{},
		bufferVkVTable{},
		bufferViewVkVTable{},
		imageVkVTable{},
		imageViewVkVTable{},
		shaderModuleVkVTable{},
		pipelineVkVTable{},
		samplerVkVTable{},
		descriptorSetVkVTable{},
		framebufferVkVTable{},
		renderPassVkVTable{},
		commandPoolVkVTable{},
		queueVkVTable{},
		graphicsCBVkVTable{},
		computeCBVkVTable{}
{

	allocationCallbacks.pfnAllocation = &VulkanAlloc;
	allocationCallbacks.pfnReallocation = &VulkanRealloc;
	allocationCallbacks.pfnFree = &VulkanFree;
	allocationCallbacks.pfnInternalAllocation = &VulkanInternalAllocNotify;
	allocationCallbacks.pfnInternalFree = &VulkanInternalFreeNotify;
	allocationCallbacks.pUserData = nullptr;

	CHKED(vkCreateDevice(physicalDevice_, &deviceCreateInfo, &allocationCallbacks, &device));

	std::unordered_set<std::string> extensions;
	for(auto j = 0u; j < deviceCreateInfo.enabledExtensionCount; ++j)
	{
		auto const& ext = deviceCreateInfo.ppEnabledExtensionNames[j];
		extensions.insert(ext);
	}

	{
#define DEVICE_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define FENCE_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   fenceVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define SEMAPHORE_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   semaphoreVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define EVENT_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   eventVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define QUERYPOOL_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   queryPoolVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define BUFFER_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   bufferVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define BUFFERVIEW_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   bufferViewVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define IMAGE_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   imageVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define IMAGEVIEW_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   imageViewVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define SHADERMODULE_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   shaderModuleVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define PIPELINE_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   pipelineVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define SAMPLER_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   samplerVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define DESCRIPTORSET_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   descriptorSetVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define FRAMEBUFFER_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   framebufferVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define RENDERPASS_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   renderPassVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define COMMANDPOOL_VULKAN_FUNC(name, level) \
   { deviceVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   commandPoolVkVTable. name = deviceVkVTable. name;       \
   if( deviceVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define QUEUE_VULKAN_FUNC(name, level) \
   { queueVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   if( queueVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define GRAPHICS_CB_VULKAN_FUNC(name, level) \
   { graphicsCBVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   if( graphicsCBVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define COMPUTE_CB_VULKAN_FUNC(name, level) \
   { computeCBVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   if( computeCBVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define GENERAL_CB_VULKAN_FUNC(name, level) \
   { graphicsCBVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   computeCBVkVTable. name = graphicsCBVkVTable. name;       \
   generalCBVkVTable. name = graphicsCBVkVTable. name;      \
   if( graphicsCBVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define GFXCOMP_CB_VULKAN_FUNC(name, level) \
   { graphicsCBVkVTable. name = (PFN_##name)vkGetDeviceProcAddr( device, #name );       \
   computeCBVkVTable. name = graphicsCBVkVTable. name;       \
   if( graphicsCBVkVTable. name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; } }

#define CHK_EXT(extension) if( extensions.find(extension) != extensions.end())

#define DEVICE_VK_FUNC(name) DEVICE_VULKAN_FUNC(name, device);
#define DEVICE_VK_FUNC_EXT(name, extension) CHK_EXT(extension) DEVICE_VULKAN_FUNC(name, device extension);
#define FENCE_VK_FUNC(name) FENCE_VULKAN_FUNC(name, fence);
#define FENCE_VK_FUNC_EXT(name, extension) CHK_EXT(extension) FENCE_VULKAN_FUNC(name, fence extension);
#define SEMAPHORE_VK_FUNC(name) SEMAPHORE_VULKAN_FUNC(name, semaphore);
#define SEMAPHORE_VK_FUNC_EXT(name, extension) CHK_EXT(extension) SEMAPHORE_VULKAN_FUNC(name, semaphore extension);
#define EVENT_VK_FUNC(name) EVENT_VULKAN_FUNC(name, event);
#define EVENT_VK_FUNC_EXT(name, extension) CHK_EXT(extension) EVENT_VULKAN_FUNC(name, event extension);
#define QUERYPOOL_VK_FUNC(name) QUERYPOOL_VULKAN_FUNC(name, querypool);
#define QUERYPOOL_VK_FUNC_EXT(name, extension) CHK_EXT(extension) QUERYPOOL_VULKAN_FUNC(name, querypool extension);
#define BUFFER_VK_FUNC(name) BUFFER_VULKAN_FUNC(name, buffer);
#define BUFFER_VK_FUNC_EXT(name, extension) CHK_EXT(extension) BUFFER_VULKAN_FUNC(name, buffer extension);
#define BUFFERVIEW_VK_FUNC(name) BUFFERVIEW_VULKAN_FUNC(name, bufferview);
#define BUFFERVIEW_VK_FUNC_EXT(name, extension) CHK_EXT(extension)  BUFFERVIEW_VULKAN_FUNC(name, bufferview extension);
#define IMAGE_VK_FUNC(name) IMAGE_VULKAN_FUNC(name, image);
#define IMAGE_VK_FUNC_EXT(name, extension) CHK_EXT(extension) IMAGE_VULKAN_FUNC(name, image extension);
#define IMAGEVIEW_VK_FUNC(name) IMAGEVIEW_VULKAN_FUNC(name, imageview);
#define IMAGEVIEW_VK_FUNC_EXT(name, extension) CHK_EXT(extension)  IMAGEVIEW_VULKAN_FUNC(name, imageview extension);
#define SHADERMODULE_VK_FUNC(name) SHADERMODULE_VULKAN_FUNC(name, shadermodule);
#define SHADERMODULE_VK_FUNC_EXT(name, extension) CHK_EXT(extension)  SHADERMODULE_VULKAN_FUNC(name, shadermodule extension);
#define PIPELINE_VK_FUNC(name) PIPELINE_VULKAN_FUNC(name, pipeline);
#define PIPELINE_VK_FUNC_EXT(name, extension) CHK_EXT(extension) PIPELINE_VULKAN_FUNC(name, pipeline extension);
#define SAMPLER_VK_FUNC(name) SAMPLER_VULKAN_FUNC(name, sampler);
#define SAMPLER_VK_FUNC_EXT(name, extension) CHK_EXT(extension) SAMPLER_VULKAN_FUNC(name, sampler extension);
#define DESCRIPTORSET_VK_FUNC(name) DESCRIPTORSET_VULKAN_FUNC(name, descriptorset);
#define DESCRIPTORSET_VK_FUNC_EXT(name, extension) CHK_EXT(extension) DESCRIPTORSET_VULKAN_FUNC(name, descriptorset extension);
#define FRAMEBUFFER_VK_FUNC(name) FRAMEBUFFER_VULKAN_FUNC(name, framebuffer);
#define FRAMEBUFFER_VK_FUNC_EXT(name, extension) CHK_EXT(extension) FRAMEBUFFER_VULKAN_FUNC(name, framebuffer extension);
#define RENDERPASS_VK_FUNC(name) RENDERPASS_VULKAN_FUNC(name, renderpass);
#define RENDERPASS_VK_FUNC_EXT(name, extension) CHK_EXT(extension)  RENDERPASS_VULKAN_FUNC(name, renderpass extension);
#define COMMANDPOOL_VK_FUNC(name) COMMANDPOOL_VULKAN_FUNC(name, commandpool);
#define COMMANDPOOL_VK_FUNC_EXT(name, extension) CHK_EXT(extension) COMMANDPOOL_VULKAN_FUNC(name, commandpool extension);
#define QUEUE_VK_FUNC(name) QUEUE_VULKAN_FUNC(name, queue);
#define QUEUE_VK_FUNC_EXT(name, extension) CHK_EXT(extension) QUEUE_VULKAN_FUNC(name, queue extension);
#define GRAPHIC_CB_VK_FUNC(name) GRAPHICS_CB_VULKAN_FUNC(name, graphics command buffer);
#define GRAPHIC_CB_VK_FUNC_EXT(name, extension) CHK_EXT(extension) GRAPHICS_CB_VULKAN_FUNC(name, graphic command buffer extension);
#define COMPUTE_CB_VK_FUNC(name) COMPUTE_CB_VULKAN_FUNC(name, compute command buffer);
#define COMPUTE_CB_VK_FUNC_EXT(name, extension) CHK_EXT(extension) COMPUTE_CB_VULKAN_FUNC(name, compute command buffer extension);
#define GENERAL_CB_VK_FUNC(name) GENERAL_CB_VULKAN_FUNC(name, transfer command buffer);
#define GENERAL_CB_VK_FUNC_EXT(name, extension) CHK_EXT(extension) GENERAL_CB_VULKAN_FUNC(name, transfer command buffer extension);
#define GFXCOMP_CB_VK_FUNC(name) GFXCOMP_CB_VULKAN_FUNC(name, graphics & compute command buffer);
#define GFXCOMP_CB_VK_FUNC_EXT(name, extension) CHK_EXT(extension) GFXCOMP_CB_VULKAN_FUNC(name, graphics & compute command buffer extension);


#include "functionlist.inl"

#undef CHK_EXT
	}
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

	VmaAllocatorCreateInfo create = {0, physicalDevice, device, 0,            // preferred block size default 256MB
									 &allocationCallbacks,    // allocation callbacks
									 nullptr,    // memory callbacks
									 1,            // double buffer lost capable allocations
									 nullptr,    // device size limits
									 &vmaVulkanFunctions,    // api pointer
									 nullptr    // recorder
	};

	CHKED(vmaCreateAllocator(&create, &allocator));

	std::vector<CommandQueue::Ptr> cqs;

	for(auto i = 0u; i < createInfo_.queueCreateInfoCount; ++i)
	{
		using namespace Render;
		using namespace Core::bitmask;

		CommandQueueFlavour flavour{zero<CommandQueueFlavour>()};
		flavour |= (renderCapable && queueFamilies_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ?
				   CommandQueueFlavour::Render : zero<CommandQueueFlavour>();
		flavour |= (queueFamilies_[i].queueFlags & VK_QUEUE_COMPUTE_BIT) ?
				   CommandQueueFlavour::Compute : zero<CommandQueueFlavour>();
		flavour |= queueFamilies_[i].queueFlags & VK_QUEUE_TRANSFER_BIT ?
				   CommandQueueFlavour::DMA : zero<CommandQueueFlavour>();
		flavour |= i == presentQ_ ?
				   CommandQueueFlavour::Present : zero<CommandQueueFlavour>();

		VkQueue q;
		vkGetDeviceQueue(i, 0, &q);
		auto que = cqs.emplace_back(std::make_shared<CommandQueue>(
				device,
				&queueVkVTable,
				q,
				i,
				flavour));
	}

	for(auto que : cqs)
	{
		using namespace Render;
		using namespace Core::bitmask;
		if(!renderSpecificQueue && test_equal(que->getFlavour(), CommandQueueFlavour::Render |
																 CommandQueueFlavour::DMA))
		{
			renderSpecificQueue = que;
		}
		if(!computeSpecificQueue && test_equal(que->getFlavour(), CommandQueueFlavour::Compute |
																  CommandQueueFlavour::DMA))
		{
			computeSpecificQueue = que;
		}
		if(!dmaOnlyQueue && test_equal(que->getFlavour(), CommandQueueFlavour::DMA))
		{
			dmaOnlyQueue = que;
		}
		if(!allQueue && test_equal(que->getFlavour(), CommandQueueFlavour::Render |
													  CommandQueueFlavour::Compute |
													  CommandQueueFlavour::DMA))
		{
			allQueue = que;
		}

		if(que->isPresentFlavour() && presentQueue.expired())
		{
			presentQueue = que;
		}
	}

	// if we don't have specific queues, use multiple capable one
	if(renderCapable)
	{
		// currently render capable requires 1 general queue
		assert(allQueue);
		renderSpecificQueue = renderSpecificQueue ? renderSpecificQueue : allQueue;
		computeSpecificQueue = computeSpecificQueue ? computeSpecificQueue : allQueue;
		dmaOnlyQueue = dmaOnlyQueue ? dmaOnlyQueue : allQueue;
	} else
	{
		// compute requires a compute queue
		assert(computeSpecificQueue);
		dmaOnlyQueue = dmaOnlyQueue ? dmaOnlyQueue : computeSpecificQueue;
	}

}

Device::~Device()
{
	allQueue.reset();
	dmaOnlyQueue.reset();
	computeSpecificQueue.reset();
	renderSpecificQueue.reset();

	dmaEncoderPool.reset();
	vkDestroyDevice(device, &allocationCallbacks);
}

auto Device::getDisplay() const -> std::shared_ptr<Render::Display>
{
	return std::static_pointer_cast<Render::Display>(display);
};

auto Device::upload(uint8_t* data_, uint32_t size_, VkImageCreateInfo const& createInfo_,
					std::shared_ptr<Render::Texture> const& dst_) -> void
{
	if(!dmaEncoderPool)
	{
		using namespace Render;
		dmaEncoderPool = std::static_pointer_cast<EncoderPool>(
				makeEncoderPool(true, CommandQueueFlavour::DMA));
	}
	// create CPU side texture
	VkImageCreateInfo cpuCreateInfo = createInfo_;
	cpuCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	cpuCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	cpuCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	VmaAllocationCreateInfo cpuAllocInfo{};
	cpuAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	cpuAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	VmaAllocationInfo cpuInfo;
	auto[cpuImage, cpuAlloc] = createImage(cpuCreateInfo, cpuAllocInfo, cpuInfo);

	std::memcpy(cpuInfo.pMappedData, data_, size_);

	upload(cpuImage, dst_);

	destroyImage({cpuImage, cpuAlloc});
}

auto Device::fill(uint32_t value_, VkImageCreateInfo const& createInfo_,
				  std::shared_ptr<Render::Texture> const& dst_) -> void
{
	if(!dmaEncoderPool)
	{
		using namespace Render;
		dmaEncoderPool = std::static_pointer_cast<EncoderPool>(
				makeEncoderPool(true, CommandQueueFlavour::DMA));
	}

	// create CPU side texture
	VkImageCreateInfo cpuCreateInfo = createInfo_;
	cpuCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	cpuCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	cpuCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	VmaAllocationCreateInfo cpuAllocInfo{};
	cpuAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	cpuAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	VmaAllocationInfo cpuInfo;
	auto[cpuImage, cpuAlloc] = createImage(cpuCreateInfo, cpuAllocInfo, cpuInfo);

	for(auto i = 0u; i < cpuInfo.size / sizeof(uint32_t); ++i)
	{
		((uint32_t*) cpuInfo.pMappedData)[i] = value_;
	}

	upload(cpuImage, dst_);
	destroyImage({cpuImage, cpuAlloc});
}

void Device::upload(VkImage cpuImage, std::shared_ptr<Render::Texture> const& dst_)
{
	auto encoder = std::static_pointer_cast<Encoder>(dmaEncoderPool->allocateEncoder());
	Texture* dst = dst_->getStage<Texture>(Texture::s_stage);

	encoder->begin();
	VkImageMemoryBarrier hostBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	hostBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	hostBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	hostBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	hostBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	hostBarrier.image = cpuImage;
	hostBarrier.subresourceRange.aspectMask = dst->entireRange.aspectMask;
	hostBarrier.subresourceRange.baseMipLevel = dst->entireRange.baseMipLevel;
	hostBarrier.subresourceRange.levelCount = dst->entireRange.levelCount;
	hostBarrier.subresourceRange.baseArrayLayer = dst->entireRange.baseArrayLayer;
	hostBarrier.subresourceRange.layerCount = dst->entireRange.layerCount;
	hostBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	hostBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	encoder->textureBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, hostBarrier);
	VkImageSubresourceLayers srcLayers;
	srcLayers.aspectMask = dst->entireRange.aspectMask;
	srcLayers.baseArrayLayer = dst->entireRange.baseArrayLayer;
	srcLayers.layerCount = dst->entireRange.layerCount;
	srcLayers.mipLevel = dst->entireRange.levelCount;
	encoder->copy(cpuImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcLayers, dst_);

	VkImageMemoryBarrier copyBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	copyBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	copyBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	copyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	copyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	copyBarrier.image = dst->image;
	copyBarrier.subresourceRange.aspectMask = dst->entireRange.aspectMask;
	copyBarrier.subresourceRange.baseMipLevel = dst->entireRange.baseMipLevel;
	copyBarrier.subresourceRange.levelCount = dst->entireRange.levelCount;
	copyBarrier.subresourceRange.baseArrayLayer = dst->entireRange.baseArrayLayer;
	copyBarrier.subresourceRange.layerCount = dst->entireRange.layerCount;
	copyBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	copyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	encoder->textureBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, copyBarrier);
	dst->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	encoder->end();

	getDMASpecificQueue()->enqueue(encoder);
	getDMASpecificQueue()->submit();
	getDMASpecificQueue()->stallTillIdle();
}

auto Device::destroyQueue(VkQueue const& queue_) -> void
{
	// queue aren't actually destroyed at the moment
	// so delibrate no-op currently
}

auto Device::createSemaphore(VkSemaphoreCreateInfo const& createInfo_) -> VkSemaphore
{

	VkSemaphore semaphore;
	CHKED(vkCreateSemaphore(&createInfo_, &allocationCallbacks, &semaphore));
	return semaphore;
}

auto Device::destroySemaphore(VkSemaphore semaphore_) -> void
{
	vkDestroySemaphore(semaphore_, &allocationCallbacks);
}

auto Device::createFence(VkFenceCreateInfo const& createInfo_) -> VkFence
{
	VkFence fence;
	CHKED(vkCreateFence(&createInfo_, &allocationCallbacks, &fence));
	return fence;
}

auto Device::destroyFence(VkFence semaphore_) -> void
{
	vkDestroyFence(semaphore_, &allocationCallbacks);
}


auto Device::createImage(VkImageCreateInfo const& createInfo_, VmaAllocationCreateInfo const& allocInfo_,
						 VmaAllocationInfo& outInfo_) -> std::pair<VkImage, VmaAllocation>
{
	std::pair<VkImage, VmaAllocation> result{VK_NULL_HANDLE, VK_NULL_HANDLE};
	CHKED(vmaCreateImage(allocator, &createInfo_, &allocInfo_, &result.first, &result.second, &outInfo_));

	return result;
}

auto Device::destroyImage(std::pair<VkImage, VmaAllocation> const& image_) -> void
{
	vmaDestroyImage(allocator, image_.first, image_.second);
}

auto Device::createImageView(VkImageViewCreateInfo const& createInfo_) -> VkImageView
{
	VkImageView imageView;
	CHKED(vkCreateImageView(&createInfo_, &allocationCallbacks, &imageView));

	return imageView;
}

auto Device::destroyImageView(VkImageView const& image_) -> void
{
	vkDestroyImageView(image_, &allocationCallbacks);
}

auto Device::createSwapchain(VkSwapchainCreateInfoKHR const& createInfo_) -> VkSwapchainKHR
{
	VkSwapchainKHR swapchainKHR;
	CHKED(vkCreateSwapchainKHR(&createInfo_, &allocationCallbacks, &swapchainKHR));
	return swapchainKHR;
}

auto Device::destroySwapchain(VkSwapchainKHR swapchain_) -> void
{
	vkDestroySwapchainKHR(swapchain_, &allocationCallbacks);
}

auto Device::createRenderPass(VkRenderPassCreateInfo const& createInfo_) -> VkRenderPass
{
	VkRenderPass renderPass;
	CHKED(vkCreateRenderPass(&createInfo_, &allocationCallbacks, &renderPass));
	return renderPass;
}

auto Device::destroyRenderPass(VkRenderPass renderPass_) -> void
{
	vkDestroyRenderPass(renderPass_, &allocationCallbacks);
}

auto Device::createCommandPool(VkCommandPoolCreateInfo const& createInfo_) -> VkCommandPool
{
	VkCommandPool commandPool;
	CHKED(vkCreateCommandPool(&createInfo_, &allocationCallbacks, &commandPool));
	return commandPool;
}

auto Device::destroyCommandPool(VkCommandPool const& commandPool_) -> void
{
	vkDestroyCommandPool(commandPool_, &allocationCallbacks);
}

auto Device::createFramebuffer(VkFramebufferCreateInfo const& createInfo_) -> VkFramebuffer
{
	VkFramebuffer framebuffer;
	CHKED(vkCreateFramebuffer(&createInfo_, &allocationCallbacks, &framebuffer));
	return framebuffer;
}

auto Device::destroyFramebuffer(VkFramebuffer frameBuffer_) -> void
{
	vkDestroyFramebuffer(frameBuffer_, &allocationCallbacks);
}


auto Device::houseKeepTick() -> void
{
	dmaEncoderPool->reset();
}

auto Device::makeEncoderPool(bool frameLifetime_,
							 Render::CommandQueueFlavour flavour_) -> std::shared_ptr<Render::EncoderPool>
{
	using namespace Render;
	using namespace Core::bitmask;
	assert(renderCapable || Core::bitmask::test_equal(flavour_, CommandQueueFlavour::Render));

	VkCommandPoolCreateInfo createInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	createInfo.flags = 0;
	createInfo.flags |= frameLifetime_ ? VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0;

	if(Core::bitmask::test_equal(flavour_, CommandQueueFlavour::Render | CommandQueueFlavour::Compute))
	{
		createInfo.queueFamilyIndex = allQueue->getFamilyIndex();
	} else if(Core::bitmask::test_any(flavour_, CommandQueueFlavour::Render))
	{
		createInfo.queueFamilyIndex = renderSpecificQueue->getFamilyIndex();
	} else if(Core::bitmask::test_any(flavour_, CommandQueueFlavour::Compute))
	{
		createInfo.queueFamilyIndex = computeSpecificQueue->getFamilyIndex();
	} else
	{
		createInfo.queueFamilyIndex = dmaOnlyQueue->getFamilyIndex();
	}

	VkCommandPool commandPool = createCommandPool(createInfo);

	auto encoderPool = std::make_shared<EncoderPool>(
			this->shared_from_this(),
			commandPool,
			&commandPoolVkVTable,
			&generalCBVkVTable,
			&graphicsCBVkVTable,
			&computeCBVkVTable);
	return encoderPool;
}

auto Device::makeFence() -> std::shared_ptr<Render::Fence>
{
	VkFenceCreateInfo createInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	VkFence vulkanFence = createFence(createInfo);

	Fence::Ptr fence = std::make_shared<Fence>(
			this->shared_from_this(),
			vulkanFence);

	return std::static_pointer_cast<Render::Fence>(fence);

}

auto Device::makeSemaphore() -> std::shared_ptr<Render::Semaphore>
{
	VkSemaphoreCreateInfo createInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	VkSemaphore vulkanSemaphore = createSemaphore(createInfo);

	Semaphore::Ptr semaphore = std::make_shared<Semaphore>(
			this->shared_from_this(),
			vulkanSemaphore);

	return std::static_pointer_cast<Render::Semaphore>(semaphore);
}

auto Device::makeRenderPass(
		std::vector<Render::RenderPass::Target> const& targets_) -> std::shared_ptr<Render::RenderPass>
{
	// convert between RenderPass::Load/Store and vulkans
	static constexpr VkAttachmentLoadOp LoadConvertor[3] =
			{
					VK_ATTACHMENT_LOAD_OP_LOAD,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE
			};
	static constexpr VkAttachmentStoreOp StoreConvertor[3] =
			{
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE
			};

	VkRenderPassCreateInfo createInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};

	std::vector<VkAttachmentDescription> attachments(targets_.size());
	std::vector<VkAttachmentReference> cReferences;
	cReferences.reserve(targets_.size());
	VkAttachmentReference dsReference = {~0u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

	for(auto i = 0u; i < targets_.size(); ++i)
	{
		auto& attach = attachments[i];
		auto const& target = targets_[i];

		attach.flags = 0;
		attach.samples = VK_SAMPLE_COUNT_1_BIT; // TODO
		if(target.load == Render::LoadOp::Load)
		{
			// TODO need to know what the layour was we are loading from?
			attach.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
		} else
		{
			// we don't care, but we can't load from undefined
			attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		attach.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		attach.loadOp = LoadConvertor[(int) target.load];
		attach.storeOp = StoreConvertor[(int) target.store];
		if(Render::GtfCracker::isStencil(target.format))
		{
			attach.stencilLoadOp = LoadConvertor[(int) target.stencilLoad];
			attach.stencilStoreOp = StoreConvertor[(int) target.stencilStore];
		}
		attach.format = VkfCracker::fromGeneric(target.format);

		if(Render::GtfCracker::isDepth(target.format) ||
		   Render::GtfCracker::isStencil(target.format))
		{
			dsReference = {i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
		} else
		{
			cReferences.emplace_back(
					VkAttachmentReference{i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
		}
	}

	// TODO subpasses, for now assume 1 and it matches the entire render target
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = cReferences.size();
	subpass.pColorAttachments = cReferences.data();
	subpass.pDepthStencilAttachment =
			(dsReference.attachment != ~0) ? &dsReference : nullptr;

	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.flags = 0;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.dependencyCount = 0;
	createInfo.pDependencies = nullptr;

	VkRenderPass vulkanRenderPass = createRenderPass(createInfo);

	RenderPass::Ptr renderPass = std::make_shared<RenderPass>();
	return std::static_pointer_cast<Render::RenderPass>(renderPass);

}

auto Device::makeRenderTarget(
		std::shared_ptr<Render::RenderPass> const& pass_,
		std::vector<Render::Texture::Ptr> const& targets_) -> std::shared_ptr<Render::RenderTarget>
{

	assert(targets_.empty() == false);

	std::vector<VkImageView> images(targets_.size());

	uint32_t const width = targets_[0]->width;
	uint32_t const height = targets_[0]->height;
	for(auto i = 0u; i < targets_.size(); ++i)
	{
		auto& rtarget = targets_[i];
		assert(rtarget->width == width);
		assert(rtarget->height == height);

		auto target = rtarget->getStage<Texture>(Texture::s_stage);
		images[i] = target->imageView;
		if(Render::GtfCracker::isDepth(rtarget->format) ||
		   Render::GtfCracker::isStencil(rtarget->format))
		{
			target->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		} else
		{
			target->imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}
	auto renderPass = std::static_pointer_cast<Vulkan::RenderPass>(pass_);

	VkFramebufferCreateInfo createInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	createInfo.flags = 0;
	createInfo.renderPass = renderPass->renderpass;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.attachmentCount = images.size();
	createInfo.pAttachments = images.data();
	createInfo.layers = 0;
	VkFramebuffer framebuffer = createFramebuffer(createInfo);

	RenderTarget::Ptr renderTargets = std::make_shared<RenderTarget>(
			this->shared_from_this());

	return std::static_pointer_cast<Render::RenderTarget>(renderTargets);

}

auto Device::getGeneralQueue() -> Render::CommandQueue::Ptr
{
	return allQueue ? allQueue : computeSpecificQueue;
}

auto Device::getRenderSpecificQueue() -> Render::CommandQueue::Ptr
{
	return renderSpecificQueue;
}

auto Device::getComputeSpecificQueue() -> Render::CommandQueue::Ptr
{
	return computeSpecificQueue;
}

auto Device::getDMASpecificQueue() -> Render::CommandQueue::Ptr
{
	return dmaOnlyQueue;
}

auto Device::getPresentQueue() -> Render::CommandQueue::Ptr
{
	return presentQueue.lock();
}

}
