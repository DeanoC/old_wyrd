#include "core/core.h"
#include "vulkan/system.h"
#include "vulkan/display.h"
#include "vulkan/resources.h"
#include <string_view>
#include <array>

#define CHK_F(x) if(VkResult result = (x); result != VK_SUCCESS) { LOG_S(ERROR) << "Vulkan Error: " << #x << " " << getVulkanResultString(result); return false; }

namespace {
VkDebugReportCallbackEXT debugCallbackHandle;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		VkFlags msgFlags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t srcObject,
		size_t location,
		int32_t msgCode,
		const char* pLayerPrefix,
		const char* pMsg,
		void* pUserData)
{
	if(msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		LOG_S(ERROR) << pLayerPrefix << ": " << pMsg;
	} else if(msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		LOG_S(WARNING) << pLayerPrefix << ": " << pMsg;
	} else
	{
		LOG_S(INFO) << pLayerPrefix << ": " << pMsg;
	}

	(void) objType;
	(void) srcObject;
	(void) location;
	(void) pUserData;
	(void) msgCode;
	return 1;
}
}

namespace Vulkan {
// borrow glfw result to string function
std::string_view const getVulkanResultString(VkResult result)
{
	using namespace std::string_view_literals;
	switch(result)
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

bool System::Init(std::string const& appName_,
				  std::vector<std::string> const& desiredInstanceExtensions_)
{
	//	assert(Global == nullptr);
	Global = this;

	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
#if defined(_WIN32)
	auto library = LoadLibrary("vulkan-1.dll");
	vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) GetProcAddress(library, "vkGetInstanceProcAddr");
#else
	assert(false);
#endif
	assert(vkGetInstanceProcAddr);

#define GLOBAL_VK_FUNC(name) name = (PFN_##name)vkGetInstanceProcAddr( nullptr, #name );       \
   if( name == nullptr ) { LOG_S(ERROR) << "Could not load global vulkan function named: " << #name; return false; }

#include "functionlist.inl"

	std::set<std::string> requiredInstanceExtensions;
	std::set<std::string> desiredInstanceExtensions;

	for(auto const& ext : desiredInstanceExtensions_)
	{
		desiredInstanceExtensions.insert(ext);
	}

	uint32_t instanceExtensionsCount = 0;
	CHK_F(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr));

	desiredInstanceExtensions.insert(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	desiredInstanceExtensions.insert(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

	requiredInstanceExtensions.insert(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef VK_USE_PLATFORM_WIN32_KHR
	requiredInstanceExtensions.insert(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_MACOS_MVK
	requiredInstanceExtensions.insert(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_XLIB_KHR
	requiredInstanceExtensions.insert(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_MACOS_MVK
	requiredInstanceExtensions.insert(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif

	instanceExtensions.resize(instanceExtensionsCount);
	CHK_F(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, instanceExtensions.data()));

	std::vector<char const*> instanceExtC;
	instanceExtC.reserve(requiredInstanceExtensions.size() + desiredInstanceExtensions.size());

	// this is a slow repeated search, if extension get to many apply an accelerator
	for(auto const& required : requiredInstanceExtensions)
	{
		auto const it = std::find_if(
				instanceExtensions.cbegin(), instanceExtensions.cend(),
				[&required](VkExtensionProperties const& lhs) -> bool
				{
					if(std::string(lhs.extensionName) == required)
						return true;
					else
						return false;
				});
		if(it == instanceExtensions.end())
		{
			LOG_S(ERROR) << "The extension " << required << " is required by not available";
			return false;
		} else
		{
			extensions.insert(required);
			instanceExtC.push_back(required.c_str());
		}
	}
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
				});
		if(it != instanceExtensions.end())
		{
			extensions.insert(desired);
			instanceExtC.push_back(desired.c_str());
		}
	}

	uint32_t instanceLayerCount;
	CHK_F(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
	instanceLayers.resize(instanceLayerCount);
	CHK_F(vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers.data()));

	VkApplicationInfo application_info = {
			VK_STRUCTURE_TYPE_APPLICATION_INFO,
			nullptr,
			appName_.c_str(),
			VK_MAKE_VERSION(1, 0, 0),
			"Wyrd",
			VK_MAKE_VERSION(1, 0, 0),
			0
	};
	VkInstanceCreateInfo instance_create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instance_create_info.pApplicationInfo = &application_info;
	if constexpr (enableValidationLayers)
	{
		instance_create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		instance_create_info.ppEnabledLayerNames = validationLayers.data();
	}
	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(instanceExtC.size());
	instance_create_info.ppEnabledExtensionNames = (instanceExtC.size() > 0) ? instanceExtC.data() : nullptr;

	CHK_F(vkCreateInstance(&instance_create_info, nullptr, &instance));

#define INSTANCE_VK_FUNC(name) name = (PFN_##name)vkGetInstanceProcAddr( instance, #name );       \
   if( name == nullptr ) { LOG_S(ERROR) << "Could not load instance vulkan function named: " << #name; return false; }

#define INSTANCE_VK_FUNC_EXT(name, extension) \
    if(extensions.find(extension) != extensions.end()) { \
        name = (PFN_##name)vkGetInstanceProcAddr(instance, #name); \
        if( name == nullptr ) { LOG_S(ERROR) << "Could not load instance vulkan function named: " << #name; return false; } \
    }

#include "functionlist.inl"

	VkDebugReportCallbackCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debugCreateInfo.pfnCallback = debug_callback;
	debugCreateInfo.flags = 0 |
							// VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
							//VK_DEBUG_REPORT_WARNING_BIT_EXT |
							//VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
							//VK_DEBUG_REPORT_DEBUG_BIT_EXT |
							VK_DEBUG_REPORT_ERROR_BIT_EXT;
		vkCreateDebugReportCallbackEXT(instance, &debugCreateInfo, nullptr, &debugCallbackHandle);

	uint32_t physDeviceCount;
	CHK_F(vkEnumeratePhysicalDevices(instance, &physDeviceCount, nullptr));
	physicalDevices.resize(physDeviceCount);
	deviceExtensions.resize(physDeviceCount);
	deviceProperties.resize(physDeviceCount);
	deviceFeatures.resize(physDeviceCount);
	deviceQueueFamilies.resize(physDeviceCount);

	CHK_F(vkEnumeratePhysicalDevices(instance, &physDeviceCount, physicalDevices.data()));

	for(auto i = 0u; i < physDeviceCount; ++i)
	{
		uint32_t deviceExtCount;
		CHK_F(vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &deviceExtCount, nullptr));
		deviceExtensions[i].resize(deviceExtCount);
		CHK_F(vkEnumerateDeviceExtensionProperties(physicalDevices[i],
												   nullptr,
												   &deviceExtCount,
												   deviceExtensions[i].data()));
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
		if(deviceQueueFamilies[index_][i].queueCount == 0) continue;

		VkBool32 presentSupport = false;
#if PLATFORM == WINDOWS
		presentSupport = vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevices[index_], i);
#else
#endif
		if(presentSupport) return true;
	}
	return false;
}

auto System::isGpuLowPower(uint32_t index_) const -> bool
{
	assert(index_ < physicalDevices.size());
	// TODO see if this is an okay determination.
	return (deviceProperties[index_].deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
}

auto System::createGpuDevice(uint32_t index_,
							 Render::DeviceConfig const& config_,
							 std::shared_ptr<ResourceManager::ResourceMan> const& resourceManager_) -> std::shared_ptr<Render::Device>
{
	std::vector<char const*> extensions;
	extensions.reserve(config_.requiredExtensions.size() + 1);

	if(config_.presentable)
	{
		extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	for(auto const& s : config_.requiredExtensions)
	{
		extensions.push_back(s.c_str());
	}

	VkSurfaceKHR surface = VK_NULL_HANDLE;
	if(config_.presentable)
	{
		// TODO make resizable option
#if PLATFORM == WINDOWS
		struct Win32PresentationWindow
		{
			HINSTANCE hinstance;
			HWND hwnd;
		};

		auto presentationWindow = (Win32PresentationWindow*) config_.window;
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.flags = 0;
		createInfo.hinstance = presentationWindow->hinstance;
		createInfo.hwnd = presentationWindow->hwnd;
		vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
#endif
	}

	Device::Ptr device = createGpuDevice(index_,
										 config_.renderer,
										 surface,
										 1,
										 extensions,
										 resourceManager_);
	if(!device) return {};

	RegisterResourceHandlers(*resourceManager_.get(), device);

	if(surface != VK_NULL_HANDLE)
	{
		auto display = std::make_shared<Vulkan::Display>(
						config_.width, config_.height, device, config_.window, surface);

		display->createSwapChain();
		device->display = display;
	}

	return device;
}

auto System::createGpuDevice(uint32_t deviceIndex_,
							 bool render_,
							 VkSurfaceKHR surface_,
							 uint32_t minQueues_,
							 std::vector<char const*> const& requiredExtensions,
							 std::shared_ptr<ResourceManager::ResourceMan> const& resourceManager_)
-> Vulkan::Device::Ptr
{
	// find queue of the desired type
	uint32_t qs[3]; // render, compute, transfer
	std::tie(qs[0], qs[1], qs[2]) = findQueueFamily(deviceIndex_, minQueues_);

	// render queues are optional but compute and transfer is required
	if(render_ && qs[0] == ~0) return {};
	if(qs[1] == ~0) return {};
	if(qs[2] == ~0) return {};

	// TODO queue prioritys
	int queueCount = 0;
	std::vector<float> queuePriority(minQueues_, 1.0f);
	std::array<VkDeviceQueueCreateInfo, 3> queueCreateInfos;

	uint32_t presentQ = ~0;
	for(auto i = 0u; i < 3; ++i)
	{
		uint32_t q = qs[i];
		// skip combined queues
		if(i > 0 && qs[i - 1] == q) continue;
		if(i > 1 && qs[i - 2] == q) continue;

		queueCreateInfos[queueCount] = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
		queueCreateInfos[queueCount].queueFamilyIndex = qs[i];
		queueCreateInfos[queueCount].queueCount = minQueues_;
		queueCreateInfos[queueCount].pQueuePriorities = queuePriority.data();
		queueCount++;

		if(surface_ != VK_NULL_HANDLE && presentQ == ~0)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[deviceIndex_], qs[i], surface_, &presentSupport);
			if(presentSupport) { presentQ = qs[i]; }
		}

	}

	// TODO device feature selection
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = queueCount;
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	if constexpr (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} else
	{
		createInfo.enabledLayerCount = 0;
	}

	auto device = std::make_shared<Vulkan::Device>(
			resourceManager_,
			render_,
			physicalDevices[deviceIndex_],
			createInfo,
			deviceQueueFamilies[deviceIndex_],
			presentQ);

	return device;
}

auto System::findQueueFamily(uint32_t deviceIndex_, uint32_t minQueues_) -> std::tuple<uint32_t, uint32_t, uint32_t>
{
	assert(deviceIndex_ < physicalDevices.size());
	uint32_t graphicsIndex = ~0u;
	uint32_t computeIndex = ~0u;
	uint32_t transferIndex = ~0u;

	// try and get a 'pure' compute and pure transfer queues for async
	for(auto const& familyQueue : deviceQueueFamilies[deviceIndex_])
	{
		if(familyQueue.queueCount < minQueues_) continue;

		// get compute without graphics if possible
		if(familyQueue.queueFlags & VK_QUEUE_COMPUTE_BIT &&
		   !(familyQueue.queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			computeIndex = uint32_t(&familyQueue - deviceQueueFamilies[deviceIndex_].data());
		}
		// get a blit/transfer queue with no other capabilities
		if(familyQueue.queueFlags == (familyQueue.queueFlags & VK_QUEUE_TRANSFER_BIT))
		{
			transferIndex = uint32_t(&familyQueue - deviceQueueFamilies[deviceIndex_].data());
		}
	}

	for(auto const& familyQueue : deviceQueueFamilies[deviceIndex_])
	{
		if(familyQueue.queueCount < minQueues_) continue;

		if(graphicsIndex == ~0 && familyQueue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsIndex = uint32_t(&familyQueue - deviceQueueFamilies[deviceIndex_].data());
		}
		if(computeIndex == ~0 && familyQueue.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			computeIndex = uint32_t(&familyQueue - deviceQueueFamilies[deviceIndex_].data());
		}
		if(transferIndex == ~0 && familyQueue.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			transferIndex = uint32_t(&familyQueue - deviceQueueFamilies[deviceIndex_].data());
		}
	}
	return {graphicsIndex, computeIndex, transferIndex};
}

}