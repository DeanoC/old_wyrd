#pragma once
#ifndef WYRD_RENDER_RASTERISATIONSTATE_H
#define WYRD_RENDER_RASTERISATIONSTATE_H

#include "core/core.h"
#include "render/resources.h"
#include "render/types.h"
#include "resourcemanager/resource.h"
#include "resourcemanager/resourcehandle.h"
#include "resourcemanager/resourcename.h"

namespace ResourceManager { class ResourceMan; }

namespace Render {

enum class RasterisationStateFlags : uint16_t
{
	None = 0,
	DepthClampEnable = Core::Bit(0),
	DepthBiasEnable = Core::Bit(1),
	DepthTestEnable = Core::Bit(2),
	DepthWriteEnable = Core::Bit(3),
	DepthBoundsEnable = Core::Bit(4),
	StencilTestEnable = Core::Bit(5),
	Discard = Core::Bit(6),
	SampleRateShading = Core::Bit(7),
	AlphaToCoverage = Core::Bit(8),
	AlphaToOne = Core::Bit(9),
};

constexpr auto is_bitmask_enum(RasterisationStateFlags) -> bool { return true; }

struct DepthBias
{
	float constantFactor;
	float clamp;
	float slopeFactor;
};

struct StencilState
{
	StencilOp failOp;
	StencilOp passOp;
	StencilOp depthFailOp;
	CompareOp compareOp;

	uint8_t compareMask;
	uint8_t writeMask;
	uint8_t reference;
	uint8_t padd;
};

struct alignas(8) RasterisationState : public ResourceManager::Resource<RasterisationStateId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	auto isDepthClampEnabled() const { return testFlag(RasterisationStateFlags::DepthClampEnable); }

	auto isDepthBiasEnabled() const { return testFlag(RasterisationStateFlags::DepthBiasEnable); }

	auto isDepthTestEnabled() const { return testFlag(RasterisationStateFlags::DepthTestEnable); }

	auto isDepthWriteEnabled() const { return testFlag(RasterisationStateFlags::DepthWriteEnable); }

	auto isDepthBoundsEnabled() const { return testFlag(RasterisationStateFlags::DepthBoundsEnable); }

	auto isStencilTestEnabled() const { return testFlag(RasterisationStateFlags::StencilTestEnable); }

	auto isDiscardEnabled() const { return testFlag(RasterisationStateFlags::Discard); }

	auto isSampleRateShadingEnabled() const { return testFlag(RasterisationStateFlags::SampleRateShading); }

	auto isAlphaToCoverageEnabled() const { return testFlag(RasterisationStateFlags::AlphaToCoverage); }

	auto isAlphaToOneEnabled() const { return testFlag(RasterisationStateFlags::AlphaToOne); }

	static auto Create(std::shared_ptr<ResourceManager::ResourceMan> rm_,
					   ResourceManager::ResourceNameView const& name_,
					   RasterisationStateFlags flags_ = RasterisationStateFlags::None,
					   CullMode cullMode_ = CullMode::None,
					   CompareOp depthCompare_ = CompareOp::GreaterOrEqual,
					   FillMode fillMode_ = FillMode::Fill,
					   FrontFace frontFace_ = FrontFace::CounterClockWise,
					   SampleCounts sampleCount_ = SampleCounts::One,
					   StencilState frontStencil_ = {},
					   StencilState backStencil_ = {},
					   float lineWidth_ = 1.0f,
					   float minDepthBounds_ = 0.0f,
					   float maxDepthBounds_ = 0.0f,
					   float minSampleShadingRate_ = 1.0f,
					   DepthBias depthBias_ = {},
					   uint32_t patchControlPointsCount_ = 0,
					   uint64_t sampleMask_ = ~0
	) -> RasterisationStateHandle;

	FrontFace frontFace;
	CullMode cullMode;
	FillMode fillMode;
	uint8_t padd8[1];

	CompareOp depthCompare;
	SampleCounts sampleCount;
	RasterisationStateFlags flags;

	StencilState frontStencil;
	StencilState backStencil;

	float lineWidth = 1.0f;
	float minDepthBounds = 0.0f;
	float maxDepthBounds = 0.0f;
	float minSampleShadingRate;

	DepthBias depthBias;
	uint32_t patchControlPointsCount;

	uint64_t sampleMask;

	constexpr auto testFlag(RasterisationStateFlags flag_) const -> bool
	{
		return Core::bitmask::test_equal(flags, flag_);
	}

};

}

#endif //WYRD_RENDER_RASTERISATIONSTATE_H
