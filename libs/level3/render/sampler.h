#pragma once
#ifndef WYRD_RENDER_SAMPLER_H
#define WYRD_RENDER_SAMPLER_H

#include "core/core.h"
#include "resourcemanager/resource.h"
#include "resourcemanager/resourcename.h"
#include "render/resources.h"
#include "render/types.h"

namespace Render {

enum class SamplerFlags : uint8_t
{
	None = 0,
	CompareEnable = Core::Bit(1),
	AnistropyEnable = Core::Bit(2),
};

constexpr auto is_bitmask_enum(SamplerFlags) -> bool { return true; }

struct alignas(8) Sampler : public ResourceManager::Resource<SamplerId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 1;

	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			Filter magFilter_  = Filter::Linear,
			Filter minFilter_  = Filter::Linear,
			Filter mipFilter_  = Filter::Linear,
			SamplerAddressMode uAddressMode_ = SamplerAddressMode::ClampToEdge,
			SamplerAddressMode vAddressMode_ = SamplerAddressMode::ClampToEdge,
			SamplerAddressMode wAddressMode_ = SamplerAddressMode::ClampToEdge,
			SamplerFlags flags_ = SamplerFlags::None,
			float maxAnistropy_ = 0.0f,
			float lodBias_ = 0.0f,
			CompareOp compareOp_ = CompareOp::Always,
			float minLod_ = -1000.0f,
			float maxLod_ = 1000.0f
			) -> SamplerHandle;

	auto isCompareEnabled() const { return Core::test_equal(flags, SamplerFlags::CompareEnable); }
	auto isAnistropyEnabled() const { return Core::test_equal(flags, SamplerFlags::AnistropyEnable); }

	Filter magFilter;
	Filter minFilter;
	Filter mipFilter;
	SamplerAddressMode uAddressMode;
	SamplerAddressMode vAddressMode;
	SamplerAddressMode wAddressMode;
	CompareOp compareOp;
	SamplerFlags flags;

	float lodBias;
	float maxAnistropy;
	float minLod;
	float maxLod;


};

}
#endif //WYRD_RENDER_SAMPLER_H
