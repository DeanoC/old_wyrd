#ifndef GLOBAL_VK_FUNC
#define GLOBAL_VK_FUNC( function )
#endif

#ifndef INSTANCE_VK_FUNC
#define INSTANCE_VK_FUNC( function )
#endif


#ifndef INSTANCE_VK_FUNC_EXT
#define INSTANCE_VK_FUNC_EXT( function, extension )
#endif


#ifndef DEVICE_VK_FUNC
#define DEVICE_VK_FUNC( function )
#endif

#ifndef DEVICE_VK_FUNC_EXT
#define DEVICE_VK_FUNC_EXT( function, extension )
#endif

#ifndef FENCE_VK_FUNC
#define FENCE_VK_FUNC(function)
#endif

#ifndef FENCE_VK_FUNC_EXT
#define FENCE_VK_FUNC_EXT(function, extension)
#endif

#ifndef SEMAPHORE_VK_FUNC
#define SEMAPHORE_VK_FUNC(function)
#endif

#ifndef SEMAPHORE_VK_FUNC_EXT
#define SEMAPHORE_VK_FUNC_EXT(function, extension)
#endif

#ifndef EVENT_VK_FUNC
#define EVENT_VK_FUNC(function)
#endif

#ifndef EVENT_VK_FUNC_EXT
#define EVENT_VK_FUNC_EXT(function, extension)
#endif

#ifndef QUEUE_VK_FUNC
#define QUEUE_VK_FUNC(function)
#endif

#ifndef QUEUE_VK_FUNC_EXT
#define QUEUE_VK_FUNC_EXT(function, extension)
#endif

#ifndef QUERYPOOL_VK_FUNC
#define QUERYPOOL_VK_FUNC(function)
#endif

#ifndef QUERYPOOL_VK_FUNC_EXT
#define QUERYPOOL_VK_FUNC_EXT(function, extension)
#endif

#ifndef BUFFER_VK_FUNC
#define BUFFER_VK_FUNC(function)
#endif

#ifndef BUFFER_VK_FUNC_EXT
#define BUFFER_VK_FUNC_EXT(function, extension)
#endif

#ifndef BUFFERVIEW_VK_FUNC
#define BUFFERVIEW_VK_FUNC(function)
#endif

#ifndef BUFFERVIEW_VK_FUNC_EXT
#define BUFFERVIEW_VK_FUNC_EXT(function, extension)
#endif

#ifndef IMAGE_VK_FUNC
#define IMAGE_VK_FUNC(function)
#endif

#ifndef IMAGE_VK_FUNC_EXT
#define IMAGE_VK_FUNC_EXT(function, extension)
#endif

#ifndef IMAGEVIEW_VK_FUNC
#define IMAGEVIEW_VK_FUNC(function)
#endif

#ifndef IMAGEVIEW_VK_FUNC_EXT
#define IMAGEVIEW_VK_FUNC_EXT(function, extension)
#endif

#ifndef SHADERMODULE_VK_FUNC
#define SHADERMODULE_VK_FUNC(function)
#endif

#ifndef SHADERMODULE_VK_FUNC_EXT
#define SHADERMODULE_VK_FUNC_EXT(function, extension)
#endif

#ifndef PIPELINE_VK_FUNC
#define PIPELINE_VK_FUNC(function)
#endif

#ifndef PIPELINE_VK_FUNC_EXT
#define PIPELINE_VK_FUNC_EXT(function, extension)
#endif

#ifndef SAMPLER_VK_FUNC
#define SAMPLER_VK_FUNC(function)
#endif

#ifndef SAMPLER_VK_FUNC_EXT
#define SAMPLER_VK_FUNC_EXT(function, extension)
#endif

#ifndef DESCRIPTORSET_VK_FUNC
#define DESCRIPTORSET_VK_FUNC(function)
#endif

#ifndef DESCRIPTORSET_VK_FUNC_EXT
#define DESCRIPTORSET_VK_FUNC_EXT(function, extension)
#endif

#ifndef FRAMEBUFFER_VK_FUNC
#define FRAMEBUFFER_VK_FUNC(function)
#endif

#ifndef FRAMEBUFFER_VK_FUNC_EXT
#define FRAMEBUFFER_VK_FUNC_EXT(function, extension)
#endif

