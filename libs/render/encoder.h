#pragma once
#ifndef WYRD_RENDER_ENCODER_H
#define WYRD_RENDER_ENCODER_H

#include "core/core.h"
#include "core/utils.h"
#include "math/vector_math_c.h"
#include "render/commandqueue.h"
#include "render/types.h"
#include "render/resources.h"

namespace Render {

struct IRenderEncoder;
struct IComputeEncoder;
struct Fence;
struct Semaphore;


// All Encoders are DMA encoders so there is no asDMAEncoder just use the encoder
// object
struct Encoder
{
public:

	using Ptr = std::shared_ptr<Encoder>;
	using WeakPtr = std::weak_ptr<Encoder>;

	auto canEncodeRenderCommands() -> bool const
	{
		return Core::bitmask::test_equal(encoderFlags, EncoderFlag::RenderEncoder);
	}

	auto canEncodeComputeCommands() -> bool const
	{
		return Core::bitmask::test_equal(encoderFlags, EncoderFlag::ComputeEncoder);
	}

	auto canSubmitToQueue() -> bool const { return !Core::bitmask::test_equal(encoderFlags, EncoderFlag::Callable); }

	auto isCallable() -> bool const { return Core::bitmask::test_equal(encoderFlags, EncoderFlag::Callable); }

	auto getFlags() -> EncoderFlag const { return encoderFlags; }

	virtual ~Encoder() = default;
	virtual auto asRenderEncoder() -> IRenderEncoder* = 0;
	virtual auto asComputeEncoder() -> IComputeEncoder* = 0;

	virtual auto begin(std::shared_ptr<Semaphore> const& semaphore_ = {}) -> void = 0;
	virtual auto end(std::shared_ptr<Semaphore> const& semaphore_ = {}) -> void = 0;
	virtual auto reset() -> void = 0;
	virtual auto copy(TexturePtr const& src_, TexturePtr const& dst_) -> void = 0;

	//	virtual auto pipelineBarrier(
	//			Render::DMAPipelineStages waitStages_,
	//			Render::DMAPipelineStages stallStages_ ) -> void = 0;

	//	virtual auto memoryBarrier() -> void = 0;
	//	virtual auto bufferBarrier() -> void = 0;
	virtual auto textureBarrier(MemoryAccess waitAccess_, MemoryAccess stallAccess_,
								TexturePtr const& texture_) -> void = 0;

	virtual auto textureBarrier(TexturePtr const& texture_) -> void = 0;

protected:
	Encoder(EncoderFlag encoderFlags_) : encoderFlags(encoderFlags_) {};
	EncoderFlag encoderFlags;
};

struct IRenderEncoder
{
	virtual auto
	clearTexture(TexturePtr const& texture_, std::array<float_t, 4> const& floats_) -> void = 0;
	virtual auto beginRenderPass(RenderPassPtr const& renderPass_, RenderTargetPtr const& renderTarget_) -> void = 0;
	virtual auto endRenderPass() -> void = 0;
	virtual auto blit(TexturePtr const& src_, TexturePtr const& dst_) -> void = 0;
	virtual auto bind(RenderPipelinePtr const& pipeline_) -> void = 0;
};

struct IComputeEncoder
{
	virtual auto clearTexture(std::shared_ptr<Texture> const& texture_,
							  std::array<float_t, 4> const& floats_) -> void = 0;
	virtual auto bind(ComputePipelinePtr const& pipeline_) -> void = 0;
};

struct EncoderPool
{
	using Ptr = std::shared_ptr<EncoderPool>;
	using WeakPtr = std::shared_ptr<EncoderPool>;

	virtual ~EncoderPool() = default;
	virtual auto allocateEncoder(
			EncoderFlag encoderFlags_ = Core::bitmask::zero<EncoderFlag>()) -> Render::Encoder::Ptr = 0;
	virtual auto reset() -> void = 0;
};

}
#endif //WYRD_ENCODER_H
