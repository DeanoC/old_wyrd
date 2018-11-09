#pragma once
#ifndef WYRD_RENDER_VIEWPORT_H
#define WYRD_RENDER_VIEWPORT_H

#include "core/core.h"
#include "render/resources.h"
#include "render/types.h"
#include "render/generictextureformat.h"
#include "resourcemanager/resource.h"
#include <array>

namespace ResourceManager {
class ResourceMan;

struct ResourceNameView;
}

namespace Render {

struct ViewportAndScissor
{
	float x, y;
	float width, height;
	float minDepth, maxDepth;
	std::array<int32_t, 2> scissorOffset;
	std::array<uint32_t, 2> scissorExtent;
};

struct alignas(8) Viewport : public ResourceManager::Resource<ViewportId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 1;

	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			std::vector<ViewportAndScissor> const& viewports_) -> ViewportHandle;

	ViewportAndScissor* getViewports() { return (ViewportAndScissor*) (this + 1); }

	uint8_t numViewports;
	uint8_t padd[7];

};

}

#endif //WYRD_RENDERPASS_H
