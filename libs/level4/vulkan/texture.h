#pragma once
#ifndef WYRD_VULKAN_TEXTURE_H
#define WYRD_VULKAN_TEXTURE_H

#include "core/core.h"
#include "render/texture.h"
#include "vulkan/api.h"
#include "vulkan/device.h"

namespace Render {
struct Encoder;
}

namespace Vulkan {
struct Device;

struct Texture : public Render::IGpuTexture
{
	using Ptr = std::shared_ptr<Texture>;
	using ConstPtr = std::shared_ptr<Texture const>;
	using WeakPtr = std::weak_ptr<Texture>;
	using ConstWeakPtr = std::weak_ptr<Texture const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_, std::weak_ptr<Device> device_) -> void;
	inline static int s_stage = -1;

	~Texture() final {};

	auto transitionToRenderTarget(std::shared_ptr<Render::Encoder> const& encoder_) -> void final;
	auto transitionToDMADest(std::shared_ptr<Render::Encoder> const& encoder_) -> void;
	auto transitionToShaderSrc(std::shared_ptr<Render::Encoder> const& encoder_) -> void final;
	auto transitionToDMASrc(std::shared_ptr<Render::Encoder> const& encoder_) -> void final;

	VkImage image;
	VmaAllocation allocation;

	VkFormat format;
	VkImageViewType imageViewType;
	VkImageView imageView; // the default view same as when created
	VkImageSubresourceRange entireRange;
	VkImageCreateInfo createInfo;

	// note this assumes a single writer of this state!
	mutable VkImageLayout imageLayout; // this holds the current layout
};

}

#endif //WYRD_TEXTURE_H
