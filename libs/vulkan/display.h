#pragma once
#ifndef WYRD_VULKAN_DISPLAY_H
#define WYRD_VULKAN_DISPLAY_H

#include "core/core.h"
#include "render/display.h"
#include "vulkan/api.h"
#include "vulkan/device.h"
#include "GLFW/glfw3.h"
#include <vector>


namespace Vulkan {
class Display : public Render::Display
{
public:
	Display(uint32_t width_, uint32_t height_,
			Vulkan::Device::Ptr device_,
			GLFWwindow *window_,
			VkSurfaceKHR surface_)
			: Render::Display{width_, height_},
			window(window_),
			weakDevice(device_),
			surface(surface_) {}

	auto createSwapChain() -> void;

	auto present() -> bool final;

private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat() const;
	VkPresentModeKHR chooseSwapPresentMode() const;
	VkExtent2D chooseSwapExtent() const;

	Device::WeakPtr weakDevice;
	GLFWwindow *window;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchainKHR;
	VkSemaphore imageAvailable;
	VkSemaphore presentComplete;
	std::vector<VkImage> images;

	VkSurfaceCapabilitiesKHR capabilities;
	VkSwapchainCreateInfoKHR createInfo;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
}

#endif //WYRD_VULKAN_DISPLAY_H
