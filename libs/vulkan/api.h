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

// some evil magic/include madness to simulate c++ vtables from a
// dynamically bound shared library/dll
#define DEVICE_VK_FUNC(name) PFN_##name name;
#define DEVICE_VK_FUNC_EXT(name, extension)PFN_##name name;
struct DeviceVkVTable
{
#include "functionlist.inl"
};

#define QUEUE_VK_FUNC(name) PFN_##name name;
#define QUEUE_VK_FUNC_EXT(name, extension) PFN_##name name;
struct QueueVkVTable
{
#include "functionlist.inl"
};

#define GRAPHICS_CB_VK_FUNC(name) PFN_##name name;
#define GRAPHICS_CB_VK_FUNC_EXT(name, extension) PFN_##name name;
struct GraphicsCBVkVTable
{
#include "functionlist.inl"
};

#define COMPUTE_CB_VK_FUNC(name) PFN_##name name;
#define COMPUTE_CB_VK_FUNC_EXT(name, extension) PFN_##name name;
struct ComputeCBVkVTable
{
#include "functionlist.inl"
};

#define TRANSFER_CB_VK_FUNC(name) PFN_##name name;
#define TRANSFER_CB_VK_FUNC_EXT(name, extension) PFN_##name name;
struct TransferCBVkVTable
{
#include "functionlist.inl"
};

} // namespace Vulkan

#define CHKED(x) if(auto r = (x); r != VK_SUCCESS) { LOG_S(ERROR) << "Vulkan Error: " << #x << " " << getVulkanResultString(r); }

#endif //WYRD_VULKAN_FUNCTIONS_H
