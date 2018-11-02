#pragma once
#ifndef WYRD_VULKAN_DISPLAY_H
#define WYRD_VULKAN_DISPLAY_H

#include "core/core.h"
#include "render/display.h"
#include "vulkan/api.h"
#include "vulkan/device.h"
#include "vulkan/semaphore.h"
#include "encoder.h"
#include <vector>

namespace Shell { struct PresentableWindow; }

namespace Vulkan {

struct Display : public Render::Display
{
public:
	Display(uint32_t width_, uint32_t height_,
			Vulkan::Device::Ptr device_,
			Shell::PresentableWindow* window_,
			VkSurfaceKHR surface_);

	~Display() final;

	auto createSwapChain() -> void;

	auto present(std::shared_ptr<Render::Texture> const& src_) -> void final;

private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat() const;
	VkPresentModeKHR chooseSwapPresentMode() const;
	VkExtent2D chooseSwapExtent() const;

	Device::WeakPtr weakDevice;
	Shell::PresentableWindow* window;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchainKHR;
	Semaphore::Ptr imageAvailable;
	Semaphore::Ptr presentComplete;
	std::vector<VkImage> images;
	uint32_t lastImageIndex = ~0;

	VkSurfaceCapabilitiesKHR capabilities;
	VkSwapchainCreateInfoKHR createInfo;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	EncoderPool::Ptr blitterPool;
};
}

#endif //WYRD_VULKAN_DISPLAY_H
