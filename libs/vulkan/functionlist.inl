#ifndef GLOBAL_VK_FUNC
#define GLOBAL_VK_FUNC( function )
#endif

#ifndef INSTANCE_VK_FUNC
#define INSTANCE_VK_FUNC( function )
#endif
//
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

GLOBAL_VK_FUNC(vkEnumerateInstanceExtensionProperties)
GLOBAL_VK_FUNC(vkEnumerateInstanceLayerProperties)
GLOBAL_VK_FUNC(vkCreateInstance)
GLOBAL_VK_FUNC(vkDestroyInstance)

INSTANCE_VK_FUNC(vkEnumeratePhysicalDevices)
INSTANCE_VK_FUNC(vkCreateDevice)
INSTANCE_VK_FUNC(vkGetDeviceProcAddr)
INSTANCE_VK_FUNC(vkDestroyDevice)
INSTANCE_VK_FUNC(vkEnumerateDeviceExtensionProperties)
INSTANCE_VK_FUNC(vkGetPhysicalDeviceProperties)
INSTANCE_VK_FUNC(vkGetPhysicalDeviceFeatures)
INSTANCE_VK_FUNC(vkGetPhysicalDeviceQueueFamilyProperties)
INSTANCE_VK_FUNC(vkGetImageMemoryRequirements)
INSTANCE_VK_FUNC(vkGetPhysicalDeviceMemoryProperties)

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

DEVICE_VK_FUNC(vkGetDeviceQueue);

DEVICE_VK_FUNC(vkAllocateMemory);
DEVICE_VK_FUNC(vkBindBufferMemory);
DEVICE_VK_FUNC(vkBindImageMemory);

DEVICE_VK_FUNC(vkCreateBuffer);
DEVICE_VK_FUNC(vkDestroyBuffer);

DEVICE_VK_FUNC(vkCreateImage);
DEVICE_VK_FUNC(vkDestroyImage);

DEVICE_VK_FUNC(vkCreateSemaphore);
DEVICE_VK_FUNC(vkDestroySemaphore);


DEVICE_VK_FUNC(vkFlushMappedMemoryRanges);
DEVICE_VK_FUNC(vkFreeMemory);
DEVICE_VK_FUNC(vkGetBufferMemoryRequirements);
DEVICE_VK_FUNC(vkInvalidateMappedMemoryRanges)
DEVICE_VK_FUNC(vkMapMemory)
DEVICE_VK_FUNC(vkUnmapMemory)
DEVICE_VK_FUNC(vkGetImageMemoryRequirements)


DEVICE_VK_FUNC_EXT( vkCreateSwapchainKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME )
DEVICE_VK_FUNC_EXT( vkGetSwapchainImagesKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME )
DEVICE_VK_FUNC_EXT( vkAcquireNextImageKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME )
DEVICE_VK_FUNC_EXT( vkQueuePresentKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME )
DEVICE_VK_FUNC_EXT( vkDestroySwapchainKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME )


#undef GLOBAL_VK_FUNC
#undef INSTANCE_VK_FUNC
#undef INSTANCE_VK_FUNC_EXT
#undef DEVICE_VK_FUNC
#undef DEVICE_VK_FUNC_EXT