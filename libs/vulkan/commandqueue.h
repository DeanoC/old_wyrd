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
	CommandQueue(QueueVkVTable *vtable_, VkQueue queue_, uint32_t familyIndex_, uint32_t flavour_);

	friend class System;
	friend class Device;

	friend class Display;

	using Ptr = std::shared_ptr<CommandQueue>;
	using WeakPtr = std::weak_ptr<CommandQueue>;

	auto getQueue() -> VkQueue { return queue; };

	auto getFamilyIndex() const -> uint32_t { return familyIndex; }

	auto submit(std::shared_ptr<Render::Encoder> const& encoder_) -> void final;

	auto stallTillIdle() -> void final;

protected:
#define QUEUE_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(queue, args...); }
#define QUEUE_VK_FUNC_EXT(name, extension) QUEUE_VK_FUNC(name)
#include "functionlist.inl"

	QueueVkVTable *vtable;
	VkQueue queue;
	uint32_t familyIndex;
};

}

#endif //WYRD_VULKAN_COMMANDQUEUE_H