#ifndef RENDERPASS_VK_FUNC
#define RENDERPASS_VK_FUNC(function)
#endif

#ifndef RENDERPASS_VK_FUNC_EXT
#define RENDERPASS_VK_FUNC_EXT(function, extension)
#endif

#ifndef COMMANDPOOL_VK_FUNC
#define COMMANDPOOL_VK_FUNC(function)
#endif

#ifndef COMMANDPOOL_VK_FUNC_EXT
#define COMMANDPOOL_VK_FUNC_EXT(function, extension)
#endif

#ifndef GRAPHICS_CB_VK_FUNC
#define GRAPHICS_CB_VK_FUNC(function)
#endif

#ifndef GRAPHICS_CB_VK_FUNC_EXT
#define GRAPHICS_CB_VK_FUNC_EXT( function, extension )
#endif

#ifndef COMPUTE_CB_VK_FUNC
#define COMPUTE_CB_VK_FUNC(function)
#endif

#ifndef COMPUTE_CB_VK_FUNC_EXT
#define COMPUTE_CB_VK_FUNC_EXT( functionm, extension )
#endif

#ifndef TRANSFER_CB_VK_FUNC
#define TRANSFER_CB_VK_FUNC(function)
#endif

#ifndef TRANSFER_CB_VK_FUNC_EXT
#define TRANSFER_CB_VK_FUNC_EXT( function, extension )
#endif

#ifndef GENERAL_CB_VK_FUNC
#define GENERAL_CB_VK_FUNC(function) \
            GRAPHICS_CB_VK_FUNC(function) \
            COMPUTE_CB_VK_FUNC(function) \
            TRANSFER_CB_VK_FUNC(function)
#endif

#ifndef GENERAL_CB_VK_FUNC_EXT
#define GENERAL_CB_VK_FUNC_EXT(function, extension) \
            GRAPHICS_CB_VK_FUNC_EXT(function) \
            COMPUTE_CB_VK_FUNC_EXT(function) \
            TRANSFER_CB_VK_FUNC_EXT(function)
#endif


#ifndef GC_CB_VK_FUNC
#define GC_CB_VK_FUNC(function) \
            GRAPHICS_CB_VK_FUNC(function) \
            COMPUTE_CB_VK_FUNC(function)
#endif

#ifndef GC_CB_VK_FUNC_EXT
#define GC_CB_VK_FUNC_EXT(function, extension) \
            GRAPHICS_CB_VK_FUNC(function, extension) \
            COMPUTE_CB_VK_FUNC(function, extension)
#endif

GLOBAL_VK_FUNC(vkEnumerateInstanceExtensionProperties)
GLOBAL_VK_FUNC(vkEnumerateInstanceLayerProperties)
GLOBAL_VK_FUNC(vkCreateInstance)
GLOBAL_VK_FUNC(vkDestroyInstance)

INSTANCE_VK_FUNC(vkEnumeratePhysicalDevices)
INSTANCE_VK_FUNC(vkGetDeviceProcAddr)
INSTANCE_VK_FUNC(vkCreateDevice)
INSTANCE_VK_FUNC(vkDestroyDevice)
INSTANCE_VK_FUNC(vkGetImageMemoryRequirements)

INSTANCE_VK_FUNC(vkEnumerateDeviceExtensionProperties)
INSTANCE_VK_FUNC(vkGetPhysicalDeviceMemoryProperties)
INSTANCE_VK_FUNC(vkGetPhysicalDeviceFormatProperties)
INSTANCE_VK_FUNC(vkGetPhysicalDeviceImageFormatProperties)
INSTANCE_VK_FUNC(vkGetPhysicalDeviceProperties)
INSTANCE_VK_FUNC(vkGetPhysicalDeviceFeatures)
INSTANCE_VK_FUNC(vkGetPhysicalDeviceQueueFamilyProperties)
INSTANCE_VK_FUNC(vkGetPhysicalDeviceSparseImageFormatProperties)

INSTANCE_VK_FUNC_EXT(vkGetPhysicalDeviceSurfaceSupportKHR, VK_KHR_SURFACE_EXTENSION_NAME)
INSTANCE_VK_FUNC_EXT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, VK_KHR_SURFACE_EXTENSION_NAME)
INSTANCE_VK_FUNC_EXT(vkGetPhysicalDeviceSurfaceFormatsKHR, VK_KHR_SURFACE_EXTENSION_NAME)
INSTANCE_VK_FUNC_EXT(vkGetPhysicalDeviceSurfacePresentModesKHR, VK_KHR_SURFACE_EXTENSION_NAME)

