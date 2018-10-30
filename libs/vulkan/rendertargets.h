#pragma once
#ifndef WYRD_RENDERTARGETS_H
#define WYRD_RENDERTARGETS_H

#include <vulkan/vulkan.h>
#include "core/core.h"
#include "render/rendertargets.h"

namespace Vulkan {
struct RenderTargets : public Render::RenderTargets
{
	~RenderTargets();
	VkFramebuffer framebuffer;
};

}

#endif //WYRD_RENDERTARGETS_H
