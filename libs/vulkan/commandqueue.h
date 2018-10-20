#pragma once
#ifndef WYRD_VULKAN_COMMANDQUEUE_H
#define WYRD_VULKAN_COMMANDQUEUE_H

#include "core/core.h"
#include "vulkan/api.h"
#include "render/commandqueue.h"

namespace Vulkan {

class Device;
class Display;

class CommandQueue : public Render::CommandQueue
{
public:
	CommandQueue(VkQueue queue_, uint32_t flavour_);

	friend class System;
	friend class Device;
	using Ptr = std::shared_ptr<CommandQueue>;
	using WeakPtr = std::weak_ptr<CommandQueue>;

	auto getQueue() -> VkQueue { return queue; };

protected:

	VkQueue queue;
};

}

#endif //WYRD_VULKAN_COMMANDQUEUE_H
