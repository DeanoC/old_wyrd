#pragma once
#ifndef WYRD_VULKAN_SHADERMODULE_H
#define WYRD_VULKAN_SHADERMODULE_H

#include "core/core.h"
#include "vulkan/api.h"

namespace ResourceManager { class ResourceMan; }

namespace Vulkan {
struct Device;

struct ShaderModule
{
	using Ptr = std::shared_ptr<ShaderModule>;
	using ConstPtr = std::shared_ptr<ShaderModule const>;
	using WeakPtr = std::weak_ptr<ShaderModule>;
	using ConstWeakPtr = std::weak_ptr<ShaderModule const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

#define SHADERMODULE_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(renderpass, args...); }

#include "functionlist.inl"

	VkShaderModule shaderModule;

	ShaderModuleVkVTable* vtable;

};

}
#endif //WYRD_VULKAN_SHADERMODULE_H
