#pragma once
#ifndef WYRD_RENDER_DEVICE_H
#define WYRD_RENDER_DEVICE_H

#include "core/core.h"
#include "render/display.h"

namespace Render
{
class CommandQueue;

struct EncoderPool;

//opaque type
using Semaphore = struct Semaphore_t *;


struct Device
{
	virtual ~Device() = default;
	virtual auto getDisplay() const -> std::shared_ptr<Display> = 0;

	virtual auto createSemaphore() -> Semaphore = 0;
	virtual auto destroySemaphore(Semaphore semaphore_) -> void = 0;

	virtual auto createEncoderPool(bool frameLifetime_, uint32_t queueFlavour_) -> std::shared_ptr<EncoderPool> = 0;
	virtual auto destroyEncoderPool() -> void = 0;

	virtual auto getMainRenderQueue() -> std::shared_ptr<CommandQueue> = 0;
	virtual auto getMainComputeQueue() -> std::shared_ptr<CommandQueue> = 0;
	virtual auto getMainBlitQueue() -> std::shared_ptr<CommandQueue> = 0;
	virtual auto getMainPresentQueue() -> std::shared_ptr<CommandQueue> = 0;
};

}

#endif //WYRD_GPU_DEVICE_H
