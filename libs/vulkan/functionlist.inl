#ifndef GLOBAL_VK_FUNC
#define GLOBAL_VK_FUNC( function )
#endif

#ifndef INSTANCE_VK_FUNC
#define INSTANCE_VK_FUNC( function )
#endif


#ifndef INSTANCE_VK_FUNC_EXT
#define INSTANCE_VK_FUNC_EXT( function, extension )
#endif
//
#ifndef DEVICE_VK_FUNC
#define DEVICE_VK_FUNC( function )
#endif

#ifndef DEVICE_VK_FUNC_EXT
#define DEVICE_VK_FUNC_EXT( function, extension )
#endif

#ifndef QUEUE_VK_FUNC
#define QUEUE_VK_FUNC(function)
#endif

#ifndef QUEUE_VK_FUNC_EXT
#define QUEUE_VK_FUNC_EXT(function, extension)
#endif

#ifndef GRAPHICS_CB_VK_FUNC
#define GRAPHICS_CB_VK_FUNC(function)
#endif

#ifndef GRAPHICS_CB_VK_FUNC
#define GRAPHICS_CB_VK_FUNC_EXT( function, extension )
#endif

#ifndef COMPUTE_CB_VK_FUNC
#define COMPUTE_CB_VK_FUNC(function)
#endif

#ifndef COMPUTE_CB_VK_FUNC
#define COMPUTE_CB_VK_FUNC_EXT( functionm, extension )
#endif

#ifndef TRANSFER_CB_VK_FUNC
#define TRANSFER_CB_VK_FUNC(function)
#endif

#ifndef TRANSFER_CB_VK_FUNC
#define TRANSFER_CB_VK_FUNC_EXT( function, extension )
#endif

#define GC_CB_VK_FUNC(function) \
            GRAPHICS_CB_VK_FUNC(function) \
            COMPUTE_CB_VK_FUNC(function)

#define GENERAL_CB_VK_FUNC(function) \
            GRAPHICS_CB_VK_FUNC(function) \
            COMPUTE_CB_VK_FUNC(function) \
            TRANSFER_CB_VK_FUNC(function)

#define GC_CB_VK_FUNC_EXT(function, extension) \
            GRAPHICS_CB_VK_FUNC(function, extension) \
            COMPUTE_CB_VK_FUNC(function, extension)

#define GENERAL_CB_VK_FUNC_EXT(function, extension) \
            GRAPHICS_CB_VK_FUNC(function, extension) \
            COMPUTE_CB_VK_FUNC(function, extension) \
            TRANSFER_CB_VK_FUNC(function, extension)

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
DEVICE_VK_FUNC(vkDestroyFence)
DEVICE_VK_FUNC(vkResetFences)
DEVICE_VK_FUNC(vkGetFenceStatus)
DEVICE_VK_FUNC(vkWaitForFences)
DEVICE_VK_FUNC(vkCreateSemaphore)
DEVICE_VK_FUNC(vkDestroySemaphore)
DEVICE_VK_FUNC(vkCreateEvent)
DEVICE_VK_FUNC(vkDestroyEvent)
DEVICE_VK_FUNC(vkGetEventStatus)
DEVICE_VK_FUNC(vkSetEvent)
DEVICE_VK_FUNC(vkResetEvent)
DEVICE_VK_FUNC(vkCreateQueryPool)
DEVICE_VK_FUNC(vkDestroyQueryPool)
DEVICE_VK_FUNC(vkGetQueryPoolResults)
DEVICE_VK_FUNC(vkCreateBuffer)
DEVICE_VK_FUNC(vkDestroyBuffer)
DEVICE_VK_FUNC(vkCreateBufferView)
DEVICE_VK_FUNC(vkDestroyBufferView)
DEVICE_VK_FUNC(vkCreateImage)
DEVICE_VK_FUNC(vkDestroyImage)
DEVICE_VK_FUNC(vkGetImageSubresourceLayout)
DEVICE_VK_FUNC(vkCreateImageView)
DEVICE_VK_FUNC(vkDestroyImageView)
DEVICE_VK_FUNC(vkCreateShaderModule)
DEVICE_VK_FUNC(vkDestroyShaderModule)
DEVICE_VK_FUNC(vkCreatePipelineCache)
DEVICE_VK_FUNC(vkDestroyPipelineCache)
DEVICE_VK_FUNC(vkGetPipelineCacheData)
DEVICE_VK_FUNC(vkMergePipelineCaches)
DEVICE_VK_FUNC(vkCreateGraphicsPipelines)
DEVICE_VK_FUNC(vkCreateComputePipelines)
DEVICE_VK_FUNC(vkDestroyPipeline)
DEVICE_VK_FUNC(vkCreatePipelineLayout)
DEVICE_VK_FUNC(vkDestroyPipelineLayout)
DEVICE_VK_FUNC(vkCreateSampler)
DEVICE_VK_FUNC(vkDestroySampler)
DEVICE_VK_FUNC(vkCreateDescriptorSetLayout)
DEVICE_VK_FUNC(vkDestroyDescriptorSetLayout)
DEVICE_VK_FUNC(vkCreateDescriptorPool)
DEVICE_VK_FUNC(vkDestroyDescriptorPool)
DEVICE_VK_FUNC(vkResetDescriptorPool)
DEVICE_VK_FUNC(vkAllocateDescriptorSets)
DEVICE_VK_FUNC(vkFreeDescriptorSets)
DEVICE_VK_FUNC(vkUpdateDescriptorSets)
DEVICE_VK_FUNC(vkCreateFramebuffer)
DEVICE_VK_FUNC(vkDestroyFramebuffer)
DEVICE_VK_FUNC(vkCreateRenderPass)
DEVICE_VK_FUNC(vkDestroyRenderPass)
DEVICE_VK_FUNC(vkGetRenderAreaGranularity)
DEVICE_VK_FUNC(vkCreateCommandPool)
DEVICE_VK_FUNC(vkDestroyCommandPool)
DEVICE_VK_FUNC(vkResetCommandPool)
DEVICE_VK_FUNC(vkAllocateCommandBuffers)
DEVICE_VK_FUNC(vkFreeCommandBuffers)

DEVICE_VK_FUNC_EXT(vkCreateSwapchainKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
DEVICE_VK_FUNC_EXT(vkGetSwapchainImagesKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
DEVICE_VK_FUNC_EXT(vkAcquireNextImageKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
DEVICE_VK_FUNC_EXT(vkDestroySwapchainKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)

QUEUE_VK_FUNC(vkQueueSubmit)
QUEUE_VK_FUNC(vkQueueWaitIdle)
QUEUE_VK_FUNC(vkQueueBindSparse)
QUEUE_VK_FUNC_EXT(vkQueuePresentKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)

GENERAL_CB_VK_FUNC(vkBeginCommandBuffer)
GENERAL_CB_VK_FUNC(vkEndCommandBuffer)
GENERAL_CB_VK_FUNC(vkResetCommandBuffer)

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

#undef GLOBAL_VK_FUNC
#undef INSTANCE_VK_FUNC
#undef INSTANCE_VK_FUNC_EXT

#undef DEVICE_VK_FUNC
#undef DEVICE_VK_FUNC_EXT

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
