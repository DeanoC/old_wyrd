#include "core/core.h"
#include "vulkan/queue.h"
#include "vulkan/device.h"

namespace Vulkan {

Queue::Queue(VkQueue queue_, uint32_t flavour_) :
	queue(queue_), Render::Queue(flavour_)
{
}

}