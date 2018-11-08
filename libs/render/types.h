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

enum class EncoderFlag : uint32_t
{
	RenderEncoder = (uint32_t) CommandQueueFlavour::Render,
	ComputeEncoder = (uint32_t) CommandQueueFlavour::Compute,
	Callable = Core::Bit(4),
};


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

enum class TextureFlag : uint32_t
{
	// should we keep a copy of the texture data after init?
			KeepCpuCopy = Core::Bit(0),
	NoInit = Core::Bit(1),
	InitZero = Core::Bit(2),
	CubeMap = Core::Bit(3),
	ComputeMipMaps = Core::Bit(4),
	UsageMask = (uint32_t) (Usage::MASK) << 5
};

enum class MemoryAccess : uint32_t
{
	IndirectCommandRead = Core::Bit(0),
	IndexRead = Core::Bit(1),
	VertexRead = Core::Bit(2),
	ConstantRead = Core::Bit(3),
	InputAttachmentRead = Core::Bit(4),
	ShaderRead = Core::Bit(5),
	ShaderWrite = Core::Bit(6),
	ColourRopRead = Core::Bit(7),
	ColourRopWrite = Core::Bit(8),
	DepthStencilRopRead = Core::Bit(9),
	DepthStencilRopWrite = Core::Bit(10),
	DMARead = Core::Bit(11),
	DMAWrite = Core::Bit(12),
	CpuRead = Core::Bit(13),
	CpuWrite = Core::Bit(14),
	GeneralRead = Core::Bit(15),
	GeneralWrite = Core::Bit(16),
};

enum class RenderPipelineStages : uint32_t
{
	Begin = Core::Bit(0), // pseudo stage for syncing at the beginning
	DrawIndirect = Core::Bit(1),
	VertexInput = Core::Bit(2),
	VertexShader = Core::Bit(3),
	TessellationControlShader = Core::Bit(4),
	TesselationEvalShader = Core::Bit(5),
	GeometryShader = Core::Bit(6),
	EarlyFragmentTests = Core::Bit(7),
	FragmentShader = Core::Bit(8),
	LateFragmentTests = Core::Bit(9),
	ROPWrite = Core::Bit(10),
	End = Core::Bit(11), // pseudo stage for syncing at the end
	AllGfx = ~1u, // all graphics stage
	All = ~0u, // all commands in the queue
};

enum class ComputePipelineStages : uint32_t
{
	Begin = Core::Bit(0), // pseudo stage for syncing at the beginning
	DrawIndirect = Core::Bit(1),
	ComputeShader = Core::Bit(2),
	End = Core::Bit(3), // pseudo stage for syncing at the end
	All = ~0u, // all commands in the queue
};

enum class HostPipelineStages : uint32_t
{
	Host = Core::Bit(0),
	All = ~0u, // all commands in the queue
};

enum class DMAPipelineStages : uint32_t
{
	Begin = Core::Bit(0), // pseudo stage for syncing at the beginning
	DMA = Core::Bit(1),
	End = Core::Bit(2), // pseudo stage for syncing at the end
	All = ~0u, // all commands in the queue
};

enum class Topology : uint8_t
{
	Points,
	Lines,
	Triangles,
	Patches,
	LinesWithAdjacecy,
	TrianglesWithAdjacency,
	LineStrips,
	TriangleStrips,
	LineStripsWithAdjacency,
	TriangleStripsWithAdjacency,
	TriangleFans,
};

enum class ShaderSourceLanguage : uint8_t
{
	GLSL,
	HLSL,
};

enum class ShaderType : uint8_t
{
	Vertex = Core::Bit(0),
	TesselationControl = Core::Bit(1),
	TesselationEval = Core::Bit(2),
	Geometry = Core::Bit(3),
	Fragment = Core::Bit(4),
	Compute = Core::Bit(5)
};

enum class BindingTableType : uint8_t
{
	Texture,
	RWTexture,
	Buffer,
	RWBuffer,
	TextureBuffer,
	RWTextureBuffer,
	DynamicBuffer,
	DynamicRWBuffer,
	Sampler,
};

constexpr auto is_bitmask_enum(RenderPipelineStages) -> bool { return true; }

constexpr auto is_bitmask_enum(ComputePipelineStages) -> bool { return true; }

constexpr auto is_bitmask_enum(HostPipelineStages) -> bool { return true; }

constexpr auto is_bitmask_enum(DMAPipelineStages) -> bool { return true; }

constexpr auto is_bitmask_enum(ShaderType) -> bool { return true; }

constexpr auto is_bitmask_enum(TextureFlag) -> bool { return true; }

constexpr auto is_bitmask_enum(EncoderFlag) -> bool { return true; }

constexpr auto is_bitmask_enum(CommandQueueFlavour) -> bool { return true; }

constexpr auto is_bitmask_enum(Usage) -> bool { return true; }

constexpr auto is_bitmask_enum(Render::MemoryAccess) -> bool { return true; }


constexpr auto TextureFlagsToUsage(TextureFlag flags_) -> Usage
{
	using namespace Core::bitmask;
	return from_uint<Usage>(to_uint(flags_ & TextureFlag::UsageMask) >> 5);
}

constexpr auto TextureFlagFromUsage(Usage usage) -> TextureFlag
{
	using namespace Core::bitmask;
	return from_uint<TextureFlag>(to_uint(usage) << 5);
}

}

#endif //WYRD_RENDER_TYPES_H