#ifdef VK_USE_PLATFORM_WIN32_KHR
INSTANCE_VK_FUNC_EXT( vkCreateWin32SurfaceKHR, VK_KHR_WIN32_SURFACE_EXTENSION_NAME )
#elif defined VK_USE_PLATFORM_XCB_KHR
INSTANCE_VK_FUNC_EXT( vkCreateXcbSurfaceKHR, VK_KHR_XLIB_SURFACE_EXTENSION_NAME )
#elif defined VK_USE_PLATFORM_XLIB_KHR
INSTANCE_VK_FUNC_EXT( vkCreateXlibSurfaceKHR, VK_KHR_XCB_SURFACE_EXTENSION_NAME )
#elif defined VK_USE_PLATFORM_MACOS_MVK
INSTANCE_VK_FUNC_EXT( vkCreateMacOSSurfaceMVK, VK_MVK_MACOS_SURFACE_EXTENSION_NAME )
#endif

DEVICE_VK_FUNC(vkGetDeviceQueue)
DEVICE_VK_FUNC(vkDeviceWaitIdle)

DEVICE_VK_FUNC(vkAllocateMemory)
DEVICE_VK_FUNC(vkFreeMemory)
DEVICE_VK_FUNC(vkMapMemory)
DEVICE_VK_FUNC(vkUnmapMemory)
DEVICE_VK_FUNC(vkFlushMappedMemoryRanges)
DEVICE_VK_FUNC(vkInvalidateMappedMemoryRanges)
DEVICE_VK_FUNC(vkGetDeviceMemoryCommitment)
DEVICE_VK_FUNC(vkBindBufferMemory)
DEVICE_VK_FUNC(vkBindImageMemory)
DEVICE_VK_FUNC(vkGetBufferMemoryRequirements)
DEVICE_VK_FUNC(vkGetImageMemoryRequirements)
DEVICE_VK_FUNC(vkGetImageSparseMemoryRequirements)

DEVICE_VK_FUNC(vkCreateFence)
FENCE_VK_FUNC(vkDestroyFence)
FENCE_VK_FUNC(vkResetFences)
FENCE_VK_FUNC(vkGetFenceStatus)
FENCE_VK_FUNC(vkWaitForFences)

DEVICE_VK_FUNC(vkCreateSemaphore)
SEMAPHORE_VK_FUNC(vkDestroySemaphore)

DEVICE_VK_FUNC(vkCreateEvent)
EVENT_VK_FUNC(vkDestroyEvent)
EVENT_VK_FUNC(vkGetEventStatus)
EVENT_VK_FUNC(vkSetEvent)
EVENT_VK_FUNC(vkResetEvent)

DEVICE_VK_FUNC(vkCreateQueryPool)
QUERYPOOL_VK_FUNC(vkDestroyQueryPool)
QUERYPOOL_VK_FUNC(vkGetQueryPoolResults)

DEVICE_VK_FUNC(vkCreateBuffer)
BUFFER_VK_FUNC(vkDestroyBuffer)

DEVICE_VK_FUNC(vkCreateBufferView)
BUFFERVIEW_VK_FUNC(vkDestroyBufferView)

DEVICE_VK_FUNC(vkCreateImage)
IMAGE_VK_FUNC(vkDestroyImage)
IMAGE_VK_FUNC(vkGetImageSubresourceLayout)

DEVICE_VK_FUNC(vkCreateImageView)
IMAGEVIEW_VK_FUNC(vkDestroyImageView)

DEVICE_VK_FUNC(vkCreateShaderModule)
SHADERMODULE_VK_FUNC(vkDestroyShaderModule)

DEVICE_VK_FUNC(vkCreatePipelineCache)
PIPELINE_VK_FUNC(vkDestroyPipelineCache)
PIPELINE_VK_FUNC(vkGetPipelineCacheData)
PIPELINE_VK_FUNC(vkMergePipelineCaches)
DEVICE_VK_FUNC(vkCreateGraphicsPipelines)
DEVICE_VK_FUNC(vkCreateComputePipelines)
PIPELINE_VK_FUNC(vkDestroyPipeline)
PIPELINE_VK_FUNC(vkCreatePipelineLayout)
PIPELINE_VK_FUNC(vkDestroyPipelineLayout)

