#include "core/core.h"
#include "vulkan/device.h"
#include "vulkan/commandqueue.h"
#include "vulkan/encoder.h"
#include "vulkan/semaphore.h"
#include "vulkan/fence.h"

namespace Vulkan {

CommandQueue::CommandQueue(
		VkDevice device_,
		QueueVkVTable* vtable_,
		VkQueue queue_,
		uint32_t familyIndex_,
		Render::CommandQueueFlavour flavour_) :
		device(device_),
		vtable(vtable_),
		queue(queue_),
		familyIndex(familyIndex_),
		Render::CommandQueue(flavour_)
{
}

CommandQueue::~CommandQueue()
{
}

auto CommandQueue::enqueue(std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	auto encoder = std::static_pointer_cast<Vulkan::Encoder>(encoder_);

	enqueuedEncoders.push_back(encoder);
}

auto CommandQueue::submit(std::shared_ptr<Render::Fence> const& fence_) -> void
{
	// Should we signal semaphores on an empty submit...
	if(enqueuedEncoders.empty()) return;

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> cbBeginSemaphores;
	std::vector<VkSemaphore> cbEndSemaphores;
	std::vector<VkPipelineStageFlags> cbBeginPipelineFlags;

	commandBuffers.reserve(enqueuedEncoders.size());
	cbBeginSemaphores.reserve(enqueuedEncoders.size());
	cbEndSemaphores.reserve(enqueuedEncoders.size());
	cbBeginPipelineFlags.reserve(enqueuedEncoders.size());

	for(Vulkan::Encoder::Ptr encoder : enqueuedEncoders)
	{
		commandBuffers.push_back(encoder->commandBuffer);
		if(encoder->beginSemaphore)
		{
			cbBeginSemaphores.push_back(encoder->beginSemaphore->vulkanSemaphore);
			// TODO pipeline flags (this is very conservative but should work in all cases...)
			cbBeginPipelineFlags.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		}
		if(encoder->endSemaphore) cbEndSemaphores.push_back(encoder->endSemaphore->vulkanSemaphore);
	}

	submitInfo.commandBufferCount = (uint32_t) commandBuffers.size();
	submitInfo.pCommandBuffers = commandBuffers.data();

	submitInfo.waitSemaphoreCount = (uint32_t) cbBeginSemaphores.size();
	submitInfo.pWaitSemaphores = cbBeginSemaphores.data();
	submitInfo.pWaitDstStageMask = cbBeginPipelineFlags.data();

	submitInfo.signalSemaphoreCount = (uint32_t) cbEndSemaphores.size();
	submitInfo.pSignalSemaphores = cbEndSemaphores.data();

	VkFence vulkanFence = VK_NULL_HANDLE;
	if(fence_)
	{
		auto fence = std::static_pointer_cast<Fence>(fence_);
		vulkanFence = fence->vulkanFence;
	}
	CHKED(vkQueueSubmit(1, &submitInfo, vulkanFence));

	enqueuedEncoders.clear();
}

auto CommandQueue::stallTillIdle() -> void
{
	vkQueueWaitIdle();
}

}