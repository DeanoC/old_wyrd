#include "core/core.h"
#include <string_view>

#define GLFW_INCLUDE_VULKAN
#include "vulkan/glfw/include/GLFW/glfw3.h"
#include "system.h"
#include <array>

#define CHK_F(x, call) result = (x); if(result != VK_SUCCESS) { LOG_S(ERROR) << "Vulkan Error: " << #call << " " << getVulkanResultString(result); return false; }

#define VULKAN_FUNC( name, level ) \
   name = (PFN_##name)glfwGetInstanceProcAddress( nullptr, #name );       \
   if( name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; \
     return false;                                                   \
   }
#define GLOBAL_VK_FUNC( name ) VULKAN_FUNC(name, global)
#define INSTANCE_VK_FUNC( name ) VULKAN_FUNC(name, instance)

namespace Vulkan
{
// borrow glfw result to string function
std::string_view const getVulkanResultString(VkResult result)
{
	using namespace std::string_view_literals;
	switch (result)
	{
		case VK_SUCCESS:
			return "Success"sv;
		case VK_NOT_READY:
			return "A fence or query has not yet completed"sv;
		case VK_TIMEOUT:
			return "A wait operation has not completed in the specified time"sv;
		case VK_EVENT_SET:
			return "An event is signaled"sv;
		case VK_EVENT_RESET:
			return "An event is unsignaled"sv;
		case VK_INCOMPLETE:
			return "A return array was too small for the result"sv;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return "A host memory allocation has failed"sv;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return "A device memory allocation has failed"sv;
		case VK_ERROR_INITIALIZATION_FAILED:
			return "Initialization of an object could not be completed for implementation-specific reasons"sv;
		case VK_ERROR_DEVICE_LOST:
			return "The logical or physical device has been lost"sv;
		case VK_ERROR_MEMORY_MAP_FAILED:
			return "Mapping of a memory object has failed"sv;
		case VK_ERROR_LAYER_NOT_PRESENT:
			return "A requested layer is not present or could not be loaded"sv;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			return "A requested extension is not supported"sv;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return "A requested feature is not supported"sv;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible"sv;
		case VK_ERROR_TOO_MANY_OBJECTS:
			return "Too many objects of the type have already been created"sv;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return "A requested format is not supported on this device"sv;
		case VK_ERROR_SURFACE_LOST_KHR:
			return "A surface is no longer available"sv;
		case VK_SUBOPTIMAL_KHR:
			return "A swapchain no longer matches the surface properties exactly, but can still be used"sv;
		case VK_ERROR_OUT_OF_DATE_KHR:
			return "A surface has changed in such a way that it is no longer compatible with the swapchain"sv;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return "The display used by a swapchain does not use the same presentable image layout"sv;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API"sv;
		case VK_ERROR_VALIDATION_FAILED_EXT:
			return "A validation layer found an error"sv;
		default:
			return "ERROR: UNKNOWN VULKAN ERROR"sv;
	}
}

bool System::Init(std::string const& appName_, std::vector<std::string> const& desiredInstanceExtensions_)
{
//	assert(Global == nullptr);
	Global = this;
	glfwInit();

	if (!glfwVulkanSupported())
	{
		LOG_S(INFO) << "No Vulkan";
		return false;
	}

#include "functionlist.inl"

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	for( auto const& ext : desiredInstanceExtensions_)
	{
		addDesiredInstanceExtensions(ext);
	}

	uint32_t instanceExtensionsCount = 0;
	VkResult result = VK_SUCCESS;
	result = vkEnumerateInstanceExtensionProperties( nullptr,
													 &instanceExtensionsCount, nullptr );
	if(result != VK_SUCCESS) { LOG_S(ERROR) << "Vulkan Error: " << getVulkanResultString(result); return false; }

	uint32_t glfwRequiredInstanceExtensionCount;
	const char** extensions = glfwGetRequiredInstanceExtensions(&glfwRequiredInstanceExtensionCount);
	if(instanceExtensionsCount < glfwRequiredInstanceExtensionCount)
	{
		LOG_S(ERROR) << "Cannot get extensions required by glfw";
		return false;
	}
	for(auto i = 0u; i < glfwRequiredInstanceExtensionCount; ++i)
	{
		addDesiredInstanceExtensions(extensions[i]);
	}

	instanceExtensions.resize( instanceExtensionsCount );
	result = vkEnumerateInstanceExtensionProperties(
			nullptr, &instanceExtensionsCount, &instanceExtensions[0] );
	CHK_F(result, vkEnumerateInstanceExtensionProperties)

	std::vector<char const*> desiredInstanceExt;
	desiredInstanceExt.reserve(desiredInstanceExtensions.size());
	// this is a slow repeated search, if extension get to many apply an accelerator
	for(auto const& desired : desiredInstanceExtensions)
	{
		auto const it = std::find_if(
				instanceExtensions.cbegin(), instanceExtensions.cend(),
				[&desired](VkExtensionProperties const& lhs) -> bool
				{
					if(std::string(lhs.extensionName) == desired)
						return true;
					else
						return false;
				}) ;
		if(it == instanceExtensions.end())
		{
			LOG_S(ERROR) << "The extension " << desired << "is required by not available";
			return false;
		} else
		{
			desiredInstanceExt.push_back(desired.c_str());
		}
	}

	uint32_t instanceLayerCount;
	result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
	CHK_F(result, vkEnumerateInstanceLayerProperties);
	instanceLayers.resize(instanceLayerCount);
	result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers.data());
	CHK_F(result, vkEnumerateInstanceLayerProperties);

#define INSTANCE_VK_FUNC_EXT( name, extension )            \
	for( auto const& ext : desiredInstanceExtensions ) { \
		if( ext == std::string( extension ) ) {\
			VULKAN_FUNC(name, instance extension) \
		} \
	}
#include "functionlist.inl"

	VkApplicationInfo application_info = {
			VK_STRUCTURE_TYPE_APPLICATION_INFO,
			nullptr,
			appName_.c_str(),
			VK_MAKE_VERSION( 1, 0, 0 ),
			"Wyrd",
			VK_MAKE_VERSION( 1, 0, 0 ),
			0
	};
	VkInstanceCreateInfo instance_create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			nullptr,
			0,
			&application_info,
			0,
			nullptr,
			static_cast<uint32_t>(desiredInstanceExt.size()),
			desiredInstanceExt.size() > 0 ? desiredInstanceExt.data() : nullptr
	};

	result = vkCreateInstance( &instance_create_info, nullptr, &instance );
	CHK_F(result, vkCreateInstance);

	uint32_t physDeviceCount;
	result = vkEnumeratePhysicalDevices(instance, &physDeviceCount, nullptr);
	CHK_F(result, vkEnumeratePhysicalDevices);
	physicalDevices.resize(physDeviceCount);
	deviceExtensions.resize(physDeviceCount);
	deviceProperties.resize(physDeviceCount);
	deviceFeatures.resize(physDeviceCount);
	deviceQueueFamilies.resize(physDeviceCount);

	result = vkEnumeratePhysicalDevices(instance, &physDeviceCount, physicalDevices.data());
	CHK_F(result, vkEnumeratePhysicalDevices);

	for(auto i = 0u; i < physDeviceCount; ++i)
	{
		uint32_t deviceExtCount;
		result = vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &deviceExtCount, nullptr);
		CHK_F(result, vkEnumerateDeviceExtensionProperties);
		deviceExtensions[i].resize(deviceExtCount);
		result = vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &deviceExtCount, deviceExtensions[i].data());
		CHK_F(result, vkEnumerateDeviceExtensionProperties);
		vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties[i]);
		vkGetPhysicalDeviceFeatures(physicalDevices[i], &deviceFeatures[i]);
		uint32_t qFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &qFamilyCount, nullptr);
		deviceQueueFamilies[i].resize(qFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &qFamilyCount, deviceQueueFamilies[i].data());

	}


	return true;
}