DEVICE_VK_FUNC(vkCreateSampler)
SAMPLER_VK_FUNC(vkDestroySampler)

DESCRIPTORSET_VK_FUNC(vkCreateDescriptorSetLayout)
DESCRIPTORSET_VK_FUNC(vkDestroyDescriptorSetLayout)
DESCRIPTORSET_VK_FUNC(vkCreateDescriptorPool)
DESCRIPTORSET_VK_FUNC(vkDestroyDescriptorPool)
DESCRIPTORSET_VK_FUNC(vkResetDescriptorPool)
DESCRIPTORSET_VK_FUNC(vkAllocateDescriptorSets)
DESCRIPTORSET_VK_FUNC(vkFreeDescriptorSets)
DESCRIPTORSET_VK_FUNC(vkUpdateDescriptorSets)

DEVICE_VK_FUNC(vkCreateFramebuffer)
FRAMEBUFFER_VK_FUNC(vkDestroyFramebuffer)

DEVICE_VK_FUNC(vkCreateRenderPass)
RENDERPASS_VK_FUNC(vkDestroyRenderPass)
RENDERPASS_VK_FUNC(vkGetRenderAreaGranularity)

DEVICE_VK_FUNC(vkCreateCommandPool)
COMMANDPOOL_VK_FUNC(vkDestroyCommandPool)
COMMANDPOOL_VK_FUNC(vkResetCommandPool)
COMMANDPOOL_VK_FUNC(vkAllocateCommandBuffers)
COMMANDPOOL_VK_FUNC(vkFreeCommandBuffers)

