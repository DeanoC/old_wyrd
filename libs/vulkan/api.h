#pragma once
#ifndef WYRD_VULKAN_FUNCTIONS_H
#define WYRD_VULKAN_FUNCTIONS_H

#if PLATFORM == WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR 1
#elif PLATFORM == OSX
#define VK_USE_PLATFORM_MACOS_MVK 1
#else
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include "vulkan/vk_mem_alloc.h"
#include <string_view>


namespace Vulkan {
#define GLOBAL_VK_FUNC( name ) extern PFN_##name name;
#define INSTANCE_VK_FUNC( name ) extern PFN_##name name;
#define INSTANCE_VK_FUNC_EXT( name, extension ) extern PFN_##name name;
#include "functionlist.inl"
std::string_view const getVulkanResultString(VkResult result);

// some evil magic/include madness to simulate c++ vtables from a
// dynamically bound shared library/dll

// device function table includes many things i've also broken out
// into 'object' based vtables.
#define DEVICE_VK_FUNC(name) PFN_##name name;
#define DEVICE_VK_FUNC_EXT(name, extension)PFN_##name name;
#define FENCE_VK_FUNC(name) PFN_##name name;
#define FENCE_VK_FUNC_EXT(name, extension)PFN_##name name;
#define SEMAPHORE_VK_FUNC(name) PFN_##name name;
#define SEMAPHORE_VK_FUNC_EXT(name, extension) PFN_##name name;
#define EVENT_VK_FUNC(name) PFN_##name name;
#define EVENT_VK_FUNC_EXT(name, extension) PFN_##name name;
#define QUERYPOOL_VK_FUNC(name) PFN_##name name;
#define QUERYPOOL_VK_FUNC_EXT(name, extension) PFN_##name name;
#define BUFFER_VK_FUNC(name) PFN_##name name;
#define BUFFER_VK_FUNC_EXT(name, extension) PFN_##name name;
#define BUFFERVIEW_VK_FUNC(name) PFN_##name name;
#define BUFFERVIEW_VK_FUNC_EXT(name, extension) PFN_##name name;
#define IMAGE_VK_FUNC(name) PFN_##name name;
#define IMAGE_VK_FUNC_EXT(name, extension) PFN_##name name;
#define IMAGEVIEW_VK_FUNC(name) PFN_##name name;
#define IMAGEVIEW_VK_FUNC_EXT(name, extension) PFN_##name name;
#define SHADERMODULE_VK_FUNC(name) PFN_##name name;
#define SHADERMODULE_VK_FUNC_EXT(name, extension) PFN_##name name;
#define PIPELINE_VK_FUNC(name) PFN_##name name;
#define PIPELINE_VK_FUNC_EXT(name, extension) PFN_##name name;
#define SAMPLER_VK_FUNC(name) PFN_##name name;
#define SAMPLER_VK_FUNC_EXT(name, extension) PFN_##name name;
#define DESCRIPTORSET_VK_FUNC(name) PFN_##name name;
#define DESCRIPTORSET_VK_FUNC_EXT(name, extension) PFN_##name name;
#define FRAMEBUFFER_VK_FUNC(name) PFN_##name name;
#define FRAMEBUFFER_VK_FUNC_EXT(name, extension) PFN_##name name;
#define RENDERPASS_VK_FUNC(name) PFN_##name name;
#define RENDERPASS_VK_FUNC_EXT(name, extension) PFN_##name name;
#define COMMANDPOOL_VK_FUNC(name) PFN_##name name;
#define COMMANDPOOL_VK_FUNC_EXT(name, extension) PFN_##name name;
#define COMMANDBUFFER_VK_FUNC(name) PFN_##name name;
#define COMMANDBUFFER_VK_FUNC_EXT(name, extension) PFN_##name name;

struct DeviceVkVTable
{
#include "functionlist.inl"
};

#define FENCE_VK_FUNC(name) PFN_##name name;
#define FENCE_VK_FUNC_EXT(name, extension) FENCE_VK_FUNC(name)
struct FenceVkVTable
{
#include "functionlist.inl"
};

#define SEMAPHORE_VK_FUNC(name) PFN_##name name;
#define SEMAPHORE_VK_FUNC_EXT(name, extension) SEMAPHORE_VK_FUNC(name)
struct SemaphoreVkVTable
{
#include "functionlist.inl"
};

#define EVENT_VK_FUNC(name) PFN_##name name;
#define EVENT_VK_FUNC_EXT(name, extension) EVENT_VK_FUNC(name)
struct EventVkVTable
{
#include "functionlist.inl"
};

#define QUERYPOOL_VK_FUNC(name) PFN_##name name;
#define QUERYPOOL_VK_FUNC_EXT(name, extension) QUERYPOOL_VK_FUNC(name)
struct QueryPoolVkVTable
{
#include "functionlist.inl"
};

#define BUFFER_VK_FUNC(name) PFN_##name name;
#define BUFFER_VK_FUNC_EXT(name, extension) BUFFER_VK_FUNC(name)
struct BufferVkVTable
{
#include "functionlist.inl"
};

#define BUFFERVIEW_VK_FUNC(name) PFN_##name name;
#define BUFFERVIEW_VK_FUNC_EXT(name, extension) BUFFERVIEW_VK_FUNC(name)
struct BufferViewVkVTable
{
#include "functionlist.inl"
};

#define IMAGE_VK_FUNC(name) PFN_##name name;
#define IMAGE_VK_FUNC_EXT(name, extension) IMAGE_VK_FUNC(name)
struct ImageVkVTable
{
#include "functionlist.inl"
};

#define IMAGEVIEW_VK_FUNC(name) PFN_##name name;
#define IMAGEVIEW_VK_FUNC_EXT(name, extension) IMAGEVIEW_VK_FUNC(name)
struct ImageViewVkVTable
{
#include "functionlist.inl"
};
#define SHADERMODULE_VK_FUNC(name) PFN_##name name;
#define SHADERMODULE_VK_FUNC_EXT(name, extension) SHADERMODULE_VK_FUNC(name)
struct ShaderModuleVkVTable
{
#include "functionlist.inl"
};

#define PIPELINE_VK_FUNC(name) PFN_##name name;
#define PIPELINE_VK_FUNC_EXT(name, extension) PIPELINE_VK_FUNC(name)
struct PipelineVkVTable
{
#include "functionlist.inl"
};

#define SAMPLER_VK_FUNC(name) PFN_##name name;
#define SAMPLER_VK_FUNC_EXT(name, extension) SAMPLER_VK_FUNC(name)
struct SamplerVkVTable
{
#include "functionlist.inl"
};

#define DESCRIPTORSET_VK_FUNC(name) PFN_##name name;
#define DESCRIPTORSET_VK_FUNC_EXT(name, extension) DESCRIPTORSET_VK_FUNC(name)
struct DescriptorSetVkVTable
{
#include "functionlist.inl"
};

#define FRAMEBUFFER_VK_FUNC(name) PFN_##name name;
#define FRAMEBUFFER_VK_FUNC_EXT(name, extension) FRAMEBUFFER_VK_FUNC(name)
struct FramebufferVkVTable
{
#include "functionlist.inl"
};

#define RENDERPASS_VK_FUNC(name) PFN_##name name;
#define RENDERPASS_VK_FUNC_EXT(name, extension) RENDERPASS_VK_FUNC(name)
struct RenderPassVkVTable
{
#include "functionlist.inl"
};

#define COMMANDPOOL_VK_FUNC(name) PFN_##name name;
#define COMMANDPOOL_VK_FUNC_EXT(name, extension) COMMANDPOOL_VK_FUNC(name)
struct CommandPoolVkVTable
{
#include "functionlist.inl"
};

#define QUEUE_VK_FUNC(name) PFN_##name name;
#define QUEUE_VK_FUNC_EXT(name, extension) QUEUE_VK_FUNC(name)
struct QueueVkVTable
{
#include "functionlist.inl"
};

#define GENERAL_CB_VK_FUNC(name) PFN_##name name;
#define GENERAL_CB_VK_FUNC_EXT(name, extension) GENERAL_CB_VK_FUNC(name)
struct GeneralCBVkVTable
{
#include "functionlist.inl"
};

#define GFXCOMP_CB_VK_FUNC(name) PFN_##name name;
#define GFXCOMP_CB_VK_FUNC_EXT(name, extension) GFXCOMP_CB_VK_FUNC(name)
struct GCCBVkVTable : public GeneralCBVkVTable
{
#include "functionlist.inl"
};


#define GRAPHIC_CB_VK_FUNC(name) PFN_##name name;
#define GRAPHIC_CB_VK_FUNC_EXT(name, extension) GRAPHICS_CB_VK_FUNC(name)

struct GraphicsCBVkVTable : public GCCBVkVTable
{
#include "functionlist.inl"
};

#define COMPUTE_CB_VK_FUNC(name) PFN_##name name;
#define COMPUTE_CB_VK_FUNC_EXT(name, extension) COMPUTE_CB_VK_FUNC(name);
struct ComputeCBVkVTable : public GCCBVkVTable
{
#include "functionlist.inl"
};

} // namespace Vulkan

#define CHKED(x) if(auto r = (x); r != VK_SUCCESS) { LOG_S(ERROR) << "Vulkan Error: " << #x << " " << getVulkanResultString(r); }

#endif //WYRD_VULKAN_FUNCTIONS_H
