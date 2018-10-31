#pragma once
#ifndef WYRD_VULKAN_RENDERPASS_H
#define WYRD_VULKAN_RENDERPASS_H

#include "core/core.h"
#include "render/renderpass.h"
#include "vulkan/api.h"

namespace Vulkan {
struct RenderPass : public Render::RenderPass
{
	using Ptr = std::shared_ptr<RenderPass>;
	using WeakPtr = std::shared_ptr<RenderPass>;

	~RenderPass() final;

	VkRenderPass renderpass;
};

}

#endif //WYRD_RENDERPASS_H
