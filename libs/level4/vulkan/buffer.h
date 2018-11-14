#pragma once
#ifndef WYRD_VULKAN_BUFFER_H
#define WYRD_VULKAN_BUFFER_H

#include "core/core.h"
#include "render/buffer.h"
#include "vulkan/api.h"

namespace Vulkan {
struct Device;

struct Buffer : public Render::IGpuBuffer
{
	using Ptr = std::shared_ptr<Buffer>;
	using ConstPtr = std::shared_ptr<Buffer const>;
	using WeakPtr = std::weak_ptr<Buffer>;
	using ConstWeakPtr = std::weak_ptr<Buffer const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

	auto update(uint8_t const* data_, uint64_t size_) -> void final;
	auto map() -> void* final;
	auto unmap() -> void final;


#define BUFFER_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(buffer, args...); }
#define BUFFER_VK_FUNC_EXT(name, extension) BUFFER_VK_FUNC(name)

#include "functionlist.inl"

	std::weak_ptr<Device> weakDevice;
	VkBuffer buffer;
	VmaAllocation allocation;
	VkBufferCreateInfo createInfo;

	BufferVkVTable* vtable;
};

}

#endif //WYRD_VULKAN_BUFFER_H
