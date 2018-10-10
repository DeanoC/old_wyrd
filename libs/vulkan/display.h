#pragma once
#ifndef WYRD_VULKAN_DISPLAY_H
#define WYRD_VULKAN_DISPLAY_H

#include "core/core.h"
#include "render/display.h"
#include "vulkan/api.h"
#include "GLFW/glfw3.h"
#include <vector>


namespace Vulkan
{
class Display : public Render::Display
{
public:
	friend class Device;
	friend class System;

	Display(uint32_t width_,
			uint32_t height_,
			GLFWwindow* window_,
			VkSurfaceKHR surface_) : window(window_), surface(surface_)
	{
		width = width_;
		height = height_;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat() const
	{
		if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		{
			return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
		}

		for (const auto& availableFormat : formats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return formats[0];
	}
	VkPresentModeKHR chooseSwapPresentMode() const
	{
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& availablePresentMode : presentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			} else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				bestMode = availablePresentMode;
			}
		}

		return bestMode;
	}
	VkExtent2D chooseSwapExtent() const
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D actualExtent = {width, height};

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}
private:
	GLFWwindow* window;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchainKHR;
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
}

#endif //WYRD_VULKAN_DISPLAY_H
