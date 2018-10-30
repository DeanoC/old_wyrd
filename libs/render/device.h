#pragma once
#ifndef WYRD_RENDER_DEVICE_H
#define WYRD_RENDER_DEVICE_H

#include "core/core.h"
#include "render/display.h"

namespace Render
{
struct CommandQueue;
struct EncoderPool;
struct Fence;
struct Semaphore;

struct Device
{
	virtual ~Device() = default;
	virtual auto getDisplay() const -> std::shared_ptr<Display> = 0;

	virtual auto makeEncoderPool(bool frameLifetime_, uint32_t queueFlavour_) -> std::shared_ptr<EncoderPool> = 0;
	virtual auto makeFence() -> std::shared_ptr<Fence> = 0;
	virtual auto makeSemaphore() -> std::shared_ptr<Semaphore> = 0;

	virtual auto getMainRenderQueue() -> std::shared_ptr<CommandQueue> = 0;
	virtual auto getMainComputeQueue() -> std::shared_ptr<CommandQueue> = 0;
	virtual auto getMainBlitQueue() -> std::shared_ptr<CommandQueue> = 0;
	virtual auto getMainPresentQueue() -> std::shared_ptr<CommandQueue> = 0;
};

}

#endif //WYRD_GPU_DEVICE_H
