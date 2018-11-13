#pragma once
#ifndef WYRD_RENDER_IMAGE_H
#define WYRD_RENDER_IMAGE_H

#include "core/core.h"
#include "math/scalar_math.h"
#include "math/colourspace.h"
#include "render/generictextureformat.h"
#include "render/gtfcracker.h"
#include "render/resources.h"
#include "resourcemanager/resource.h"
#include <cstddef>
#include <array>

namespace ResourceManager {
class ResourceMan;

struct ResourceNameView;
}

namespace Render {

struct ImageBase
{
};

// Upto 3D image data, stored as packed formats but accessed as double
// upto 4 channels per pixel always RGBA (R = channel 0, A = channel 3)
// Support image arrays/slices
// the default is the generic texture format image (GIMG) but can be subclassed
// Image always requires to the first channel as R etc.
// this means that you ask for R and it will retrieve it from wherever
// it really is in the format (i.e. you don't worry about how its encoded)
// however this does leave a few formats a bit weird, i.e. X8D24 has X as
// R and D as G.. but that matches shaders generally anyway.
template<ResourceManager::ResourceId id_ = GenericImageId>
class Image : public ResourceManager::Resource<id_>, public ImageBase
{
public:

	using Pixel = struct
	{
		double r;
		double g;
		double b;
		double a;

		auto log() const { LOG_F(INFO, "rgba = {%f, %f, %f, %f}", r, g, b, a); }
		auto clamp( double const min_[4], double const max_[4])
		{
			r = Math::Clamp(r, min_[0], max_[0]);
			g = Math::Clamp(g, min_[1], max_[1]);
			b = Math::Clamp(b, min_[2], max_[2]);
			a = Math::Clamp(a, min_[3], max_[3]);
		}
	};

	constexpr auto is1D() const { return height == 1 && depth == 1; }

	constexpr auto is2D() const { return depth == 1; }

	constexpr auto is3D() const { return depth != 1; }

	constexpr auto isArray() const { return slices != 1; }

	constexpr static auto computeDataSize(uint32_t width_, uint32_t height_, uint32_t depth_, uint32_t slices_,
										  GenericTextureFormat fmt_) -> uint64_t
	{
		return width_ * height_ * depth_ * slices_ * (GtfCracker::bitWidth(fmt_) / 8);
	}

	constexpr auto computeDataSize() const -> uint64_t
	{
		return computeDataSize(width, height, depth, slices, format);
	}

	uint64_t dataSize;

	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t slices;

	GenericTextureFormat format;
	uint32_t subClassData; // not used by generic but allows subclasses to store something

	auto data() { return (uint8_t*) (this + 1); }

	auto data() const { return (uint8_t*) (this + 1); }

	auto pixelAt(unsigned int x_, unsigned int y_ = 0, unsigned int z_ = 0, unsigned int slice_ = 0) const -> Pixel;

	auto channelAt(Channel channel_, unsigned int x_, unsigned int y_ = 0, unsigned int z_ = 0,
				   unsigned int slice_ = 0) const -> double;

	auto setPixelAt(Pixel const& pixel_, unsigned int x_, unsigned int y_ = 0, unsigned int z_ = 0,
					unsigned int slice_ = 0) -> void;

	auto setChannelAt(double value_, Channel channel_, unsigned int x_, unsigned int y_ = 0, unsigned int z_ = 0,
					  unsigned int slice_ = 0) -> void;


protected:
	~Image() = default;
	constexpr auto
	calculateIndex(unsigned int x_, unsigned int y_, unsigned int z_, unsigned int slice_) const -> size_t;

	using SwizzleFormat = GtfCracker::SwizzleFormat;
	static auto swizzle(GenericTextureFormat fmt_, Channel channel_) -> uint8_t
	{
		return GtfCracker::swizzle(GtfCracker::swizzleFormat(fmt_), channel_);
	}


