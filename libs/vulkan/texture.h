#pragma once
#ifndef WYRD_VULKAN_TEXTURE_H
#define WYRD_VULKAN_TEXTURE_H

#include "core/core.h"
#include "render/texture.h"
#include "vulkan/api.h"
#include "vulkan/device.h"

namespace Vulkan {
class Device;

struct Texture
{
	using Ptr = std::shared_ptr<Texture>;
	using ConstPtr = std::shared_ptr<Texture const>;
	using WeakPtr = std::weak_ptr<Texture>;
	using ConstWeakPtr = std::weak_ptr<Texture const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

	Texture() = delete;
	~Texture();

	constexpr static uint32_t Id = Render::Texture::Id;

	Render::Texture::ConstWeakPtr cpuTexture;
	VkImage image;
	VmaAllocation allocation;

	VkImageViewType imageViewType;
	VkImageView imageView; // the default view same as when created
	VkImageSubresourceRange entireRange;
};

}

#endif //WYRD_TEXTURE_H
