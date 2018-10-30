#pragma once
#ifndef WYRD_VULKAN_DISPLAY_H
#define WYRD_VULKAN_DISPLAY_H

#include "core/core.h"
#include "render/display.h"
#include "vulkan/api.h"
#include "vulkan/device.h"
#include "vulkan/semaphore.h"
#include "GLFW/glfw3.h"
#include <vector>


namespace Vulkan {
struct Display : public Render::Display
{
public:
	Display(uint32_t width_, uint32_t height_,
			Vulkan::Device::Ptr device_,
			GLFWwindow *window_,
			VkSurfaceKHR surface_);

	~Display() final;

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
	Semaphore::Ptr imageAvailable;
	Semaphore::Ptr presentComplete;
	std::vector<VkImage> images;

	VkSurfaceCapabilitiesKHR capabilities;
	VkSwapchainCreateInfoKHR createInfo;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
}

#endif //WYRD_VULKAN_DISPLAY_H
