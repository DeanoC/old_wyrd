#pragma once
#ifndef WYRD_RENDER_RENDERPASS_H
#define WYRD_RENDER_RENDERPASS_H

#include "core/core.h"
#include "render/resources.h"
#include "render/types.h"
#include "render/generictextureformat.h"
#include "resourcemanager/resource.h"

namespace ResourceManager {
class ResourceMan;
}

namespace Render {

struct RenderPass : public ResourceManager::Resource<RenderPassId>
{
	using Ptr = std::shared_ptr<RenderPass>;
	using ConstPtr = std::shared_ptr<RenderPass const>;
	using WeakPtr = std::weak_ptr<RenderPass>;
	using ConstWeakPtr = std::weak_ptr<RenderPass const>;
	using Handle = ResourceManager::ResourceHandle<Id>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 1;

	static constexpr uint8_t MaxTargets = 16;
	struct Target
	{
		LoadOp load;
		StoreOp store;
		GenericTextureFormat format;

		LoadOp stencilLoad; // nop except for stencil formats
		StoreOp stencilStore;
	};

	uint8_t numTargets;
	uint8_t padd[3];
	// if load op is clear these will be used for all targets
	// TODO better clear support
	uint8_t byteClearValues[4];

	Target targets[MaxTargets];


};

}

#endif //WYRD_RENDERPASS_H
