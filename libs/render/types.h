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

	VertexRead = Core::Bit(6),
	IndexRead = Core::Bit(7),
	IndirectRead = Core::Bit(8),
	TextureRead = Core::Bit(9), // implicitly true for textures, buffers can use for texture buffers
	TextureWrite = Core::Bit(10) // implicity true for texture with ShaderWrite, buffer that can be written to as a texture
};

static constexpr uint32_t UsageBitCount = 11u;
static constexpr uint32_t UsageMask = (1u << UsageBitCount) - 1u;


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

enum class ROPLogicOps : uint8_t
{
	Dest,        // d
	Src,        // s
	NSrc,        // ~s
	Zero,        // 0
	NZero,        // all 1s

	Or,            // s | d
	OrNSrc,        // ~s | d
	OrNDest,    // s | ~d
	And,        // s & d
	AndNSrc,    // ~s & d
	AndNDest,    // s & ~d
	Xor,        // s ^ d

	Not,        // ~s
	Nor,        // ~(s | d)
	Nand,        // ~(s & d)
	NXor,        // ~(s ^ d) AKA equivilant
};

enum class ROPBlendFactor : uint8_t
{
	Zero,
	One, // One could be encoded as invert(Zero) but looks nicer this way
	SrcColour,
	DstColour,
	SrcAlpha,
	DstAlpha,
	ConstantColour,
	ConstantAlpha,
	Src1Colour,
	Src1Alpha,

	SrcAlphaSaturate, // no invert version

	InvertedBit = 0x80
};

// most blend ops can be inverted (AKA 1 - Op)
constexpr auto invert(ROPBlendFactor op_) -> ROPBlendFactor
{
	return (ROPBlendFactor) (((uint8_t) op_) | (uint8_t) ROPBlendFactor::InvertedBit);
}

enum class ROPBlendOps : uint8_t
{
	Add,
	Sub,
	ReverseSub,
	Min,
	Max,
};

enum class ColourComponents : uint8_t
{
	Red = Core::Bit(0),
	Green = Core::Bit(1),
	Blue = Core::Bit(2),
	Alpha = Core::Bit(3),

	All = 0xF
};

enum class CullMode : uint8_t
{
	None,
	Front,
	Back,
};

enum class FrontFace : uint8_t
{
	CounterClockWise,
	ClockWise
};

enum class FillMode : uint8_t
{
	Fill,
	Line,
	Point,
	Rect
};

enum class CompareOp : uint8_t
{
	Never,
	Less,
	Equal,
	LessOrEqual,
	Greater,
	NotEqual,
	GreaterOrEqual,
	Always,
};

enum class StencilOp : uint8_t
{
	Zero,            // 0
	Ref,            // d = reference
	Dst,            // d
	InvertDst,    // ~d
	IncAndClamp,    // clamp(d+1)
	DecAndClamp,    // clamp(d-1)
	IncAndWrap,        // d+1
	DecAndWrap        // d-1
};

enum class SampleCounts : uint8_t
{
	One = Core::Bit(1),
	Two = Core::Bit(2),
	Four = Core::Bit(3),
	Eight = Core::Bit(4),
	Sixteen = Core::Bit(5),
	ThirtyTwo = Core::Bit(6),
	SixtyFour = Core::Bit(7),
};

constexpr auto is_bitmask_enum(RenderPipelineStages) -> bool { return true; }

constexpr auto is_bitmask_enum(ComputePipelineStages) -> bool { return true; }

constexpr auto is_bitmask_enum(HostPipelineStages) -> bool { return true; }

constexpr auto is_bitmask_enum(DMAPipelineStages) -> bool { return true; }

constexpr auto is_bitmask_enum(ShaderType) -> bool { return true; }

constexpr auto is_bitmask_enum(EncoderFlag) -> bool { return true; }

constexpr auto is_bitmask_enum(CommandQueueFlavour) -> bool { return true; }

constexpr auto is_bitmask_enum(Usage) -> bool { return true; }

constexpr auto is_bitmask_enum(MemoryAccess) -> bool { return true; }

constexpr auto is_bitmask_enum(ColourComponents) -> bool { return true; }

constexpr auto is_bitmask_enum(SampleCounts) -> bool { return true; }

}

#endif //WYRD_RENDER_TYPES_H