	auto compressedChannelAt(Channel channel_, size_t index_) const -> double;

	// These fetchs were there are multiple possible decode path are taken
	// where possible from the vulkan spec, then the source to
	// DirectXTex tool and the DirectX packed vector docs
	template<typename type_>
	static auto fetchRaw(uint8_t const *ptr_) -> type_;

	template<typename type_>
	static auto fetchHomoChannel(uint8_t channel_, uint8_t const *ptr_) -> double;

	template<typename type_>
	static auto fetchHomoChannel_NORM(uint8_t channel_, uint8_t const *ptr_) -> double;

	template<typename type_>
	static auto fetchHomoChannel_sRGB(uint8_t channel_, uint8_t const *ptr_) -> double;

	static auto fetchHomoChannel_nibble(uint8_t channel_, uint8_t const *ptr_) -> double;
	static auto fetchHomoChannel_nibble_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double;
	static auto fetchChannel_R5G6B5_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double;
	static auto fetchChannel_R5G5B5A1_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double;
	static auto fetchChannel_A1R5G5B5_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double;
	static auto fetchHomoChannel_FP16(uint8_t channel_, uint8_t const *ptr_) -> double;
	static auto fetchChannel_A2R10G10B10(uint8_t channel_, uint8_t const *ptr_) -> double;
	static auto fetchChannel_A2R10G10B10_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double;
	static auto fetchChannel_X8D24_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double;
	static auto fetchChannel_D24X8_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double;
	static auto fetchChannel_D16S8_UNORM_UINT(uint8_t channel_, uint8_t const *ptr_) -> double;

	template<typename type_>
	static auto putRaw(uint8_t *ptr_, type_ const value_) -> void;
	template<typename type_>
	static auto putHomoChannel(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;

	template<typename type_>
	static auto putHomoChannel_NORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;

	template<typename type_>
	static auto putHomoChannel_sRGB(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;

	static auto putHomoChannel_nibble(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;
	static auto putHomoChannel_nibble_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;
	static auto putChannel_R5G6B5_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;
	static auto putChannel_R5G5B5A1_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;
	static auto putChannel_A1R5G5B5_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;
	static auto putHomoChannel_FP16(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;
	static auto putChannel_A2R10G10B10(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;
	static auto putChannel_A2R10G10B10_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;
	static auto putChannel_X8D24_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;
	static auto putChannel_D24X8_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;
	static auto putChannel_D16S8_UNORM_UINT(uint8_t channel_, uint8_t *ptr_, double const value_) -> void;

	static auto bitWidth256ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double;
	static auto bitWidth192ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double;
	static auto bitWidth128ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double;
	static auto bitWidth96ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double;
	static auto bitWidth64ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double;
	static auto bitWidth48ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double;
	static auto bitWidth32ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double;
	static auto bitWidth24ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double;
	static auto bitWidth16ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double;
	static auto bitWidth8ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double;

	static auto
	bitWidth256SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_, double const value_) -> void;
	static auto
	bitWidth192SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_, double const value_) -> void;
	static auto
	bitWidth128SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_, double const value_) -> void;
	static auto
	bitWidth96SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_, double const value_) -> void;
	static auto
	bitWidth64SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_, double const value_) -> void;
	static auto
	bitWidth48SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_, double const value_) -> void;
	static auto
	bitWidth32SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_, double const value_) -> void;
	static auto
	bitWidth24SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_, double const value_) -> void;
	static auto
	bitWidth16SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_, double const value_) -> void;
	static auto
	bitWidth8SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_, double const value_) -> void;

};

#include "render/image_fetch.inl"
#include "render/image_put.inl"
#include "render/image.inl"

struct alignas(8) GenericImage : public Image<>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			uint32_t width_,
			uint32_t height_,
			uint32_t depth_,
			uint32_t slices_,
			GenericTextureFormat fmt_) -> GenericImageHandle;

};

}


#endif //WYRD_RENDER_IMAGE_H
