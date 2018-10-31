#pragma once
#ifndef WYRD_RENDERTARGETS_H
#define WYRD_RENDERTARGETS_H

#include "core/core.h"
#include "render/rendertarget.h"
#include "vulkan/api.h"

namespace Vulkan {
struct Device;

struct RenderTarget : public Render::RenderTarget
{
	using Ptr = std::shared_ptr<RenderTarget>;
	using WeakPtr = std::shared_ptr<RenderTarget>;

	RenderTarget(std::shared_ptr<Device> const& device_);
	~RenderTarget();

	std::weak_ptr<Device> weakDevice;
	VkFramebuffer framebuffer;
};

}

#endif //WYRD_RENDERTARGETS_H
