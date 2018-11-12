#pragma once
#ifndef WYRD_RENDER_RENDERPASS_H
#define WYRD_RENDER_RENDERPASS_H

#include "core/core.h"
#include "render/resources.h"
#include "render/types.h"
#include "render/generictextureformat.h"
#include "resourcemanager/resource.h"
#include "math/vector_math.h"

namespace ResourceManager {
class ResourceMan;

struct ResourceNameView;
}

namespace Render {

struct alignas(8) RenderPass : public ResourceManager::Resource<RenderPassId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 1;

	struct Target
	{
		LoadOp load;
		StoreOp store;
		GenericTextureFormat format;

		LoadOp stencilLoad; // nop except for stencil formats
		StoreOp stencilStore;
	};
	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			std::vector<Target> const& targets_,
			std::vector<Math::Vector4> const& clearValues_) -> RenderPassHandle;

	Target const* getTargets() const { return (Target const*) (this + 1); }
	Math::Vector4 const* getClearValues() const { return (Math::Vector4 const*)(getTargets() + numTargets); }

	uint8_t numTargets;
	uint8_t padd[3];

};

}

#endif //WYRD_RENDERPASS_H
