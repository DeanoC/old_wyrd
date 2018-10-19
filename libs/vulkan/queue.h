#pragma once
#ifndef WYRD_VULKAN_QUEUE_H
#define WYRD_VULKAN_QUEUE_H

#include "core/core.h"
#include "vulkan/api.h"
#include "render/queue.h"

namespace Vulkan {

class Device;
class Display;

class Queue : public Render::Queue
{
public:
	Queue(VkQueue queue_, uint32_t flavour_);

	friend class System;
	friend class Device;
	using Ptr = std::shared_ptr<Queue>;
	using WeakPtr = std::weak_ptr<Queue>;

	auto getQueue() -> VkQueue { return queue; };

protected:

	VkQueue queue;
};

}

#endif //WYRD_VULKAN_QUEUE_H
