#pragma once
#ifndef WYRD_VULKAN_FUNCTIONS_H
#define WYRD_VULKAN_FUNCTIONS_H

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include "vulkan/vk_mem_alloc.h"
#include <string_view>

namespace Vulkan {
#define GLOBAL_VK_FUNC( name ) extern PFN_##name name;
#define INSTANCE_VK_FUNC( name ) extern PFN_##name name;
#define INSTANCE_VK_FUNC_EXT( name, extension ) extern PFN_##name name;
//#define DEVICE_VK_FUNC( name ) extern PFN_##name name;
//#define DEVICE_VK_FUNC_EXT( name, extension ) extern PFN_##name name;
#include "functionlist.inl"
std::string_view const getVulkanResultString(VkResult result);
} // namespace Vulkan

#define CHKED(x) if(auto r = (x); r != VK_SUCCESS) { LOG_S(ERROR) << "Vulkan Error: " << #x << " " << getVulkanResultString(r); }

#endif //WYRD_VULKAN_FUNCTIONS_H
