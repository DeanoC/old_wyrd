#pragma once
#ifndef WYRD_RENDER_TYPES_H
#define WYRD_RENDER_TYPES_H

#include "core/core.h"
#include "core/utils.h"

namespace Render {

enum class CommandQueueFlavour : uint8_t
{
	Render = Core::Bit(0),
	Compute = Core::Bit(1),
	DMA = Core::Bit(2),
	Present = Core::Bit(3),
};

constexpr auto is_bitmask_enum(Render::CommandQueueFlavour) -> bool { return true; }

enum class EncoderFlag : uint32_t
{
	RenderEncoder = Core::bitmask::to_uint(CommandQueueFlavour::Render),
	ComputeEncoder = Core::bitmask::to_uint(CommandQueueFlavour::Compute),
	Callable = Core::Bit(4),
};

constexpr auto is_bitmask_enum(Render::EncoderFlag) -> bool { return true; }

// GtfCracker function take a channel in, where the index refers to the
// physical placement in that format. If you want order agonistic use
// swizzle and the channel enum
enum class Channel
{
	R = 0, G = 1, B = 2, A = 3,
};

enum class LoadOp
{
	Load,
	Clear,
	DontCare
};

enum class StoreOp
{
	Store,
	DontCare
};

enum class Usage : uint32_t
{
	DMASrc = Core::Bit(0),
	DMADst = Core::Bit(1),
	ShaderRead = Core::Bit(2),
	ShaderWrite = Core::Bit(3),
	RopRead = Core::Bit(4),
	RopWrite = Core::Bit(5),
	MASK = DMASrc | DMADst | ShaderRead | RopRead | RopWrite,
};

constexpr auto is_bitmask_enum(Usage) -> bool { return true; }

enum class TextureFlag : uint32_t
{
	// should we keep a copy of the texture data after init?
			KeepCpuCopy = Core::Bit(0),
	NoInit = Core::Bit(1),
	InitZero = Core::Bit(2),
	CubeMap = Core::Bit(3),
	ComputeMipMaps = Core::Bit(4),
	Usage = Core::bitmask::to_uint(Usage::MASK) << 5
};

constexpr auto is_bitmask_enum(Render::TextureFlag) -> bool { return true; }

constexpr auto extractUsage(TextureFlag flags_) -> Usage
{
	using namespace Core::bitmask;
	return from_uint<Usage>(to_uint(flags_ & TextureFlag::Usage) >> 5);
}

}

#endif //WYRD_RENDER_TYPES_H
