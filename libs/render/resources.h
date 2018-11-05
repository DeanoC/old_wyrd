#pragma once
#ifndef WYRD_RENDER_RESOURCEHANDLERS_H
#define WYRD_RENDER_RESOURCEHANDLERS_H

#include "core/core.h"
#include "resourcemanager/resourceid.h"

namespace ResourceManager {
class ResourceMan;

template<uint32_t>
struct ResourceHandle;
}

namespace Render {
auto RegisterResourceHandlers(ResourceManager::ResourceMan& rm_) -> void;
using ResourceManager::operator ""_resource_id;

// render resource Ids
static constexpr uint32_t GenericImageId = "GIMG"_resource_id;
static constexpr uint32_t RenderPassId = "RPAS"_resource_id;
static constexpr uint32_t RenderTargetId = "RTGT"_resource_id;
static constexpr uint32_t TextureId = "TXTR"_resource_id;

using GenericImageHandle = ResourceManager::ResourceHandle<GenericImageId>;
using RenderPassHandle = ResourceManager::ResourceHandle<RenderPassId>;
using RenderTargetHandle = ResourceManager::ResourceHandle<RenderTargetId>;
using TextureHandle = ResourceManager::ResourceHandle<TextureId>;

}


#endif //WYRD_RENDER_RESOURCEHANDLERS_H
