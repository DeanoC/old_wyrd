#pragma once
#ifndef WYRD_RENDER_QUEUE_H
#define WYRD_RENDER_QUEUE_H

#include "core/core.h"
#include "core/utils.h"

namespace Render {
class Display;

class Queue
{
public:
	static constexpr uint32_t RenderFlavour = Core::Bit(0);
	static constexpr uint32_t ComputeFlavour = Core::Bit(1);
	static constexpr uint32_t BlitFlavour = Core::Bit(2);
	static constexpr uint32_t PresentFlavour = Core::Bit(3);

	using Ptr = std::shared_ptr<Queue>;
	using WeakPtr = std::weak_ptr<Queue>;

	auto isRenderFlavour() const -> bool { return flavour & RenderFlavour; }
	auto isComputeFlavour() const -> bool { return flavour & ComputeFlavour; }
	auto isBlitFlavour() const -> bool { return flavour & BlitFlavour; }
	auto isPresentFlavour() const -> bool { return flavour &PresentFlavour; }
	auto getFlavour() const -> uint32_t { return flavour; }

protected:
	Queue(uint32_t flavour_) : flavour(flavour_) {}
	virtual ~Queue() = default;
	uint32_t const flavour = 0;
};

}

#endif //WYRD_QUEUE_H
