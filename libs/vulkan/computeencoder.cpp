#include "core/core.h"
#include "render/commandqueue.h"
#include "vulkan/texture.h"
#include "vulkan/semaphore.h"
#include "vulkan/computeencoder.h"

namespace Vulkan {

auto ComputeEncoder::clearTexture(std::shared_ptr<Render::Texture> const& texture_,
								  std::array<float_t, 4> const& floats_) -> void
{
	Texture::Ptr texture = std::static_pointer_cast<Texture>(texture);

	// compute encoders can't clear depth/stencil images
	assert(texture->entireRange.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT);
	VkClearColorValue colour;
	std::memcpy(&colour.float32, floats_.data(), floats_.size() * sizeof(float));
	vkCmdClearColorImage(texture->image, VK_IMAGE_LAYOUT_GENERAL, &colour, 1, &texture->entireRange);
}

}