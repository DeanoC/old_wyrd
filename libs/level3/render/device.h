#pragma once
#ifndef WYRD_RENDER_DEVICE_H
#define WYRD_RENDER_DEVICE_H

#include "core/core.h"
#include "render/display.h"
#include "render/types.h"

namespace Render
{
struct CommandQueue;
struct EncoderPool;
struct Fence;
struct Semaphore;
struct RenderTarget;

struct Device
{
	using Ptr = std::shared_ptr<Device>;
	using ConstPtr = std::shared_ptr<Device const>;
	using WeakPtr = std::weak_ptr<Device>;
	using ConstWeakPtr = std::weak_ptr<Device const>;

	virtual ~Device() = default;
	virtual auto getDisplay() const -> std::weak_ptr<Display> = 0;
	virtual auto houseKeepTick() -> void = 0;

	virtual auto makeEncoderPool(bool frameLifetime_, CommandQueueFlavour flavour_) -> std::shared_ptr<EncoderPool> = 0;
	virtual auto makeFence() -> std::shared_ptr<Fence> = 0;
	virtual auto makeSemaphore() -> std::shared_ptr<Semaphore> = 0;

	virtual auto getGeneralQueue() -> std::shared_ptr<CommandQueue> = 0;
	virtual auto getRenderSpecificQueue() -> std::shared_ptr<CommandQueue> = 0;
	virtual auto getComputeSpecificQueue() -> std::shared_ptr<CommandQueue> = 0;
	virtual auto getDMASpecificQueue() -> std::shared_ptr<CommandQueue> = 0;
	virtual auto getPresentQueue() -> std::shared_ptr<CommandQueue> = 0;
};

}

#endif //WYRD_GPU_DEVICE_H
