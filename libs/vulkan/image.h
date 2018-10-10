#pragma once
#ifndef WYRD_VULKAN_IMAGE_H
#define WYRD_VULKAN_IMAGE_H

#include "core/core.h"
#include "vulkan/vkfcracker.h"
#include "render/image.h"

namespace Vulkan {

using Render::Channel;

class Image : protected Render::Image<"VIMG"_resource_id>
{
public:
	using Ptr = std::shared_ptr<Image>;
	using ConstPtr = std::shared_ptr<Image const>;
	using WeakPtr = std::weak_ptr<Image>;
	using ConstWeakPtr = std::weak_ptr<Image const>;

	using Pixel = Image::Pixel;

	static auto RegisterResourceHandler( ResourceManager::ResourceMan& rm_ ) -> void;

	static auto Create( uint32_t width_,
						uint32_t height_,
						uint32_t depth_,
						uint32_t slices_,
						VkFormat fmt_ ) -> Ptr;

	using Render::Image<Id>::is1D;
	using Render::Image<Id>::is2D;
	using Render::Image<Id>::is3D;
	using Render::Image<Id>::isArray;

	constexpr static auto computeDataSize( uint32_t width_,
										   uint32_t height_,
										   uint32_t depth_,
										   uint32_t slices_,
										   VkFormat fmt_ ) -> uint64_t
	{
		return width_ * height_ * depth_ * slices_ * (VkfCracker::bitWidth(fmt_) / 8);
	}

	auto getVkFormat() const -> VkFormat { return *(VkFormat*)subClassData; }

	constexpr auto computeDataSize() const -> uint64_t
	{
		return computeDataSize( width, height, depth, slices, getVkFormat());
	}

	auto pixelAt( unsigned int x_, unsigned int y_ = 0,
				  unsigned int z_ = 0, unsigned int slice_ = 0 ) const -> Pixel;

	auto channelAt( Channel channel_,
					unsigned int x_, unsigned int y_ = 0,
					unsigned int z_ = 0, unsigned int slice_ = 0 ) const -> double;

	auto setPixelAt( Pixel const& pixel_, unsigned int x_, unsigned int y_ = 0,
					 unsigned int z_ = 0, unsigned int slice_ = 0 ) -> void;

	auto setChannelAt( double value_, Channel channel_, unsigned int x_, unsigned int y_ = 0,
					   unsigned int z_ = 0, unsigned int slice_ = 0 ) -> void;

};

}

#endif //WYRD_IMAGE_H
