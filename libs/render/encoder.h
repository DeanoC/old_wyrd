#pragma once
#ifndef WYRD_RENDER_ENCODER_H
#define WYRD_RENDER_ENCODER_H

#include <math/vector_math_c.h>
#include "core/core.h"
#include "core/utils.h"

#include "render/commandqueue.h"

namespace Render {

class Texture;

struct IRenderEncoder;
struct IComputeEncoder;
struct IBlitEncoder;
struct Fence;
struct Semaphore;

class Encoder
{
public:
	static constexpr uint32_t FlavourMask =
			CommandQueue::RenderFlavour |
			CommandQueue::ComputeFlavour |
			CommandQueue::BlitFlavour;
	static constexpr uint32_t CallableFlag = Core::Bit(4);
	static constexpr uint32_t PooledReset = Core::Bit(5);
	using Ptr = std::shared_ptr<Encoder>;
	using WeakPtr = std::weak_ptr<Encoder>;

	auto canEncodeRenderCommands() -> bool const { return !!(encoderFlags & CommandQueue::RenderFlavour); }
	auto canEncodeComputeCommands() -> bool const { return !!(encoderFlags & CommandQueue::ComputeFlavour); }
	auto canEncodeBlitCommands() -> bool const { return !!(encoderFlags & CommandQueue::BlitFlavour); }

	auto canSubmitToQueue() -> bool const { return !(encoderFlags & CallableFlag); }

	auto isCallable() -> bool const { return !!(encoderFlags & CallableFlag); }
	auto hasPooledReset() -> bool const { return !!(encoderFlags & PooledReset); }
	auto getFlavour() -> uint32_t const { return encoderFlags & FlavourMask; }
	auto getFlags() -> uint32_t const { return encoderFlags; }

	virtual ~Encoder() = default;
	virtual auto asRenderEncoder() -> IRenderEncoder* = 0;
	virtual auto asComputeEncoder() -> IComputeEncoder* = 0;
	virtual auto asBlitEncoder() -> IBlitEncoder* = 0;

	virtual auto begin(std::shared_ptr<Semaphore> const& semaphore_ = {}) -> void = 0;
	virtual auto end(std::shared_ptr<Semaphore> const& semaphore_ = {}) -> void = 0;
	virtual auto reset() -> void = 0;
protected:
	Encoder(uint32_t encoderFlags_) : encoderFlags(encoderFlags_) {};
	uint32_t encoderFlags;
};

struct IRenderEncoder
{
	virtual auto
	clearTexture(std::shared_ptr<Texture> const& texture_, std::array<float_t, 4> const& floats_) -> void = 0;
	virtual auto beginRenderPass() -> void = 0;
	virtual auto endRenderPass() -> void = 0;
};

struct IComputeEncoder
{
	virtual auto
	clearTexture(std::shared_ptr<Texture> const& texture_, std::array<float_t, 4> const& floats_) -> void = 0;
};

struct IBlitEncoder
{
};


struct EncoderPool
{
	using Ptr = std::shared_ptr<EncoderPool>;
	using WeakPtr = std::shared_ptr<EncoderPool>;

	virtual ~EncoderPool() = default;
	virtual auto allocateEncoder(uint32_t encoderFlags_) -> Render::Encoder::Ptr = 0;
	virtual auto reset() -> void = 0;
};

}
#endif //WYRD_ENCODER_H
