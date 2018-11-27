#pragma once
#ifndef WYRD_VULKAN_PIPELINE_H
#define WYRD_VULKAN_PIPELINE_H

#include "core/core.h"
#include "vulkan/api.h"

namespace ResourceManager { class ResourceMan; }

namespace Vulkan {
struct Device;

struct RenderPipeline
{
	using Ptr = std::shared_ptr<RenderPipeline>;
	using ConstPtr = std::shared_ptr<RenderPipeline const>;
	using WeakPtr = std::weak_ptr<RenderPipeline>;
	using ConstWeakPtr = std::weak_ptr<RenderPipeline const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

#define PIPELINE_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(pipeline, args...); }
#define PIPELINE_VK_FUNC_EXT(name, extension) PIPELINE_VK_FUNC(name)
#include "functionlist.inl"

	VkPipelineLayout layout; // TODO share layouts?
	VkPipeline pipeline;
	PipelineVkVTable* vtable;

};

}
#endif //WYRD_PIPELINE_H
