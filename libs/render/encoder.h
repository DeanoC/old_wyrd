#pragma once
#ifndef WYRD_RENDER_ENCODER_H
#define WYRD_RENDER_ENCODER_H

#include "core/core.h"
#include "core/utils.h"

#include "render/commandqueue.h"

namespace Render {

struct IRenderEncoder;
struct IComputeEncoder;
struct IBlitEncoder;

class Encoder
{
public:
	virtual ~Encoder() = default;

	using Ptr = std::shared_ptr<Encoder>;
	using WeakPtr = std::weak_ptr<Encoder>;

	auto canEncodeRenderCommands() -> bool const { return !!(encoderFlavour & CommandQueue::RenderFlavour); }

	auto canEncodeComputeCommands() -> bool const { return !!(encoderFlavour & CommandQueue::ComputeFlavour); }

	auto canEncodeBlitCommands() -> bool const { return !!(encoderFlavour & CommandQueue::BlitFlavour); }

	auto getFlavour() -> uint32_t const { return encoderFlavour; }

	virtual IRenderEncoder *asRenderEncoder() = 0;
	virtual IComputeEncoder *asComputeEncoder() = 0;
	virtual IBlitEncoder *asBlitEncoder() = 0;

protected:
	uint32_t encoderFlavour;
};

struct IRenderEncoder
{
};

struct IComputeEncoder
{
};

struct IBlitEncoder
{
};


struct EncoderPool
{
	using Ptr = std::shared_ptr<EncoderPool>;

	virtual auto allocatePrimaryEncoder(uint32_t encoderFlags_) -> Render::Encoder::Ptr = 0;

};

}
#endif //WYRD_ENCODER_H
