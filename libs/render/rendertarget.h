#pragma once
#ifndef WYRD_RENDER_RENDERTARGET_H
#define WYRD_RENDER_RENDERTARGET_H

#include "core/core.h"
#include "render/resources.h"
#include "render/renderpass.h"
#include "render/texture.h"
#include "resourcemanager/resource.h"

namespace Render {
struct RenderTarget : public ResourceManager::Resource<RenderTargetId>
{
	using Ptr = std::shared_ptr<RenderTarget>;
	using ConstPtr = std::shared_ptr<RenderTarget const>;
	using WeakPtr = std::weak_ptr<RenderTarget>;
	using ConstWeakPtr = std::weak_ptr<RenderTarget const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	RenderPass::Handle renderPassHandle;
	TextureHandle targetHandles[RenderPass::MaxTargets];
	int32_t renderOffset[2];
	uint32_t renderExtent[2];

};

}
#endif //WYRD_RENDERTARGET_H
