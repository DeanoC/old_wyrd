#include "core/core.h"
#include "vulkan/device.h"
#include "vulkan/display.h"
#include "vulkan/commandqueue.h"
#include <algorithm>

namespace Vulkan {
constexpr auto PresentTimeOut = 2000000000;

Display::Display(
		uint32_t width_,
		uint32_t height_,
		Vulkan::Device::Ptr device_,
		Shell::PresentableWindow* window_,
		VkSurfaceKHR surface_) :
		Render::Display{width_, height_},
		window(window_),
		weakDevice(device_),
		surface(surface_)
{
}

Display::~Display()
{
	auto device = weakDevice.lock();
	if(device)
	{
		device->destroySwapchain(swapchainKHR);
	}
}
auto Display::createSwapChain() -> void
{
	auto device = weakDevice.lock();

	CHKED( vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			device->getPhysicalDevice(), surface, &capabilities));

	uint32_t formatCount;
	CHKED(vkGetPhysicalDeviceSurfaceFormatsKHR(
			device->getPhysicalDevice(), surface, &formatCount, nullptr));

	if (formatCount != 0) {
		formats.resize(formatCount);
		CHKED(vkGetPhysicalDeviceSurfaceFormatsKHR(
				device->getPhysicalDevice(), surface, &formatCount, formats.data()));
	}
	uint32_t presentModeCount;
	CHKED(vkGetPhysicalDeviceSurfacePresentModesKHR(
			device->getPhysicalDevice(), surface, &presentModeCount, nullptr));
	if (presentModeCount != 0) {
		presentModes.resize(presentModeCount);
		CHKED(vkGetPhysicalDeviceSurfacePresentModesKHR(
				device->getPhysicalDevice(), surface, &presentModeCount, presentModes.data()));
	}

	auto swapFormat = chooseSwapSurfaceFormat();
	uint32_t imageCount = capabilities.minImageCount + 1;
	if(capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = swapFormat.format;
	createInfo.imageColorSpace = swapFormat.colorSpace;
	createInfo.imageExtent = chooseSwapExtent();
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
							VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = chooseSwapPresentMode();
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	swapchainKHR = device->createSwapchain(createInfo);

	uint32_t swapchainImageCount = 0;
	CHKED(device->vkGetSwapchainImagesKHR(swapchainKHR, &swapchainImageCount, nullptr));
	images.resize(swapchainImageCount);
	CHKED(device->vkGetSwapchainImagesKHR(swapchainKHR, &swapchainImageCount, images.data()));

	imageAvailable = std::static_pointer_cast<Semaphore>(device->makeSemaphore());
	presentComplete = std::static_pointer_cast<Semaphore>(device->makeSemaphore());

	blitterPool = device->makeEncoderPool(true, Render::CommandQueueFlavour::Render);
}

auto Display::present(std::shared_ptr<Render::Texture> const& src_) -> void
{
	auto device = weakDevice.lock();

	uint32_t imageIndex = ~0;

	VkFence nullFence = VK_NULL_HANDLE;
	VkResult result = device->vkAcquireNextImageKHR(
			swapchainKHR,
			PresentTimeOut,
			imageAvailable->vulkanSemaphore, nullFence, &imageIndex);

	switch( result )
	{
		case VK_SUCCESS:
		case VK_SUBOPTIMAL_KHR:
			break;

		default:
			CHKED(result);
			return;
	}

	auto q = device->getGeneralQueue();
	auto enc = blitterPool->allocateEncoder(Render::EncoderFlag::RenderEncoder);
	assert(enc->canEncodeRenderCommands());

	enc->begin(imageAvailable);
	auto* renc = (RenderEncoder*) enc->asRenderEncoder();
	/*	VkClearColorValue clear;
		clear.float32[0] = 0.0f;
		clear.float32[1] = 255.0f;
		clear.float32[2] = 0.0f;
		clear.float32[3] = 255.0f;
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.layerCount = 1;
		range.levelCount = 1;
		range.baseMipLevel = 0;
		range.baseArrayLayer = 0;

		renc->vkCmdClearColorImage(images[imageIndex],
								   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &clear, 1, &range);
	*/
	renc->resolveForDisplay(src_, width, height, images[imageIndex]);

	enc->end(presentComplete);
	q->enqueue(enc);
	q->submit();

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchainKHR;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &presentComplete->vulkanSemaphore;
	presentInfo.pResults = nullptr;

	auto pq = std::static_pointer_cast<Vulkan::CommandQueue>(device->getPresentQueue());
	CHKED(pq->vkQueuePresentKHR(&presentInfo));
}

VkSurfaceFormatKHR Display::chooseSwapSurfaceFormat() const
{
	if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	for(const auto& availableFormat : formats)
	{
		if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
		   availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return formats[0];
}

VkPresentModeKHR Display::chooseSwapPresentMode() const
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for(const auto& availablePresentMode : presentModes)
	{
		if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		} else if(availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

VkExtent2D Display::chooseSwapExtent() const
{
	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	} else
	{
		VkExtent2D actualExtent = {width, height};

		actualExtent.width = std::max(capabilities.minImageExtent.width,
									  std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height,
									   std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

} // end namespace Vulkan

