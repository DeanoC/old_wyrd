#pragma once
#ifndef WYRD_RENDERTARGETS_H
#define WYRD_RENDERTARGETS_H

#include "core/core.h"
#include "render/rendertarget.h"
#include "vulkan/api.h"

namespace Vulkan {
struct Device;

struct RenderTarget
{
	using Ptr = std::shared_ptr<RenderTarget>;
	using ConstPtr = std::shared_ptr<RenderTarget const>;
	using WeakPtr = std::weak_ptr<RenderTarget>;
	using ConstWeakPtr = std::weak_ptr<RenderTarget const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

#define FRAMEBUFFER_VK_FUNC_EXT(name) template<typename... Args> auto name(Args... args) { return vtable-> name(renderpass, args...); }

#include "functionlist.inl"

	VkFramebuffer framebuffer;
	FramebufferVkVTable* vtable;
};

}

#endif //WYRD_RENDERTARGETS_H