void System::Finish()
{
	if(instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(instance, nullptr);
		instance = VK_NULL_HANDLE;
	}
	Global = nullptr;
}

auto System::canGpuPresent(uint32_t index_) const -> bool
{
	assert(index_ < physicalDevices.size());
	auto numQFamilies = deviceQueueFamilies[index_].size();
	for(auto i = 0u; i < numQFamilies; ++i)
	{
		bool yes = glfwGetPhysicalDevicePresentationSupport(instance, physicalDevices[index_], i);
		if(yes) return true;
	}
	return false;
}

auto System::isGpuLowPower(uint32_t index_) const -> bool
{
	assert(index_ < physicalDevices.size());
	// TODO see if this is an okay determination.
	return(deviceProperties[index_].deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
}
auto System::createGpuDevice(uint32_t index_, Render::DeviceConfig const& config_) -> std::shared_ptr<Render::Device>
{
	std::vector<char const*> extensions;
	extensions.reserve(config_.requiredExtensions.size()+1);
	if(config_.presentable)
	{
		extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	for(auto const& s : config_.requiredExtensions)
	{
		extensions.push_back(s.c_str());
	}

	auto device = createGpuDevice(index_, config_.renderer, 1, extensions);

	if(config_.presentable)
	{
		// TODO make resizable option
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		auto window = glfwCreateWindow(config_.width, config_.height,
										"Vulkan", nullptr, nullptr);
		if(window != nullptr)
		{
			VkSurfaceKHR surface;
			CHKED(glfwCreateWindowSurface(instance, window, nullptr, &surface));

			auto display = activeDisplays.emplace_back(
					std::make_shared<Vulkan::Display>(
							config_.width, config_.height, window, surface));
			CHKED( vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
					physicalDevices[index_], surface, &display->capabilities));

			uint32_t formatCount;
			CHKED(vkGetPhysicalDeviceSurfaceFormatsKHR(
					physicalDevices[index_], surface, &formatCount, nullptr));

			if (formatCount != 0) {
				display->formats.resize(formatCount);
				CHKED(vkGetPhysicalDeviceSurfaceFormatsKHR(
					physicalDevices[index_], surface, &formatCount, display->formats.data()));
			}
			uint32_t presentModeCount;
			CHKED(vkGetPhysicalDeviceSurfacePresentModesKHR(
					physicalDevices[index_], surface, &presentModeCount, nullptr));
			if (presentModeCount != 0) {
				display->presentModes.resize(presentModeCount);
				CHKED(vkGetPhysicalDeviceSurfacePresentModesKHR(
						physicalDevices[index_], surface, &presentModeCount, display->presentModes.data()));
			}
			uint32_t imageCount = display->capabilities.minImageCount + 1;
			if (display->capabilities.maxImageCount > 0 && imageCount > display->capabilities.maxImageCount) {
				imageCount = display->capabilities.maxImageCount;
			}

			auto swapFormat = display->chooseSwapSurfaceFormat();
			VkSwapchainCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = swapFormat.format;
			createInfo.imageColorSpace = swapFormat.colorSpace;
			createInfo.imageExtent = display->chooseSwapExtent();
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			uint32_t qs[3];
			uint32_t renderQ;
			uint32_t computeQ;
			uint32_t transferQ;
			std::tie(renderQ, computeQ, transferQ) = findQueueFamily(index_, 1);
			VkBool32 presentSupport = false;
			CHKED(vkGetPhysicalDeviceSurfaceSupportKHR(
					physicalDevices[index_], renderQ, surface, &presentSupport));

			if (!presentSupport)
			{
				qs[0] = renderQ;
				qs[1] = computeQ;
				qs[2] = transferQ;
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 3;
				createInfo.pQueueFamilyIndices = qs;
			} else
				{
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}
			createInfo.preTransform = display->capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = display->chooseSwapPresentMode();
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = nullptr;
			CHKED(device->vkCreateSwapchainKHR(&createInfo, nullptr, &display->swapchainKHR));
			device->display = display;

			return device;
		}

	}
	return {};

}
auto System::createGpuDevice(uint32_t deviceIndex_, bool render_, uint32_t minQueues_, std::vector<char const*> const& requiredExtensions) -> std::shared_ptr<Vulkan::Device>
{
	// find queue of the desired type
	uint32_t renderQ;
	uint32_t computeQ;
	uint32_t transferQ;
	std::tie(renderQ, computeQ, transferQ) = findQueueFamily(deviceIndex_, minQueues_);

	// TODO queue prioritys
	int queueCount = 0;
	std::vector<float> queuePriority(minQueues_, 1.0f);
	std::array<VkDeviceQueueCreateInfo,3> queueCreateInfos;
	queueCreateInfos[queueCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfos[queueCount].queueFamilyIndex = computeQ;
	queueCreateInfos[queueCount].queueCount = minQueues_;
	queueCreateInfos[queueCount].pQueuePriorities = queuePriority.data();
	queueCount++;
	if(transferQ != computeQ)
	{
		queueCreateInfos[queueCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[queueCount].queueFamilyIndex = transferQ;
		queueCreateInfos[queueCount].queueCount = minQueues_;
		queueCreateInfos[queueCount].pQueuePriorities = queuePriority.data();
		queueCount++;
	}
	if(render_ && renderQ != computeQ)
	{
		queueCreateInfos[queueCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[queueCount].queueFamilyIndex = renderQ;
		queueCreateInfos[queueCount].queueCount = minQueues_;
		queueCreateInfos[queueCount].pQueuePriorities = queuePriority.data();
		queueCount++;
	}

	// TODO device feature selection
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = queueCount;
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = requiredExtensions.size();
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	// TODO layers
//	if (enableValidationLayers) {
//		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
//		createInfo.ppEnabledLayerNames = validationLayers.data();
//	} else {
		createInfo.enabledLayerCount = 0;
//	}
	VkDevice vkdevice;
	CHKED(vkCreateDevice(physicalDevices[deviceIndex_], &createInfo, nullptr, &vkdevice));

	auto device = std::make_shared<Vulkan::Device>(
			physicalDevices[deviceIndex_], vkdevice, renderQ, computeQ, transferQ);
#define DEVICE_VULKAN_FUNC( name, level ) \
   device->_##name = (PFN_##name)vkGetDeviceProcAddr( device->device, #name );       \
   if( device->_##name == nullptr ) { LOG_S(ERROR) << "Could not load " << #level  << " vulkan function named: " << #name; }

#define DEVICE_VK_FUNC( name ) DEVICE_VULKAN_FUNC(name, device)

#define DEVICE_VK_FUNC_EXT( name, extension )            \
	for( auto const& ext : requiredExtensions ) { \
		if( std::string( ext ) == std::string( extension ) ) {\
			DEVICE_VULKAN_FUNC(name, device extension) \
		} \
	}
#include "functionlist.inl"
	device->vkGetDeviceQueue(renderQ, 0, &device->renderQueue);
	device->vkGetDeviceQueue(computeQ, 0, &device->computeQueue);
	device->vkGetDeviceQueue(transferQ, 0, &device->transferQueue);

	return device;
}

auto System::findQueueFamily(uint32_t deviceIndex_, uint32_t minQueues_) -> std::tuple<uint32_t, uint32_t, uint32_t>
{
	assert(deviceIndex_ < physicalDevices.size());
	auto graphicsIndex = ~0;
	auto computeIndex = ~0;
	auto transferIndex = ~0;

	for( auto const& familyQueue : deviceQueueFamilies[deviceIndex_])
	{
		if(familyQueue.queueCount < minQueues_) continue;

		if(graphicsIndex == ~0 && familyQueue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsIndex = &familyQueue - deviceQueueFamilies[deviceIndex_].data();
		}
		if(computeIndex == ~0 && familyQueue.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			computeIndex = &familyQueue - deviceQueueFamilies[deviceIndex_].data();
		}
		if(transferIndex == ~0 && familyQueue.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			transferIndex = &familyQueue - deviceQueueFamilies[deviceIndex_].data();
		}
	}
	return { graphicsIndex, computeIndex, transferIndex};
}

}