DEVICE_VK_FUNC_EXT(vkCreateSwapchainKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
DEVICE_VK_FUNC_EXT(vkGetSwapchainImagesKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
DEVICE_VK_FUNC_EXT(vkAcquireNextImageKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
DEVICE_VK_FUNC_EXT(vkDestroySwapchainKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)

QUEUE_VK_FUNC(vkQueueSubmit)
QUEUE_VK_FUNC(vkQueueWaitIdle)
QUEUE_VK_FUNC(vkQueueBindSparse)
QUEUE_VK_FUNC_EXT(vkQueuePresentKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)

// these functions must be placed in the vtable before the
// type specific vtable function, which happens if place here
GENERAL_CB_VK_FUNC(vkBeginCommandBuffer)
GENERAL_CB_VK_FUNC(vkEndCommandBuffer)
GENERAL_CB_VK_FUNC(vkResetCommandBuffer)
GENERAL_CB_VK_FUNC(vkCmdCopyBuffer)
GENERAL_CB_VK_FUNC(vkCmdCopyImage)
GENERAL_CB_VK_FUNC(vkCmdBlitImage)
GENERAL_CB_VK_FUNC(vkCmdCopyBufferToImage)
GENERAL_CB_VK_FUNC(vkCmdCopyImageToBuffer)
GENERAL_CB_VK_FUNC(vkCmdUpdateBuffer)
GENERAL_CB_VK_FUNC(vkCmdFillBuffer)
GENERAL_CB_VK_FUNC(vkCmdClearColorImage)
GENERAL_CB_VK_FUNC(vkCmdClearDepthStencilImage)
GENERAL_CB_VK_FUNC(vkCmdClearAttachments)
GENERAL_CB_VK_FUNC(vkCmdResolveImage)
GENERAL_CB_VK_FUNC(vkCmdSetEvent)
GENERAL_CB_VK_FUNC(vkCmdResetEvent)
GENERAL_CB_VK_FUNC(vkCmdWaitEvents)
GENERAL_CB_VK_FUNC(vkCmdPipelineBarrier)
GENERAL_CB_VK_FUNC(vkCmdBeginQuery)
GENERAL_CB_VK_FUNC(vkCmdEndQuery)
GENERAL_CB_VK_FUNC(vkCmdResetQueryPool)
GENERAL_CB_VK_FUNC(vkCmdWriteTimestamp)
GENERAL_CB_VK_FUNC(vkCmdCopyQueryPoolResults)
GENERAL_CB_VK_FUNC(vkCmdPushConstants)
GENERAL_CB_VK_FUNC(vkCmdBeginRenderPass)
GENERAL_CB_VK_FUNC(vkCmdNextSubpass)
GENERAL_CB_VK_FUNC(vkCmdEndRenderPass)
GENERAL_CB_VK_FUNC(vkCmdExecuteCommands)

GC_CB_VK_FUNC(vkCmdBindPipeline)
GC_CB_VK_FUNC(vkCmdBindDescriptorSets)

GRAPHICS_CB_VK_FUNC(vkCmdSetViewport)
GRAPHICS_CB_VK_FUNC(vkCmdSetScissor)
GRAPHICS_CB_VK_FUNC(vkCmdSetLineWidth)
GRAPHICS_CB_VK_FUNC(vkCmdSetDepthBias)
GRAPHICS_CB_VK_FUNC(vkCmdSetBlendConstants)
GRAPHICS_CB_VK_FUNC(vkCmdSetDepthBounds)
GRAPHICS_CB_VK_FUNC(vkCmdSetStencilCompareMask)
GRAPHICS_CB_VK_FUNC(vkCmdSetStencilWriteMask)
GRAPHICS_CB_VK_FUNC(vkCmdSetStencilReference)
GRAPHICS_CB_VK_FUNC(vkCmdBindIndexBuffer)
GRAPHICS_CB_VK_FUNC(vkCmdBindVertexBuffers)
GRAPHICS_CB_VK_FUNC(vkCmdDraw)
GRAPHICS_CB_VK_FUNC(vkCmdDrawIndexed)
GRAPHICS_CB_VK_FUNC(vkCmdDrawIndirect)
GRAPHICS_CB_VK_FUNC(vkCmdDrawIndexedIndirect)

COMPUTE_CB_VK_FUNC(vkCmdDispatch)
COMPUTE_CB_VK_FUNC(vkCmdDispatchIndirect)


#undef GLOBAL_VK_FUNC
#undef INSTANCE_VK_FUNC
#undef INSTANCE_VK_FUNC_EXT

#undef DEVICE_VK_FUNC
#undef DEVICE_VK_FUNC_EXT
#undef FENCE_VK_FUNC
#undef FENCE_VK_FUNC_EXT
#undef SEMAPHORE_VK_FUNC
#undef SEMAPHORE_VK_FUNC_EXT
#undef EVENT_VK_FUNC
#undef EVENT_VK_FUNC_EXT
#undef QUERYPOOL_VK_FUNC
#undef QUERYPOOL_VK_FUNC_EXT
#undef BUFFER_VK_FUNC
#undef BUFFER_VK_FUNC_EXT
#undef BUFFERVIEW_VK_FUNC
#undef BUFFERVIEW_VK_FUNC_EXT
#undef IMAGE_VK_FUNC
#undef IMAGE_VK_FUNC_EXT
#undef IMAGEVIEW_VK_FUNC
#undef IMAGEVIEW_VK_FUNC_EXT
#undef SHADERMODULE_VK_FUNC
#undef SHADERMODULE_VK_FUNC_EXT
#undef PIPELINE_VK_FUNC
#undef PIPELINE_VK_FUNC_EXT
#undef SAMPLER_VK_FUNC
#undef SAMPLER_VK_FUNC_EXT
#undef DESCRIPTORSET_VK_FUNC
#undef DESCRIPTORSET_VK_FUNC_EXT
#undef FRAMEBUFFER_VK_FUNC
#undef FRAMEBUFFER_VK_FUNC_EXT
#undef RENDERPASS_VK_FUNC
#undef RENDERPASS_VK_FUNC_EXT
#undef COMMANDPOOL_VK_FUNC
#undef COMMANDPOOL_VK_FUNC_EXT

#undef QUEUE_VK_FUNC
#undef QUEUE_VK_FUNC_EXT

#undef GRAPHICS_CB_VK_FUNC
#undef COMPUTE_CB_VK_FUNC
#undef TRANSFER_CB_VK_FUNC

#undef GRAPHICS_CB_VK_FUNC_EXT
#undef COMPUTE_CB_VK_FUNC_EXT
#undef TRANSFER_CB_VK_FUNC_EXT

#undef GENERAL_CB_VK_FUNC
#undef GC_CB_VK_FUNC
#undef GENERAL_CB_VK_FUNC_EXT
#undef GC_CB_VK_FUNC_EXT
