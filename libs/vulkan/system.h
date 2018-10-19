#pragma once
#ifndef WYRD_VULKAN_SYSTEM_H
#define WYRD_VULKAN_SYSTEM_H

#include "core/core.h"
#include "vulkan/api.h"
#include "render/stable.h"
#include "vulkan/device.h"
#include "vulkan/display.h"
#include <vector>
#include <set>
#include <array>

namespace Vulkan
{
class System : public Render::Stable {
public:
	inline static System* Global = nullptr;

	auto Init(std::string const& appName_, std::vector<std::string> const& desiredInstanceExtensions_) -> bool final;
	auto Finish() -> void final;

	auto getGpuCount() const -> uint32_t final { return (uint32_t) physicalDevices.size(); }
	auto canGpuPresent(uint32_t index_) const -> bool final;
	auto isGpuLowPower(uint32_t index_) const -> bool final;
	auto createGpuDevice(uint32_t index_, Render::DeviceConfig const& config_) -> std::shared_ptr<Render::Device> final;

private:
	auto createGpuDevice(uint32_t deviceIndex_, bool render_, VkSurfaceKHR surface_, uint32_t minQueues_, std::vector<char const*> const& requiredExtensions) -> Vulkan::Device::Ptr;

	auto addDesiredInstanceExtensions(std::string const& desired_)
	{
		desiredInstanceExtensions.insert( desired_ );
	}

	// returns graphics, compute, transfer. The can be the same index
	auto findQueueFamily(uint32_t deviceIndex_, uint32_t minQueues_) -> std::tuple<uint32_t,uint32_t,uint32_t>;

	VkInstance instance;

	std::vector<VkExtensionProperties> instanceExtensions;
	std::vector<VkLayerProperties> instanceLayers;

	std::vector<VkPhysicalDevice> physicalDevices;
	std::vector<std::vector<VkExtensionProperties>> deviceExtensions;
	std::vector<VkPhysicalDeviceProperties> deviceProperties;
	std::vector<VkPhysicalDeviceFeatures> deviceFeatures;
	std::vector<Device::QueueFamilies> deviceQueueFamilies;

	std::set<std::string> desiredInstanceExtensions;

	std::vector<std::shared_ptr<Vulkan::Display>> activeDisplays;

	static constexpr bool enableValidationLayers = true;
	inline static std::array<const char*, 1> validationLayers = {
			"VK_LAYER_LUNARG_standard_validation"
	};
};

}

#endif //WYRD_SYSTEM_H
