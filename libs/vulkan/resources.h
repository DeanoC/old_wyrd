#pragma once
#ifndef WYRD_VULKAN_RESOURCEHANDLERS_H
#define WYRD_VULKAN_RESOURCEHANDLERS_H

namespace ResourceManager { class ResourceMan; }

namespace Vulkan {
struct Device;

auto RegisterResourceHandlers(ResourceManager::ResourceMan& rm_, std::shared_ptr<Device> device_) -> void;

}

#endif //WYRD_VULKAN_RESOURCEHANDLERS_H
