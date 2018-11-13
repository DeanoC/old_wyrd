#pragma once
#ifndef WYRD_VULKAN_COMMANDQUEUE_H
#define WYRD_VULKAN_COMMANDQUEUE_H

#include "core/core.h"
#include "vulkan/api.h"
#include "render/commandqueue.h"
#include <vector>

namespace Vulkan {

struct Device;
struct Encoder;
struct Fence;

struct CommandQueue : public Render::CommandQueue
{
public:
	CommandQueue(
			VkDevice device_,
			QueueVkVTable* vtable_,
			VkQueue queue_,
			uint32_t familyIndex_,
			Render::CommandQueueFlavour flavour_);
	~CommandQueue() final;

	friend class System;

	friend struct Device;
	friend struct Display;

	using Ptr = std::shared_ptr<CommandQueue>;
	using WeakPtr = std::weak_ptr<CommandQueue>;

	auto enqueue(std::shared_ptr<Render::Encoder> const& encoder_) -> void final;
	auto submit(std::shared_ptr<Render::Fence> const& fence_ = {}) -> void final;
	auto stallTillIdle() -> void final;


	auto getQueue() -> VkQueue { return queue; };
	auto getFamilyIndex() const -> uint32_t { return familyIndex; }
protected:
#define QUEUE_VK_FUNC(name) template<typename... Args> auto name(Args... args) { return vtable-> name(queue, args...); }
#define QUEUE_VK_FUNC_EXT(name, extension) QUEUE_VK_FUNC(name)

#include "functionlist.inl"

	VkDevice device;
	QueueVkVTable* vtable;
	VkQueue queue;
	uint32_t familyIndex;

	std::vector<std::shared_ptr<Encoder>> enqueuedEncoders;

};

}

#endif //WYRD_VULKAN_COMMANDQUEUE_H
