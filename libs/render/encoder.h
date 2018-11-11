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
	virtual auto copy(TextureConstPtr const& src_, TextureConstPtr const& dst_) -> void = 0;
	virtual auto fill(uint32_t fill_, BufferConstPtr const& dst_) -> void = 0;

	virtual auto textureBarrier(MemoryAccess waitAccess_, MemoryAccess stallAccess_,
								TextureConstPtr const& texture_) -> void = 0;

	virtual auto textureBarrier(TextureConstPtr const& texture_) -> void = 0;

protected:
	Encoder(EncoderFlag encoderFlags_) : encoderFlags(encoderFlags_) {};
	EncoderFlag encoderFlags;
};

struct IRenderEncoder
{
	virtual auto clearTexture(TextureConstPtr const& texture_, std::array<float_t, 4> const& floats_) -> void = 0;
	virtual auto beginRenderPass(RenderPassConstPtr const& renderPass_,
								 RenderTargetConstPtr const& renderTarget_) -> void = 0;
	virtual auto endRenderPass() -> void = 0;
	virtual auto blit(TextureConstPtr const& src_, TextureConstPtr const& dst_) -> void = 0;
	virtual auto bind(RenderPipelineConstPtr const& pipeline_) -> void = 0;
	virtual auto bindVertexBuffer(Render::BufferConstPtr const& buffer_, uint64_t offset_ = 0,
								  uint32_t bindingIndex = 0) -> void = 0;
	virtual auto bindIndexBuffer(Render::BufferConstPtr const& buffer_, uint8_t bitSize_ = 16u,
								 uint64_t offset_ = 0) -> void = 0;
	virtual auto draw(uint32_t vertexCount_, uint32_t vertexOffset_ = 0, uint32_t instanceCount_ = 1,
					  uint32_t instanceOffset_ = 0) -> void = 0;
	virtual auto drawIndexed(uint32_t indexCount_, uint32_t indexOffset_ = 0, uint32_t vertexOffset = 0,
							 uint32_t instanceCount_ = 1, uint32_t instanceOffset_ = 0) -> void = 0;

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
