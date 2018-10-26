#include "core/core.h"
#include "vulkan/device.h"
#include "vulkan/commandqueue.h"
#include "vulkan/encoder.h"

namespace Vulkan {

CommandQueue::CommandQueue(QueueVkVTable *vtable_, VkQueue queue_, uint32_t familyIndex_, uint32_t flavour_) : vtable(
		vtable_), queue(queue_), familyIndex(familyIndex_), Render::CommandQueue(flavour_)
{
}

auto CommandQueue::submit(std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	assert(encoder_->getFlavour() == flavour);
	auto encoder = std::static_pointer_cast<Vulkan::Encoder>(encoder_);

	//	vkQueueSubmit(encoder->commandBuffer);

	assert(false);
}

auto CommandQueue::stallTillIdle() -> void
{
	vkQueueWaitIdle();
}

}