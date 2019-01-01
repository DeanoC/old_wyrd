#pragma once
#ifndef WYRD_RENDER_COMMANDQUEUE_H
#define WYRD_RENDER_COMMANDQUEUE_H

#include "core/core.h"
#include "core/utils.h"
#include "render/types.h"

namespace Render {
struct Display;
struct Encoder;
struct Fence;

struct CommandQueue
{
	using Ptr = std::shared_ptr<CommandQueue>;
	using WeakPtr = std::weak_ptr<CommandQueue>;

	auto isRenderFlavour() const -> bool
	{
		return bool(flavour & CommandQueueFlavour::Render);
	}

	auto isComputeFlavour() const -> bool
	{
		return bool(flavour & CommandQueueFlavour::Compute);
	}

	auto isDMAFlavour() const -> bool
	{
		return bool(flavour & CommandQueueFlavour::DMA);
	}

	auto isPresentFlavour() const -> bool
	{
		return bool(flavour & CommandQueueFlavour::Present);
	}

	auto getFlavour() const -> CommandQueueFlavour { return flavour; }

	virtual auto enqueue(std::shared_ptr<Render::Encoder> const& encoder_) -> void = 0;
	virtual auto submit(std::shared_ptr<Render::Fence> const& fence_ = {}) -> void = 0;
	virtual auto stallTillIdle() -> void = 0;

protected:
	CommandQueue(CommandQueueFlavour flavour_) : flavour(flavour_) {}
	virtual ~CommandQueue() = default;
	CommandQueueFlavour const flavour = Core::zero<CommandQueueFlavour>();
};

}

#endif //WYRD_RENDER_COMMANDQUEUE_H
