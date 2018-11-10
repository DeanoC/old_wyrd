#pragma once
#ifndef WYRD_RENDER_ROPBLENDER_H
#define WYRD_RENDER_ROPBLENDER_H

#include "core/core.h"
#include "render/resources.h"
#include "render/types.h"
#include "resourcemanager/resourcehandle.h"
#include "resourcemanager/resource.h"

namespace ResourceManager {
class ResourceMan;

struct ResourceNameView;
}

namespace Render {

struct ROPTargetBlender
{
	static constexpr uint8_t Disable = 0;
	static constexpr uint8_t EnableFlag = Core::Bit(0);

	auto isEnabled() const -> bool { return flags & EnableFlag; }

	uint8_t flags;
	ROPBlendOps colourBlendOp;
	ROPBlendOps alphaBlendOp;
	ColourComponents writeMask;

	ROPBlendFactor srcColourFactor;
	ROPBlendFactor dstColourFactor;
	ROPBlendFactor srcAlphaFactor;
	ROPBlendFactor dstAlphaFactor;
};

struct alignas(8) ROPBlender : public ResourceManager::Resource<ROPBlenderId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static constexpr uint8_t LogicEnableFlag = Core::Bit(0);

	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			std::vector<ROPTargetBlender> const& blenders_,
			std::array<float, 4> const& constants_,
			bool logicOpEnable_ = false,
			ROPLogicOps logicOp_ = ROPLogicOps::Zero
	) -> ROPBlenderHandle;

	auto isLogicBlender() const { return flags & LogicEnableFlag; }

	ROPTargetBlender const* targetBlenders() const { return (ROPTargetBlender const*) (this + 1); }

	float constants[4];
	uint8_t numTargets;
	uint8_t flags;
	ROPLogicOps logicOp;
	uint8_t padd[1];
};

}

#endif //WYRD_RENDER_ROPBLENDER_H
