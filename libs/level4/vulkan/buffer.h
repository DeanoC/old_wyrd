#pragma once
#ifndef WYRD_VULKAN_BUFFER_H
#define WYRD_VULKAN_BUFFER_H

#include "core/core.h"
#include "render/renderpass.h"
#include "vulkan/api.h"

namespace Vulkan {
struct Device;

struct Buffer
{
	using Ptr = std::shared_ptr<Buffer>;
	using ConstPtr = std::shared_ptr<Buffer const>;
	using WeakPtr = std::weak_ptr<Buffer>;
	using ConstWeakPtr = std::weak_ptr<Buffer const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

#define BUFFER_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(renderpass, args...); }
#define BUFFER_VK_FUNC_EXT(name, extension) BUFFER_VK_FUNC(name)

#include "functionlist.inl"

	VkBuffer buffer;
	VmaAllocation allocation;

	BufferVkVTable* vtable;
};

}

#endif //WYRD_VULKAN_BUFFER_H
