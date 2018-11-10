#pragma once
#ifndef WYRD_RENDER_RENDERTARGET_H
#define WYRD_RENDER_RENDERTARGET_H

#include "core/core.h"
#include "render/resources.h"
#include "render/renderpass.h"
#include "render/texture.h"
#include "resourcemanager/resource.h"

namespace Render {
struct alignas(8) RenderTarget : public ResourceManager::Resource<RenderTargetId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			RenderPassHandle renderPassHandle_,
			std::vector<TextureHandle> const& targetTextures_,
			std::array<int32_t, 2> const& renderOffset_,
			std::array<uint32_t, 2> const& renderExtent_) -> RenderTargetHandle;

	TextureHandle const* getTargetTextures() const { return (TextureHandle const*) (this + 1); }

	RenderPassHandle renderPassHandle;
	int32_t renderOffset[2];
	uint32_t renderExtent[2];
	uint8_t numTargetTextures;
	uint8_t padd[7];

protected:
	TextureHandle* getTargetTextures() { return (TextureHandle*) (this + 1); }
};

}
#endif //WYRD_RENDERTARGET_H
