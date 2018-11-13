#pragma once
#ifndef WYRD_VULKAN_RENDERPASS_H
#define WYRD_VULKAN_RENDERPASS_H

#include "core/core.h"
#include "render/renderpass.h"
#include "vulkan/api.h"

namespace Vulkan {
struct Device;

struct RenderPass
{
	using Ptr = std::shared_ptr<RenderPass>;
	using ConstPtr = std::shared_ptr<RenderPass const>;
	using WeakPtr = std::weak_ptr<RenderPass>;
	using ConstWeakPtr = std::weak_ptr<RenderPass const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

#define RENDERPASS_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(renderpass, args...); }
#define RENDERPASS_VK_FUNC_EXT(name, extension) RENDERPASS_VK_FUNC(name)

#include "functionlist.inl"

	VkRenderPass renderpass;
	RenderPassVkVTable* vtable;
};

}

#endif //WYRD_RENDERPASS_H
