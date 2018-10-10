#pragma once
#ifndef WYRD_VULKAN_TEXTURE_H
#define WYRD_VULKAN_TEXTURE_H

#include "core/core.h"
#include "render/texture.h"
#include "vulkan/api.h"
#include "vulkan/device.h"

namespace Vulkan {

class Texture
{
public:
	static auto RegisterResourceHandler( ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void;

private:
	Texture() = delete;
	~Texture();

	constexpr static uint32_t Id = Render::Texture::Id;

	Render::Texture* cpuTexture;
	Device::Image image;
};

}

#endif //WYRD_TEXTURE_H
