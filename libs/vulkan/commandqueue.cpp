#include "core/core.h"
#include "vulkan/commandqueue.h"
#include "vulkan/device.h"

namespace Vulkan {

CommandQueue::CommandQueue(VkQueue queue_, uint32_t flavour_) :
	queue(queue_), Render::CommandQueue(flavour_)
{
}

}