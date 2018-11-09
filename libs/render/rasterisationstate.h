#pragma once
#ifndef WYRD_RENDER_RASTERISATIONSTATE_H
#define WYRD_RENDER_RASTERISATIONSTATE_H

#include "core/core.h"
#include "render/types.h"

namespace Render {

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

struct RasterisationState
{
	static constexpr uint8_t DepthClampEnableFlag = Core::Bit(0);
	static constexpr uint8_t DepthBiasEnableFlag = Core::Bit(1);
	static constexpr uint8_t DepthTestEnableFlag = Core::Bit(2);
	static constexpr uint8_t DepthWriteEnableFlag = Core::Bit(3);
	static constexpr uint8_t DepthBoundsEnableFlag = Core::Bit(4);
	static constexpr uint8_t StencilTestEnableFlag = Core::Bit(5);
	static constexpr uint8_t DiscardFlag = Core::Bit(6);
	static constexpr uint8_t SampleRateShadingFlag = Core::Bit(7);
	static constexpr uint8_t AlphaToCoverageFlag = Core::Bit(8);
	static constexpr uint8_t AlphaToOneFlag = Core::Bit(9);

	auto isDepthClampEnabled() const { return flags & DepthClampEnableFlag; }

	auto isDepthBiasEnabled() const { return flags & DepthBiasEnableFlag; }

	auto isDepthTestEnabled() const { return flags & DepthTestEnableFlag; }

	auto isDepthWriteEnabled() const { return flags & DepthWriteEnableFlag; }

	auto isDepthBoundsEnabled() const { return flags & DepthBoundsEnableFlag; }

	auto isStencilTestEnable() const { return flags & StencilTestEnableFlag; }

	auto isDiscardEnabled() const { return flags & DiscardFlag; }

	FrontFace frontFace;
	CullMode cullMode;
	FillMode fillMode;
	uint8_t padd8[1];

	CompareOp depthCompare;
	SampleCounts sampleCount;
	uint16_t flags;

	StencilState frontStencil;
	StencilState backStencil;

	float lineWidth;
	float minDepthBounds;
	float maxDepthBounds;
	float minSampleShadingRate;

	DepthBias depthBias;
	uint32_t patchControlPointsCount;

	uint64_t sampleMask;
};

static_assert(sizeof(RasterisationState) == 64);

}

#endif //WYRD_RENDER_RASTERISATIONSTATE_H
