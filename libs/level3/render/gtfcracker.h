#pragma once
#if !defined( WYRD_RENDER_GTFCRACKER_H_ )
#define WYRD_RENDER_GTFCRACKER_H_

#include "core/core.h"
#include "render/types.h"
#include "render/generictextureformat.h"
#include "math/half.hpp"
#include <array>
#include <string_view>


namespace Render {

class GtfCracker
{
public:
	constexpr static auto name(GenericTextureFormat const fmt_) -> std::string_view
	{
#define GTF_START_MACRO switch(fmt_) {
#define GTF_MOD_MACRO(x) case GenericTextureFormat:: x: return #x;
#define GTF_END_MACRO };

#include "render/generictextureformat.h"

		return "";
	}

	using SwizzleFormat = std::array<uint8_t, 4>;

	static constexpr SwizzleFormat RGBA = {0, 1, 2, 3};
	static constexpr SwizzleFormat ARGB = {1, 2, 3, 0};
	static constexpr SwizzleFormat BGRA = {2, 1, 0, 3};
	static constexpr SwizzleFormat ABGR = {3, 2, 1, 0};

	static constexpr auto swizzle(SwizzleFormat const& format_, Channel from_) -> uint8_t
	{
		return format_[(int) from_];
	}
	static constexpr auto swizzle(GenericTextureFormat fmt_, Channel from_) -> uint8_t
	{
		return swizzle(swizzleFormat(fmt_), from_);
	}

	//! is this texture format a depth format?
	constexpr static auto isDepth(GenericTextureFormat const fmt_) -> bool
	{
		switch(fmt_)
		{
			case GenericTextureFormat::D16_UNORM:
			case GenericTextureFormat::X8_D24_UNORM_PACK32:
			case GenericTextureFormat::D32_SFLOAT:
			case GenericTextureFormat::D16_UNORM_S8_UINT:
			case GenericTextureFormat::D24_UNORM_S8_UINT:
			case GenericTextureFormat::D32_SFLOAT_S8_UINT:
				return true;
			default:
				return false;
		}
	}

	constexpr static auto isStencil(GenericTextureFormat const fmt_) -> bool
	{
		switch(fmt_)
		{
			case GenericTextureFormat::S8_UINT:
			case GenericTextureFormat::D16_UNORM_S8_UINT:
			case GenericTextureFormat::D24_UNORM_S8_UINT:
			case GenericTextureFormat::D32_SFLOAT_S8_UINT:
				return true;
			default:
				return false;
		}
	}

	//! is this texture format a depth stencil format?
	constexpr static auto isDepthStencil(GenericTextureFormat const fmt_) -> bool
	{
		switch(fmt_)
		{
			case GenericTextureFormat::D16_UNORM_S8_UINT:
			case GenericTextureFormat::D24_UNORM_S8_UINT:
			case GenericTextureFormat::D32_SFLOAT_S8_UINT:
				return true;
			default:
				return false;
		}
	}

	constexpr static auto isFloat(GenericTextureFormat const fmt_) -> bool
	{
		switch(fmt_)
		{
			case GenericTextureFormat::R16_SFLOAT:
			case GenericTextureFormat::R16G16_SFLOAT:
			case GenericTextureFormat::R16G16B16_SFLOAT:
			case GenericTextureFormat::R16G16B16A16_SFLOAT:
			case GenericTextureFormat::R32_SFLOAT:
			case GenericTextureFormat::R32G32_SFLOAT:
			case GenericTextureFormat::R32G32B32_SFLOAT:
			case GenericTextureFormat::R32G32B32A32_SFLOAT:
			case GenericTextureFormat::R64_SFLOAT:
			case GenericTextureFormat::R64G64_SFLOAT:
			case GenericTextureFormat::R64G64B64_SFLOAT:
			case GenericTextureFormat::R64G64B64A64_SFLOAT:
//			case GenericTextureFormat::B10G11R11_UFLOAT_PACK32:
//			case GenericTextureFormat::E5B9G9R9_UFLOAT_PACK32:
			case GenericTextureFormat::D32_SFLOAT:
			case GenericTextureFormat::D32_SFLOAT_S8_UINT:
			case GenericTextureFormat::BC6H_UFLOAT_BLOCK:
			case GenericTextureFormat::BC6H_SFLOAT_BLOCK:
				return true;
			default:
				return false;
		}
	}

	constexpr static auto isNormalised(GenericTextureFormat const fmt_) -> bool
	{
		switch(fmt_)
		{
			case GenericTextureFormat::R4G4_UNORM_PACK8:
			case GenericTextureFormat::R4G4B4A4_UNORM_PACK16:
			case GenericTextureFormat::B4G4R4A4_UNORM_PACK16:
			case GenericTextureFormat::R5G6B5_UNORM_PACK16:
			case GenericTextureFormat::B5G6R5_UNORM_PACK16:
			case GenericTextureFormat::R5G5B5A1_UNORM_PACK16:
			case GenericTextureFormat::B5G5R5A1_UNORM_PACK16:
			case GenericTextureFormat::A1R5G5B5_UNORM_PACK16:
			case GenericTextureFormat::R8_UNORM:
			case GenericTextureFormat::R8_SNORM:
			case GenericTextureFormat::R8G8_UNORM:
			case GenericTextureFormat::R8G8_SNORM:
			case GenericTextureFormat::R8G8B8_UNORM:
			case GenericTextureFormat::R8G8B8_SNORM:
			case GenericTextureFormat::B8G8R8_UNORM:
			case GenericTextureFormat::B8G8R8_SNORM:
			case GenericTextureFormat::R8G8B8A8_UNORM:
			case GenericTextureFormat::R8G8B8A8_SNORM:
			case GenericTextureFormat::B8G8R8A8_UNORM:
			case GenericTextureFormat::B8G8R8A8_SNORM:
			case GenericTextureFormat::A8B8G8R8_UNORM_PACK32:
			case GenericTextureFormat::A8B8G8R8_SNORM_PACK32:
			case GenericTextureFormat::A2R10G10B10_UNORM_PACK32:
			case GenericTextureFormat::A2B10G10R10_UNORM_PACK32:
			case GenericTextureFormat::R16_UNORM:
			case GenericTextureFormat::R16_SNORM:
			case GenericTextureFormat::R16G16_UNORM:
			case GenericTextureFormat::R16G16_SNORM:
			case GenericTextureFormat::R16G16B16_UNORM:
			case GenericTextureFormat::R16G16B16_SNORM:
			case GenericTextureFormat::R16G16B16A16_UNORM:
			case GenericTextureFormat::R16G16B16A16_SNORM:
			case GenericTextureFormat::X8_D24_UNORM_PACK32:
			case GenericTextureFormat::D16_UNORM:
			case GenericTextureFormat::D16_UNORM_S8_UINT:
			case GenericTextureFormat::D24_UNORM_S8_UINT:
			case GenericTextureFormat::BC1_RGB_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGBA_UNORM_BLOCK:
			case GenericTextureFormat::BC2_UNORM_BLOCK:
			case GenericTextureFormat::BC3_UNORM_BLOCK:
			case GenericTextureFormat::BC4_UNORM_BLOCK:
			case GenericTextureFormat::BC4_SNORM_BLOCK:
			case GenericTextureFormat::BC5_UNORM_BLOCK:
			case GenericTextureFormat::BC5_SNORM_BLOCK:
			case GenericTextureFormat::BC7_UNORM_BLOCK:
				return true;
			default:
				return false;
		}
	}
	constexpr static auto isSigned(GenericTextureFormat const fmt_) -> bool
	{
		switch(fmt_)
		{
			case GenericTextureFormat::R8_SNORM:
			case GenericTextureFormat::R8_SSCALED:
			case GenericTextureFormat::R8_SINT:
			case GenericTextureFormat::R8G8_SNORM:
			case GenericTextureFormat::R8G8_SSCALED:
			case GenericTextureFormat::R8G8_SINT:
			case GenericTextureFormat::R8G8B8_SNORM:
			case GenericTextureFormat::R8G8B8_SSCALED:
			case GenericTextureFormat::R8G8B8_SINT:
			case GenericTextureFormat::B8G8R8_SNORM:
			case GenericTextureFormat::B8G8R8_SINT:
			case GenericTextureFormat::B8G8R8_SSCALED:
			case GenericTextureFormat::R8G8B8A8_SNORM:
			case GenericTextureFormat::R8G8B8A8_SINT:
			case GenericTextureFormat::R8G8B8A8_SSCALED:
			case GenericTextureFormat::B8G8R8A8_SNORM:
			case GenericTextureFormat::B8G8R8A8_SINT:
			case GenericTextureFormat::B8G8R8A8_SSCALED:
			case GenericTextureFormat::A8B8G8R8_SNORM_PACK32:
			case GenericTextureFormat::A8B8G8R8_SINT_PACK32:
			case GenericTextureFormat::A8B8G8R8_SSCALED_PACK32:
			case GenericTextureFormat::R16_SNORM:
			case GenericTextureFormat::R16_SINT:
			case GenericTextureFormat::R16_SSCALED:
			case GenericTextureFormat::R16_SFLOAT:
			case GenericTextureFormat::R16G16_SNORM:
			case GenericTextureFormat::R16G16_SINT:
			case GenericTextureFormat::R16G16_SSCALED:
			case GenericTextureFormat::R16G16_SFLOAT:
			case GenericTextureFormat::R16G16B16_SNORM:
			case GenericTextureFormat::R16G16B16_SINT:
			case GenericTextureFormat::R16G16B16_SSCALED:
			case GenericTextureFormat::R16G16B16_SFLOAT:
			case GenericTextureFormat::R16G16B16A16_SNORM:
			case GenericTextureFormat::R16G16B16A16_SINT:
			case GenericTextureFormat::R16G16B16A16_SSCALED:
			case GenericTextureFormat::R16G16B16A16_SFLOAT:
			case GenericTextureFormat::R32_SINT:
			case GenericTextureFormat::R32_SFLOAT:
			case GenericTextureFormat::R32G32_SINT:
			case GenericTextureFormat::R32G32_SFLOAT:
			case GenericTextureFormat::R32G32B32_SINT:
			case GenericTextureFormat::R32G32B32_SFLOAT:
			case GenericTextureFormat::R32G32B32A32_SINT:
			case GenericTextureFormat::R32G32B32A32_SFLOAT:
			case GenericTextureFormat::R64_SINT:
			case GenericTextureFormat::R64_SFLOAT:
			case GenericTextureFormat::R64G64_SINT:
			case GenericTextureFormat::R64G64_SFLOAT:
			case GenericTextureFormat::R64G64B64_SINT:
			case GenericTextureFormat::R64G64B64_SFLOAT:
			case GenericTextureFormat::R64G64B64A64_SINT:
			case GenericTextureFormat::R64G64B64A64_SFLOAT:
			case GenericTextureFormat::D32_SFLOAT:
			case GenericTextureFormat::D32_SFLOAT_S8_UINT:
			case GenericTextureFormat::BC4_SNORM_BLOCK:
			case GenericTextureFormat::BC5_SNORM_BLOCK:
			case GenericTextureFormat::BC6H_SFLOAT_BLOCK:
				return true;

			default: return false;
		}
	}
	constexpr static auto isSRGB(GenericTextureFormat const fmt_) -> bool
	{
		switch(fmt_)
		{
			case GenericTextureFormat::R8_SRGB:
			case GenericTextureFormat::R8G8_SRGB:
			case GenericTextureFormat::R8G8B8_SRGB:
			case GenericTextureFormat::B8G8R8_SRGB:
			case GenericTextureFormat::R8G8B8A8_SRGB:
			case GenericTextureFormat::B8G8R8A8_SRGB:
			case GenericTextureFormat::A8B8G8R8_SRGB_PACK32:
			case GenericTextureFormat::BC1_RGB_SRGB_BLOCK:
			case GenericTextureFormat::BC1_RGBA_SRGB_BLOCK:
			case GenericTextureFormat::BC2_SRGB_BLOCK:
			case GenericTextureFormat::BC3_SRGB_BLOCK:
			case GenericTextureFormat::BC7_SRGB_BLOCK:
				return true;

			default: return false;
		}
	}
	constexpr static auto isCompressed(GenericTextureFormat const fmt_) -> bool
	{
		switch(fmt_)
		{
			case GenericTextureFormat::BC1_RGB_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGB_SRGB_BLOCK:
			case GenericTextureFormat::BC1_RGBA_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGBA_SRGB_BLOCK:
			case GenericTextureFormat::BC2_UNORM_BLOCK:
			case GenericTextureFormat::BC2_SRGB_BLOCK:
			case GenericTextureFormat::BC3_UNORM_BLOCK:
			case GenericTextureFormat::BC3_SRGB_BLOCK:
			case GenericTextureFormat::BC4_UNORM_BLOCK:
			case GenericTextureFormat::BC4_SNORM_BLOCK:
			case GenericTextureFormat::BC5_UNORM_BLOCK:
			case GenericTextureFormat::BC5_SNORM_BLOCK:
			case GenericTextureFormat::BC6H_UFLOAT_BLOCK:
			case GenericTextureFormat::BC6H_SFLOAT_BLOCK:
			case GenericTextureFormat::BC7_UNORM_BLOCK:
			case GenericTextureFormat::BC7_SRGB_BLOCK:
				return true;
			default:
				return false;
		}
	}

	//! returns the number of channels per gl format
	constexpr static auto channelCount(GenericTextureFormat const fmt_) -> unsigned int
	{
		switch(fmt_)
		{
			case GenericTextureFormat::R4G4B4A4_UNORM_PACK16:
			case GenericTextureFormat::B4G4R4A4_UNORM_PACK16:
			case GenericTextureFormat::R5G5B5A1_UNORM_PACK16:
			case GenericTextureFormat::B5G5R5A1_UNORM_PACK16:
			case GenericTextureFormat::A1R5G5B5_UNORM_PACK16:
			case GenericTextureFormat::R8G8B8A8_UNORM:
			case GenericTextureFormat::R8G8B8A8_SNORM:
			case GenericTextureFormat::R8G8B8A8_USCALED:
			case GenericTextureFormat::R8G8B8A8_SSCALED:
			case GenericTextureFormat::R8G8B8A8_UINT:
			case GenericTextureFormat::R8G8B8A8_SINT:
			case GenericTextureFormat::R8G8B8A8_SRGB:
			case GenericTextureFormat::B8G8R8A8_UNORM:
			case GenericTextureFormat::B8G8R8A8_SNORM:
			case GenericTextureFormat::B8G8R8A8_USCALED:
			case GenericTextureFormat::B8G8R8A8_SSCALED:
			case GenericTextureFormat::B8G8R8A8_UINT:
			case GenericTextureFormat::B8G8R8A8_SINT:
			case GenericTextureFormat::B8G8R8A8_SRGB:
			case GenericTextureFormat::A8B8G8R8_UNORM_PACK32:
			case GenericTextureFormat::A8B8G8R8_SNORM_PACK32:
			case GenericTextureFormat::A8B8G8R8_USCALED_PACK32:
			case GenericTextureFormat::A8B8G8R8_SSCALED_PACK32:
			case GenericTextureFormat::A8B8G8R8_UINT_PACK32:
			case GenericTextureFormat::A8B8G8R8_SINT_PACK32:
			case GenericTextureFormat::A8B8G8R8_SRGB_PACK32:
			case GenericTextureFormat::A2R10G10B10_UNORM_PACK32:
			case GenericTextureFormat::A2R10G10B10_USCALED_PACK32:
			case GenericTextureFormat::A2R10G10B10_UINT_PACK32:
			case GenericTextureFormat::A2B10G10R10_UNORM_PACK32:
			case GenericTextureFormat::A2B10G10R10_USCALED_PACK32:
			case GenericTextureFormat::A2B10G10R10_UINT_PACK32:
			case GenericTextureFormat::R16G16B16A16_UNORM:
			case GenericTextureFormat::R16G16B16A16_SNORM:
			case GenericTextureFormat::R16G16B16A16_USCALED:
			case GenericTextureFormat::R16G16B16A16_SSCALED:
			case GenericTextureFormat::R16G16B16A16_UINT:
			case GenericTextureFormat::R16G16B16A16_SINT:
			case GenericTextureFormat::R16G16B16A16_SFLOAT:
			case GenericTextureFormat::R32G32B32A32_UINT:
			case GenericTextureFormat::R32G32B32A32_SINT:
			case GenericTextureFormat::R32G32B32A32_SFLOAT:
			case GenericTextureFormat::R64G64B64A64_UINT:
			case GenericTextureFormat::R64G64B64A64_SINT:
			case GenericTextureFormat::R64G64B64A64_SFLOAT:
			case GenericTextureFormat::BC1_RGBA_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGBA_SRGB_BLOCK:
			case GenericTextureFormat::BC2_UNORM_BLOCK:
			case GenericTextureFormat::BC2_SRGB_BLOCK:
			case GenericTextureFormat::BC3_UNORM_BLOCK:
			case GenericTextureFormat::BC3_SRGB_BLOCK:
			case GenericTextureFormat::BC7_UNORM_BLOCK:
			case GenericTextureFormat::BC7_SRGB_BLOCK:
//			case GenericTextureFormat::E5B9G9R9_UFLOAT_PACK32:
				return 4;

			case GenericTextureFormat::R5G6B5_UNORM_PACK16:
			case GenericTextureFormat::B5G6R5_UNORM_PACK16:
			case GenericTextureFormat::R8G8B8_UNORM:
			case GenericTextureFormat::R8G8B8_SNORM:
			case GenericTextureFormat::R8G8B8_USCALED:
			case GenericTextureFormat::R8G8B8_SSCALED:
			case GenericTextureFormat::R8G8B8_UINT:
			case GenericTextureFormat::R8G8B8_SINT:
			case GenericTextureFormat::R8G8B8_SRGB:
			case GenericTextureFormat::B8G8R8_UNORM:
			case GenericTextureFormat::B8G8R8_SNORM:
			case GenericTextureFormat::B8G8R8_USCALED:
			case GenericTextureFormat::B8G8R8_SSCALED:
			case GenericTextureFormat::B8G8R8_UINT:
			case GenericTextureFormat::B8G8R8_SINT:
			case GenericTextureFormat::B8G8R8_SRGB:
			case GenericTextureFormat::R16G16B16_UNORM:
			case GenericTextureFormat::R16G16B16_SNORM:
			case GenericTextureFormat::R16G16B16_USCALED:
			case GenericTextureFormat::R16G16B16_SSCALED:
			case GenericTextureFormat::R16G16B16_UINT:
			case GenericTextureFormat::R16G16B16_SINT:
			case GenericTextureFormat::R16G16B16_SFLOAT:
			case GenericTextureFormat::R32G32B32_UINT:
			case GenericTextureFormat::R32G32B32_SINT:
			case GenericTextureFormat::R32G32B32_SFLOAT:
			case GenericTextureFormat::R64G64B64_UINT:
			case GenericTextureFormat::R64G64B64_SINT:
			case GenericTextureFormat::R64G64B64_SFLOAT:
//			case GenericTextureFormat::B10G11R11_UFLOAT_PACK32:
			case GenericTextureFormat::BC1_RGB_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGB_SRGB_BLOCK:
			case GenericTextureFormat::BC6H_UFLOAT_BLOCK:
			case GenericTextureFormat::BC6H_SFLOAT_BLOCK:
				return 3;

			case GenericTextureFormat::R4G4_UNORM_PACK8:
			case GenericTextureFormat::R8G8_UNORM:
			case GenericTextureFormat::R8G8_SNORM:
			case GenericTextureFormat::R8G8_USCALED:
			case GenericTextureFormat::R8G8_SSCALED:
			case GenericTextureFormat::R8G8_UINT:
			case GenericTextureFormat::R8G8_SINT:
			case GenericTextureFormat::R8G8_SRGB:
			case GenericTextureFormat::R16G16_UNORM:
			case GenericTextureFormat::R16G16_SNORM:
			case GenericTextureFormat::R16G16_USCALED:
			case GenericTextureFormat::R16G16_SSCALED:
			case GenericTextureFormat::R16G16_UINT:
			case GenericTextureFormat::R16G16_SINT:
			case GenericTextureFormat::R16G16_SFLOAT:
			case GenericTextureFormat::R32G32_UINT:
			case GenericTextureFormat::R32G32_SINT:
			case GenericTextureFormat::R32G32_SFLOAT:
			case GenericTextureFormat::R64G64_UINT:
			case GenericTextureFormat::R64G64_SINT:
			case GenericTextureFormat::R64G64_SFLOAT:
			case GenericTextureFormat::X8_D24_UNORM_PACK32:
			case GenericTextureFormat::D16_UNORM_S8_UINT:
			case GenericTextureFormat::D24_UNORM_S8_UINT:
			case GenericTextureFormat::D32_SFLOAT_S8_UINT:
			case GenericTextureFormat::BC5_UNORM_BLOCK:
			case GenericTextureFormat::BC5_SNORM_BLOCK:
				return 2;

			case GenericTextureFormat::R8_UNORM:
			case GenericTextureFormat::R8_SNORM:
			case GenericTextureFormat::R8_USCALED:
			case GenericTextureFormat::R8_SSCALED:
			case GenericTextureFormat::R8_UINT:
			case GenericTextureFormat::R8_SINT:
			case GenericTextureFormat::R8_SRGB:
			case GenericTextureFormat::R16_UNORM:
			case GenericTextureFormat::R16_SNORM:
			case GenericTextureFormat::R16_USCALED:
			case GenericTextureFormat::R16_SSCALED:
			case GenericTextureFormat::R16_UINT:
			case GenericTextureFormat::R16_SINT:
			case GenericTextureFormat::R16_SFLOAT:
			case GenericTextureFormat::R32_UINT:
			case GenericTextureFormat::R32_SINT:
			case GenericTextureFormat::R32_SFLOAT:
			case GenericTextureFormat::R64_UINT:
			case GenericTextureFormat::R64_SINT:
			case GenericTextureFormat::R64_SFLOAT:
			case GenericTextureFormat::D16_UNORM:
			case GenericTextureFormat::D32_SFLOAT:
			case GenericTextureFormat::S8_UINT:
			case GenericTextureFormat::BC4_UNORM_BLOCK:
			case GenericTextureFormat::BC4_SNORM_BLOCK:
				return 1;
			case GenericTextureFormat::UNDEFINED:
				return 0;

			default:
				LOG_S(ERROR) << "channelCount: " << name(fmt_) << " not handled";
				return 0;
		}
	}

	//! Returns the number of channel bits
	constexpr static auto channelBitWidth(GenericTextureFormat const fmt_, int const channel_ = 0) -> unsigned int
	{
		switch(fmt_)
		{
			case GenericTextureFormat::R64_UINT:
			case GenericTextureFormat::R64_SINT:
			case GenericTextureFormat::R64_SFLOAT:
			case GenericTextureFormat::R64G64_UINT:
			case GenericTextureFormat::R64G64_SINT:
			case GenericTextureFormat::R64G64_SFLOAT:
			case GenericTextureFormat::R64G64B64_UINT:
			case GenericTextureFormat::R64G64B64_SINT:
			case GenericTextureFormat::R64G64B64_SFLOAT:
			case GenericTextureFormat::R64G64B64A64_UINT:
			case GenericTextureFormat::R64G64B64A64_SINT:
			case GenericTextureFormat::R64G64B64A64_SFLOAT:
				return 64;
			case GenericTextureFormat::R32_UINT:
			case GenericTextureFormat::R32_SINT:
			case GenericTextureFormat::R32_SFLOAT:
			case GenericTextureFormat::R32G32_UINT:
			case GenericTextureFormat::R32G32_SINT:
			case GenericTextureFormat::R32G32_SFLOAT:
			case GenericTextureFormat::R32G32B32_UINT:
			case GenericTextureFormat::R32G32B32_SINT:
			case GenericTextureFormat::R32G32B32_SFLOAT:
			case GenericTextureFormat::R32G32B32A32_UINT:
			case GenericTextureFormat::R32G32B32A32_SINT:
			case GenericTextureFormat::R32G32B32A32_SFLOAT:
			case GenericTextureFormat::D32_SFLOAT:
				return 32;
			case GenericTextureFormat::X8_D24_UNORM_PACK32:
				if(channel_ == 1) return 24;
				else return 8;
			case GenericTextureFormat::D16_UNORM_S8_UINT:
				if(channel_ == 0) return 16;
				else return 8;
			case GenericTextureFormat::D24_UNORM_S8_UINT:
				if(channel_ == 0) return 24;
				else return 8;
			case GenericTextureFormat::D32_SFLOAT_S8_UINT:
				if(channel_ == 0) return 32;
				else return 8;

			case GenericTextureFormat::R16_UNORM:
			case GenericTextureFormat::R16_SNORM:
			case GenericTextureFormat::R16_USCALED:
			case GenericTextureFormat::R16_SSCALED:
			case GenericTextureFormat::R16_UINT:
			case GenericTextureFormat::R16_SINT:
			case GenericTextureFormat::R16_SFLOAT:
			case GenericTextureFormat::R16G16_UNORM:
			case GenericTextureFormat::R16G16_SNORM:
			case GenericTextureFormat::R16G16_USCALED:
			case GenericTextureFormat::R16G16_SSCALED:
			case GenericTextureFormat::R16G16_UINT:
			case GenericTextureFormat::R16G16_SINT:
			case GenericTextureFormat::R16G16_SFLOAT:
			case GenericTextureFormat::R16G16B16_UNORM:
			case GenericTextureFormat::R16G16B16_SNORM:
			case GenericTextureFormat::R16G16B16_USCALED:
			case GenericTextureFormat::R16G16B16_SSCALED:
			case GenericTextureFormat::R16G16B16_UINT:
			case GenericTextureFormat::R16G16B16_SINT:
			case GenericTextureFormat::R16G16B16_SFLOAT:
			case GenericTextureFormat::R16G16B16A16_UNORM:
			case GenericTextureFormat::R16G16B16A16_SNORM:
			case GenericTextureFormat::R16G16B16A16_USCALED:
			case GenericTextureFormat::R16G16B16A16_SSCALED:
			case GenericTextureFormat::R16G16B16A16_UINT:
			case GenericTextureFormat::R16G16B16A16_SINT:
			case GenericTextureFormat::R16G16B16A16_SFLOAT:
			case GenericTextureFormat::D16_UNORM:
				return 16;
//			case GenericTextureFormat::B10G11R11_UFLOAT_PACK32:
//				if(channel_ == 0) return 10;
//				else return 11;
			case GenericTextureFormat::A2R10G10B10_UNORM_PACK32:
			case GenericTextureFormat::A2R10G10B10_USCALED_PACK32:
			case GenericTextureFormat::A2R10G10B10_UINT_PACK32:
			case GenericTextureFormat::A2B10G10R10_UNORM_PACK32:
			case GenericTextureFormat::A2B10G10R10_USCALED_PACK32:
			case GenericTextureFormat::A2B10G10R10_UINT_PACK32:
				if(channel_ == 0) return 2;
				else return 10;
//			case GenericTextureFormat::E5B9G9R9_UFLOAT_PACK32:
//				if(channel_ == 0) return 5;
//				else return 9;
			case GenericTextureFormat::R8_UNORM:
			case GenericTextureFormat::R8_SNORM:
			case GenericTextureFormat::R8_USCALED:
			case GenericTextureFormat::R8_SSCALED:
			case GenericTextureFormat::R8_UINT:
			case GenericTextureFormat::R8_SINT:
			case GenericTextureFormat::R8_SRGB:
			case GenericTextureFormat::R8G8_UNORM:
			case GenericTextureFormat::R8G8_SNORM:
			case GenericTextureFormat::R8G8_USCALED:
			case GenericTextureFormat::R8G8_SSCALED:
			case GenericTextureFormat::R8G8_UINT:
			case GenericTextureFormat::R8G8_SINT:
			case GenericTextureFormat::R8G8_SRGB:
			case GenericTextureFormat::R8G8B8_UNORM:
			case GenericTextureFormat::R8G8B8_SNORM:
			case GenericTextureFormat::R8G8B8_USCALED:
			case GenericTextureFormat::R8G8B8_SSCALED:
			case GenericTextureFormat::R8G8B8_UINT:
			case GenericTextureFormat::R8G8B8_SINT:
			case GenericTextureFormat::R8G8B8_SRGB:
			case GenericTextureFormat::B8G8R8_UNORM:
			case GenericTextureFormat::B8G8R8_SNORM:
			case GenericTextureFormat::B8G8R8_USCALED:
			case GenericTextureFormat::B8G8R8_SSCALED:
			case GenericTextureFormat::B8G8R8_UINT:
			case GenericTextureFormat::B8G8R8_SINT:
			case GenericTextureFormat::B8G8R8_SRGB:
			case GenericTextureFormat::R8G8B8A8_UNORM:
			case GenericTextureFormat::R8G8B8A8_SNORM:
			case GenericTextureFormat::R8G8B8A8_USCALED:
			case GenericTextureFormat::R8G8B8A8_SSCALED:
			case GenericTextureFormat::R8G8B8A8_UINT:
			case GenericTextureFormat::R8G8B8A8_SINT:
			case GenericTextureFormat::R8G8B8A8_SRGB:
			case GenericTextureFormat::B8G8R8A8_UNORM:
			case GenericTextureFormat::B8G8R8A8_SNORM:
			case GenericTextureFormat::B8G8R8A8_USCALED:
			case GenericTextureFormat::B8G8R8A8_SSCALED:
			case GenericTextureFormat::B8G8R8A8_UINT:
			case GenericTextureFormat::B8G8R8A8_SINT:
			case GenericTextureFormat::B8G8R8A8_SRGB:
			case GenericTextureFormat::A8B8G8R8_UNORM_PACK32:
			case GenericTextureFormat::A8B8G8R8_SNORM_PACK32:
			case GenericTextureFormat::A8B8G8R8_USCALED_PACK32:
			case GenericTextureFormat::A8B8G8R8_SSCALED_PACK32:
			case GenericTextureFormat::A8B8G8R8_UINT_PACK32:
			case GenericTextureFormat::A8B8G8R8_SINT_PACK32:
			case GenericTextureFormat::A8B8G8R8_SRGB_PACK32:
			case GenericTextureFormat::BC4_UNORM_BLOCK:
			case GenericTextureFormat::BC4_SNORM_BLOCK:
			case GenericTextureFormat::BC5_UNORM_BLOCK:
			case GenericTextureFormat::BC5_SNORM_BLOCK:
			case GenericTextureFormat::BC6H_SFLOAT_BLOCK:
			case GenericTextureFormat::BC6H_UFLOAT_BLOCK:
			case GenericTextureFormat::S8_UINT:
				return 8;
				// BC7 is variable between 4-7 bits for colour and 0-8 bits alpha
			case GenericTextureFormat::BC7_UNORM_BLOCK:
			case GenericTextureFormat::BC7_SRGB_BLOCK:
				return 6;
			case GenericTextureFormat::R5G5B5A1_UNORM_PACK16:
			case GenericTextureFormat::B5G5R5A1_UNORM_PACK16:
				if(channel_ == 4) return 1;
				else return 5;
			case GenericTextureFormat::A1R5G5B5_UNORM_PACK16:
				if(channel_ == 0) return 1;
				else return 5;
			case GenericTextureFormat::B5G6R5_UNORM_PACK16:
			case GenericTextureFormat::R5G6B5_UNORM_PACK16:
			case GenericTextureFormat::BC1_RGB_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGB_SRGB_BLOCK:
			case GenericTextureFormat::BC1_RGBA_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGBA_SRGB_BLOCK:
			case GenericTextureFormat::BC2_UNORM_BLOCK:
			case GenericTextureFormat::BC2_SRGB_BLOCK:
			case GenericTextureFormat::BC3_UNORM_BLOCK:
			case GenericTextureFormat::BC3_SRGB_BLOCK:
				return 5;
			case GenericTextureFormat::R4G4_UNORM_PACK8:
			case GenericTextureFormat::R4G4B4A4_UNORM_PACK16:
			case GenericTextureFormat::B4G4R4A4_UNORM_PACK16:
				return 4;
			case GenericTextureFormat::UNDEFINED:
				return 0;
				// unknown
			default:
				LOG_S(ERROR) << "channelBitWidth: " << name(fmt_) << " not handled";
				return 0;
		}
	}
	constexpr static auto channelBitWidth(GenericTextureFormat const fmt_, Channel channel_)
	{
		return GtfCracker::channelBitWidth(fmt_, swizzle(fmt_,channel_));
	}


	/// \fn	static unsigned int getBitWidth( GenericTextureFormat fmt )
	/// \brief	Returns the number of bits in the specified GenericTextureFormat
	/// \details BCx are block compressed the value is the amount per texel even tho never accessed
	/// 		 like that, so BC1 take 4 bits to store a RGBA5551 but can't be accessed 1 pixel at
	/// 		 at time
	/// \param	fmt	Describes the format to use.
	/// \return	The bit width of the format.
	constexpr static auto bitWidth(GenericTextureFormat const fmt_) -> unsigned int
	{
		switch(fmt_)
		{
			case GenericTextureFormat::R64G64B64A64_UINT:
			case GenericTextureFormat::R64G64B64A64_SINT:
			case GenericTextureFormat::R64G64B64A64_SFLOAT:
				return 256;
			case GenericTextureFormat::R64G64B64_UINT:
			case GenericTextureFormat::R64G64B64_SINT:
			case GenericTextureFormat::R64G64B64_SFLOAT:
				return 192;

			case GenericTextureFormat::R64G64_UINT:
			case GenericTextureFormat::R64G64_SINT:
			case GenericTextureFormat::R64G64_SFLOAT:
			case GenericTextureFormat::R32G32B32A32_UINT:
			case GenericTextureFormat::R32G32B32A32_SINT:
			case GenericTextureFormat::R32G32B32A32_SFLOAT:
				return 128;

			case GenericTextureFormat::R32G32B32_UINT:
			case GenericTextureFormat::R32G32B32_SINT:
			case GenericTextureFormat::R32G32B32_SFLOAT:
				return 96;
			case GenericTextureFormat::R64_UINT:
			case GenericTextureFormat::R64_SINT:
			case GenericTextureFormat::R64_SFLOAT:
			case GenericTextureFormat::R32G32_UINT:
			case GenericTextureFormat::R32G32_SINT:
			case GenericTextureFormat::R32G32_SFLOAT:
			case GenericTextureFormat::R16G16B16A16_UNORM:
			case GenericTextureFormat::R16G16B16A16_SNORM:
			case GenericTextureFormat::R16G16B16A16_USCALED:
			case GenericTextureFormat::R16G16B16A16_SSCALED:
			case GenericTextureFormat::R16G16B16A16_UINT:
			case GenericTextureFormat::R16G16B16A16_SINT:
			case GenericTextureFormat::R16G16B16A16_SFLOAT:
				return 64;
				// best case 40 bits worse case 64, be a pessimist
			case GenericTextureFormat::D32_SFLOAT_S8_UINT:
				return 64;

			case GenericTextureFormat::R16G16B16_UNORM:
			case GenericTextureFormat::R16G16B16_SNORM:
			case GenericTextureFormat::R16G16B16_USCALED:
			case GenericTextureFormat::R16G16B16_SSCALED:
			case GenericTextureFormat::R16G16B16_UINT:
			case GenericTextureFormat::R16G16B16_SINT:
			case GenericTextureFormat::R16G16B16_SFLOAT:
				return 48;
			case GenericTextureFormat::R32_SINT:
			case GenericTextureFormat::R32_UINT:
			case GenericTextureFormat::R32_SFLOAT:
			case GenericTextureFormat::R16G16_SINT:
			case GenericTextureFormat::R16G16_SFLOAT:
			case GenericTextureFormat::R16G16_SNORM:
			case GenericTextureFormat::R16G16_UNORM:
			case GenericTextureFormat::R16G16_UINT:
			case GenericTextureFormat::R16G16_USCALED:
			case GenericTextureFormat::R16G16_SSCALED:
			case GenericTextureFormat::R8G8B8A8_UNORM:
			case GenericTextureFormat::R8G8B8A8_SNORM:
			case GenericTextureFormat::R8G8B8A8_USCALED:
			case GenericTextureFormat::R8G8B8A8_SSCALED:
			case GenericTextureFormat::R8G8B8A8_UINT:
			case GenericTextureFormat::R8G8B8A8_SINT:
			case GenericTextureFormat::R8G8B8A8_SRGB:
			case GenericTextureFormat::B8G8R8A8_UNORM:
			case GenericTextureFormat::B8G8R8A8_SNORM:
			case GenericTextureFormat::B8G8R8A8_USCALED:
			case GenericTextureFormat::B8G8R8A8_SSCALED:
			case GenericTextureFormat::B8G8R8A8_UINT:
			case GenericTextureFormat::B8G8R8A8_SINT:
			case GenericTextureFormat::B8G8R8A8_SRGB:
			case GenericTextureFormat::A8B8G8R8_UNORM_PACK32:
			case GenericTextureFormat::A8B8G8R8_SNORM_PACK32:
			case GenericTextureFormat::A8B8G8R8_USCALED_PACK32:
			case GenericTextureFormat::A8B8G8R8_SSCALED_PACK32:
			case GenericTextureFormat::A8B8G8R8_UINT_PACK32:
			case GenericTextureFormat::A8B8G8R8_SINT_PACK32:
			case GenericTextureFormat::A8B8G8R8_SRGB_PACK32:
			case GenericTextureFormat::A2R10G10B10_UNORM_PACK32:
			case GenericTextureFormat::A2R10G10B10_USCALED_PACK32:
			case GenericTextureFormat::A2R10G10B10_UINT_PACK32:
			case GenericTextureFormat::A2B10G10R10_UNORM_PACK32:
			case GenericTextureFormat::A2B10G10R10_USCALED_PACK32:
			case GenericTextureFormat::A2B10G10R10_UINT_PACK32:
//			case GenericTextureFormat::B10G11R11_UFLOAT_PACK32:
//			case GenericTextureFormat::E5B9G9R9_UFLOAT_PACK32:
			case GenericTextureFormat::X8_D24_UNORM_PACK32:
			case GenericTextureFormat::D32_SFLOAT:
			case GenericTextureFormat::D24_UNORM_S8_UINT:
				return 32;
			case GenericTextureFormat::R8G8B8_UNORM:
			case GenericTextureFormat::R8G8B8_SNORM:
			case GenericTextureFormat::R8G8B8_USCALED:
			case GenericTextureFormat::R8G8B8_SSCALED:
			case GenericTextureFormat::R8G8B8_UINT:
			case GenericTextureFormat::R8G8B8_SINT:
			case GenericTextureFormat::R8G8B8_SRGB:
			case GenericTextureFormat::B8G8R8_UNORM:
			case GenericTextureFormat::B8G8R8_SNORM:
			case GenericTextureFormat::B8G8R8_USCALED:
			case GenericTextureFormat::B8G8R8_SSCALED:
			case GenericTextureFormat::B8G8R8_UINT:
			case GenericTextureFormat::B8G8R8_SINT:
			case GenericTextureFormat::B8G8R8_SRGB:
			case GenericTextureFormat::D16_UNORM_S8_UINT:
				return 24;
			case GenericTextureFormat::R4G4B4A4_UNORM_PACK16:
			case GenericTextureFormat::B4G4R4A4_UNORM_PACK16:
			case GenericTextureFormat::R5G6B5_UNORM_PACK16:
			case GenericTextureFormat::B5G6R5_UNORM_PACK16:
			case GenericTextureFormat::R5G5B5A1_UNORM_PACK16:
			case GenericTextureFormat::B5G5R5A1_UNORM_PACK16:
			case GenericTextureFormat::A1R5G5B5_UNORM_PACK16:
			case GenericTextureFormat::R8G8_UNORM:
			case GenericTextureFormat::R8G8_SNORM:
			case GenericTextureFormat::R8G8_USCALED:
			case GenericTextureFormat::R8G8_SSCALED:
			case GenericTextureFormat::R8G8_UINT:
			case GenericTextureFormat::R8G8_SINT:
			case GenericTextureFormat::R8G8_SRGB:
			case GenericTextureFormat::R16_UNORM:
			case GenericTextureFormat::R16_SNORM:
			case GenericTextureFormat::R16_USCALED:
			case GenericTextureFormat::R16_SSCALED:
			case GenericTextureFormat::R16_UINT:
			case GenericTextureFormat::R16_SINT:
			case GenericTextureFormat::R16_SFLOAT:
			case GenericTextureFormat::D16_UNORM:
				return 16;

			case GenericTextureFormat::R4G4_UNORM_PACK8:
			case GenericTextureFormat::R8_UNORM:
			case GenericTextureFormat::R8_SNORM:
			case GenericTextureFormat::R8_USCALED:
			case GenericTextureFormat::R8_SSCALED:
			case GenericTextureFormat::R8_UINT:
			case GenericTextureFormat::R8_SINT:
			case GenericTextureFormat::R8_SRGB:
			case GenericTextureFormat::BC2_UNORM_BLOCK:
			case GenericTextureFormat::BC2_SRGB_BLOCK:
			case GenericTextureFormat::BC3_UNORM_BLOCK:
			case GenericTextureFormat::BC3_SRGB_BLOCK:
			case GenericTextureFormat::BC5_UNORM_BLOCK:
			case GenericTextureFormat::BC5_SNORM_BLOCK:
			case GenericTextureFormat::BC6H_UFLOAT_BLOCK:
			case GenericTextureFormat::BC6H_SFLOAT_BLOCK:
			case GenericTextureFormat::BC7_UNORM_BLOCK:
			case GenericTextureFormat::BC7_SRGB_BLOCK:
			case GenericTextureFormat::S8_UINT:
				return 8;
			case GenericTextureFormat::BC1_RGB_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGB_SRGB_BLOCK:
			case GenericTextureFormat::BC1_RGBA_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGBA_SRGB_BLOCK:
			case GenericTextureFormat::BC4_UNORM_BLOCK:
			case GenericTextureFormat::BC4_SNORM_BLOCK:
				return 4;
			default:
				LOG_S(ERROR) << "bitWidth: " << name(fmt_) << " not handled";
				return 0;
		}
	}

	constexpr static auto max(GenericTextureFormat const fmt_, int channel_) -> double
	{
		switch(fmt_)
		{
			case GenericTextureFormat::R64G64B64A64_UINT:
			case GenericTextureFormat::R64G64B64_UINT:
			case GenericTextureFormat::R64G64_UINT:
			case GenericTextureFormat::R64_UINT:
				return std::numeric_limits<uint64_t>::max();
			case GenericTextureFormat::R64G64B64A64_SINT:
			case GenericTextureFormat::R64G64B64_SINT:
			case GenericTextureFormat::R64G64_SINT:
			case GenericTextureFormat::R64_SINT:
				return std::numeric_limits<int64_t>::max();
			case GenericTextureFormat::R64G64B64_SFLOAT:
			case GenericTextureFormat::R64G64B64A64_SFLOAT:
			case GenericTextureFormat::R64G64_SFLOAT:
			case GenericTextureFormat::R64_SFLOAT:
				return std::numeric_limits<double>::max();

			case GenericTextureFormat::R32G32B32A32_UINT:
			case GenericTextureFormat::R32G32B32_UINT:
			case GenericTextureFormat::R32G32_UINT:
			case GenericTextureFormat::R32_UINT:
				return std::numeric_limits<uint32_t>::max();
			case GenericTextureFormat::R32G32B32A32_SINT:
			case GenericTextureFormat::R32G32B32_SINT:
			case GenericTextureFormat::R32G32_SINT:
			case GenericTextureFormat::R32_SINT:
				return std::numeric_limits<int32_t>::max();
			case GenericTextureFormat::R32G32B32A32_SFLOAT:
			case GenericTextureFormat::R32G32B32_SFLOAT:
			case GenericTextureFormat::R32G32_SFLOAT:
			case GenericTextureFormat::R32_SFLOAT:
			case GenericTextureFormat::D32_SFLOAT:
				return std::numeric_limits<float>::max();

			case GenericTextureFormat::R16G16B16A16_UNORM:
			case GenericTextureFormat::R16G16B16_UNORM:
			case GenericTextureFormat::R16G16_UNORM:
			case GenericTextureFormat::R16_UNORM:
			case GenericTextureFormat::D16_UNORM:
			case GenericTextureFormat::R16G16B16A16_USCALED:
			case GenericTextureFormat::R16G16B16_USCALED:
			case GenericTextureFormat::R16G16_USCALED:
			case GenericTextureFormat::R16_USCALED:
			case GenericTextureFormat::R16G16B16A16_UINT:
			case GenericTextureFormat::R16G16B16_UINT:
			case GenericTextureFormat::R16G16_UINT:
			case GenericTextureFormat::R16_UINT:
				return std::numeric_limits<uint16_t >::max();

			case GenericTextureFormat::R16G16B16A16_SNORM:
			case GenericTextureFormat::R16G16B16_SNORM:
			case GenericTextureFormat::R16G16_SNORM:
			case GenericTextureFormat::R16_SNORM:
			case GenericTextureFormat::R16G16B16A16_SSCALED:
			case GenericTextureFormat::R16G16B16_SSCALED:
			case GenericTextureFormat::R16G16_SSCALED:
			case GenericTextureFormat::R16_SSCALED:
			case GenericTextureFormat::R16G16B16A16_SINT:
			case GenericTextureFormat::R16G16B16_SINT:
			case GenericTextureFormat::R16G16_SINT:
			case GenericTextureFormat::R16_SINT:
				return std::numeric_limits<int16_t>::max();

			case GenericTextureFormat::R16G16B16A16_SFLOAT:
			case GenericTextureFormat::R16G16B16_SFLOAT:
			case GenericTextureFormat::R16G16_SFLOAT:
			case GenericTextureFormat::R16_SFLOAT:
			case GenericTextureFormat::BC6H_UFLOAT_BLOCK:
			case GenericTextureFormat::BC6H_SFLOAT_BLOCK:
				return std::numeric_limits<half_float::half>::max();

			case GenericTextureFormat::D32_SFLOAT_S8_UINT:
				if(channel_ == 0) return std::numeric_limits<float>::max();
				else return std::numeric_limits<uint8_t>::max();

			case GenericTextureFormat::R8G8B8A8_UNORM:
			case GenericTextureFormat::B8G8R8A8_UNORM:
			case GenericTextureFormat::A8B8G8R8_UNORM_PACK32:
			case GenericTextureFormat::R8G8B8_UNORM:
			case GenericTextureFormat::B8G8R8_UNORM:
			case GenericTextureFormat::R8G8_UNORM:
			case GenericTextureFormat::R8_UNORM:
			case GenericTextureFormat::R8G8B8A8_USCALED:
			case GenericTextureFormat::B8G8R8A8_USCALED:
			case GenericTextureFormat::A8B8G8R8_USCALED_PACK32:
			case GenericTextureFormat::R8G8B8_USCALED:
			case GenericTextureFormat::B8G8R8_USCALED:
			case GenericTextureFormat::R8G8_USCALED:
			case GenericTextureFormat::R8_USCALED:
			case GenericTextureFormat::R8G8B8A8_UINT:
			case GenericTextureFormat::B8G8R8A8_UINT:
			case GenericTextureFormat::A8B8G8R8_UINT_PACK32:
			case GenericTextureFormat::R8G8B8_UINT:
			case GenericTextureFormat::B8G8R8_UINT:
			case GenericTextureFormat::R8G8_UINT:
			case GenericTextureFormat::R8_UINT:
			case GenericTextureFormat::S8_UINT:
			case GenericTextureFormat::A8B8G8R8_SRGB_PACK32:
			case GenericTextureFormat::R8G8B8A8_SRGB:
			case GenericTextureFormat::B8G8R8A8_SRGB:
			case GenericTextureFormat::B8G8R8_SRGB:
			case GenericTextureFormat::R8G8B8_SRGB:
			case GenericTextureFormat::R8G8_SRGB:
			case GenericTextureFormat::R8_SRGB:
				return std::numeric_limits<uint8_t>::max();

			case GenericTextureFormat::R8G8B8A8_SNORM:
			case GenericTextureFormat::B8G8R8A8_SNORM:
			case GenericTextureFormat::A8B8G8R8_SNORM_PACK32:
			case GenericTextureFormat::R8G8B8_SNORM:
			case GenericTextureFormat::B8G8R8_SNORM:
			case GenericTextureFormat::R8G8_SNORM:
			case GenericTextureFormat::R8_SNORM:
			case GenericTextureFormat::R8G8B8A8_SSCALED:
			case GenericTextureFormat::B8G8R8A8_SSCALED:
			case GenericTextureFormat::A8B8G8R8_SSCALED_PACK32:
			case GenericTextureFormat::R8G8B8_SSCALED:
			case GenericTextureFormat::B8G8R8_SSCALED:
			case GenericTextureFormat::R8G8_SSCALED:
			case GenericTextureFormat::R8_SSCALED:
			case GenericTextureFormat::R8G8B8A8_SINT:
			case GenericTextureFormat::B8G8R8A8_SINT:
			case GenericTextureFormat::A8B8G8R8_SINT_PACK32:
			case GenericTextureFormat::R8G8B8_SINT:
			case GenericTextureFormat::B8G8R8_SINT:
			case GenericTextureFormat::R8G8_SINT:
			case GenericTextureFormat::R8_SINT:
				return std::numeric_limits<int8_t>::max();

			case GenericTextureFormat::A2R10G10B10_UNORM_PACK32:
			case GenericTextureFormat::A2R10G10B10_USCALED_PACK32:
			case GenericTextureFormat::A2R10G10B10_UINT_PACK32:
			case GenericTextureFormat::A2B10G10R10_UNORM_PACK32:
			case GenericTextureFormat::A2B10G10R10_USCALED_PACK32:
			case GenericTextureFormat::A2B10G10R10_UINT_PACK32:
				if(channel_ == 0) return 3.0;
				else return 1023.0;

			case GenericTextureFormat::X8_D24_UNORM_PACK32:
				if(channel_ == 0) return 255.0;
				else return double(1 << 24) - 1.0;
			case GenericTextureFormat::D24_UNORM_S8_UINT:
				if(channel_ == 1) return 255.0;
				else return double(1 << 24) - 1.0;
			case GenericTextureFormat::D16_UNORM_S8_UINT:
				if(channel_ == 1) return 255.0;
				else return 65535.0;
			case GenericTextureFormat::R4G4B4A4_UNORM_PACK16:
			case GenericTextureFormat::B4G4R4A4_UNORM_PACK16:
			case GenericTextureFormat::R4G4_UNORM_PACK8:
				return 15.0;
			case GenericTextureFormat::BC1_RGB_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGB_SRGB_BLOCK:
			case GenericTextureFormat::R5G6B5_UNORM_PACK16:
			case GenericTextureFormat::B5G6R5_UNORM_PACK16:
				if(channel_ == 1) return 63.0;
				else return 31.0;

			case GenericTextureFormat::BC1_RGBA_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGBA_SRGB_BLOCK:
			case GenericTextureFormat::R5G5B5A1_UNORM_PACK16:
			case GenericTextureFormat::B5G5R5A1_UNORM_PACK16:
				if(channel_ == 3) return 1.0;
				else return 31.0;
			case GenericTextureFormat::A1R5G5B5_UNORM_PACK16:
				if(channel_ == 0) return 1.0;
				else return 31.0;
//			case GenericTextureFormat::B10G11R11_UFLOAT_PACK32:
//				return 65000.0;
//			case GenericTextureFormat::E5B9G9R9_UFLOAT_PACK32:
//				return 65472.0;

			case GenericTextureFormat::UNDEFINED:
			case GenericTextureFormat::BC2_UNORM_BLOCK:
			case GenericTextureFormat::BC2_SRGB_BLOCK:
			case GenericTextureFormat::BC3_UNORM_BLOCK:
			case GenericTextureFormat::BC3_SRGB_BLOCK:
			case GenericTextureFormat::BC4_UNORM_BLOCK:
			case GenericTextureFormat::BC4_SNORM_BLOCK:
			case GenericTextureFormat::BC5_UNORM_BLOCK:
			case GenericTextureFormat::BC5_SNORM_BLOCK:
			case GenericTextureFormat::BC7_UNORM_BLOCK:
			case GenericTextureFormat::BC7_SRGB_BLOCK:
				return 255.0; // TODO
			default:
				LOG_S(ERROR) << "max " << name(fmt_) << " not handled";
				return 0;
		}
	}
	constexpr static auto max(GenericTextureFormat const fmt_, Channel channel_) -> double
	{
		return GtfCracker::max(fmt_, swizzle(fmt_,channel_));
	}


	constexpr static auto min(GenericTextureFormat const fmt_, int channel_) -> double
	{
		switch(fmt_)
		{
			case GenericTextureFormat::R64G64B64A64_UINT:
			case GenericTextureFormat::R64G64B64_UINT:
			case GenericTextureFormat::R64G64_UINT:
			case GenericTextureFormat::R64_UINT:
				return std::numeric_limits<uint64_t>::min();
			case GenericTextureFormat::R64G64B64A64_SINT:
			case GenericTextureFormat::R64G64B64_SINT:
			case GenericTextureFormat::R64G64_SINT:
			case GenericTextureFormat::R64_SINT:
				return std::numeric_limits<int64_t>::min();
			case GenericTextureFormat::R64G64B64_SFLOAT:
			case GenericTextureFormat::R64G64B64A64_SFLOAT:
			case GenericTextureFormat::R64G64_SFLOAT:
			case GenericTextureFormat::R64_SFLOAT:
				return std::numeric_limits<double>::min();

			case GenericTextureFormat::R32G32B32A32_UINT:
			case GenericTextureFormat::R32G32B32_UINT:
			case GenericTextureFormat::R32G32_UINT:
			case GenericTextureFormat::R32_UINT:
				return std::numeric_limits<uint32_t>::min();
			case GenericTextureFormat::R32G32B32A32_SINT:
			case GenericTextureFormat::R32G32B32_SINT:
			case GenericTextureFormat::R32G32_SINT:
			case GenericTextureFormat::R32_SINT:
				return std::numeric_limits<int32_t>::min();
			case GenericTextureFormat::R32G32B32A32_SFLOAT:
			case GenericTextureFormat::R32G32B32_SFLOAT:
			case GenericTextureFormat::R32G32_SFLOAT:
			case GenericTextureFormat::R32_SFLOAT:
			case GenericTextureFormat::D32_SFLOAT:
				return std::numeric_limits<float>::min();

			case GenericTextureFormat::R16G16B16A16_UNORM:
			case GenericTextureFormat::R16G16B16_UNORM:
			case GenericTextureFormat::R16G16_UNORM:
			case GenericTextureFormat::R16_UNORM:
			case GenericTextureFormat::D16_UNORM:
			case GenericTextureFormat::R16G16B16A16_USCALED:
			case GenericTextureFormat::R16G16B16_USCALED:
			case GenericTextureFormat::R16G16_USCALED:
			case GenericTextureFormat::R16_USCALED:
			case GenericTextureFormat::R16G16B16A16_UINT:
			case GenericTextureFormat::R16G16B16_UINT:
			case GenericTextureFormat::R16G16_UINT:
			case GenericTextureFormat::R16_UINT:
				return std::numeric_limits<uint16_t >::min();

			case GenericTextureFormat::R16G16B16A16_SNORM:
			case GenericTextureFormat::R16G16B16_SNORM:
			case GenericTextureFormat::R16G16_SNORM:
			case GenericTextureFormat::R16_SNORM:
			case GenericTextureFormat::R16G16B16A16_SSCALED:
			case GenericTextureFormat::R16G16B16_SSCALED:
			case GenericTextureFormat::R16G16_SSCALED:
			case GenericTextureFormat::R16_SSCALED:
			case GenericTextureFormat::R16G16B16A16_SINT:
			case GenericTextureFormat::R16G16B16_SINT:
			case GenericTextureFormat::R16G16_SINT:
			case GenericTextureFormat::R16_SINT:
				return std::numeric_limits<int16_t>::min();

			case GenericTextureFormat::R16G16B16A16_SFLOAT:
			case GenericTextureFormat::R16G16B16_SFLOAT:
			case GenericTextureFormat::R16G16_SFLOAT:
			case GenericTextureFormat::R16_SFLOAT:
			case GenericTextureFormat::BC6H_SFLOAT_BLOCK:
				return std::numeric_limits<half_float::half>::min();

			case GenericTextureFormat::D32_SFLOAT_S8_UINT:
				if(channel_ == 0) return std::numeric_limits<float>::min();
				else return std::numeric_limits<uint8_t>::min();

			case GenericTextureFormat::R8G8B8A8_UNORM:
			case GenericTextureFormat::B8G8R8A8_UNORM:
			case GenericTextureFormat::A8B8G8R8_UNORM_PACK32:
			case GenericTextureFormat::R8G8B8_UNORM:
			case GenericTextureFormat::B8G8R8_UNORM:
			case GenericTextureFormat::R8G8_UNORM:
			case GenericTextureFormat::R8_UNORM:
			case GenericTextureFormat::R8G8B8A8_USCALED:
			case GenericTextureFormat::B8G8R8A8_USCALED:
			case GenericTextureFormat::A8B8G8R8_USCALED_PACK32:
			case GenericTextureFormat::R8G8B8_USCALED:
			case GenericTextureFormat::B8G8R8_USCALED:
			case GenericTextureFormat::R8G8_USCALED:
			case GenericTextureFormat::R8_USCALED:
			case GenericTextureFormat::R8G8B8A8_UINT:
			case GenericTextureFormat::B8G8R8A8_UINT:
			case GenericTextureFormat::A8B8G8R8_UINT_PACK32:
			case GenericTextureFormat::R8G8B8_UINT:
			case GenericTextureFormat::B8G8R8_UINT:
			case GenericTextureFormat::R8G8_UINT:
			case GenericTextureFormat::R8_UINT:
			case GenericTextureFormat::S8_UINT:
			case GenericTextureFormat::R8G8B8A8_SRGB:
			case GenericTextureFormat::B8G8R8A8_SRGB:
			case GenericTextureFormat::A8B8G8R8_SRGB_PACK32:
			case GenericTextureFormat::R8G8B8_SRGB:
			case GenericTextureFormat::B8G8R8_SRGB:
			case GenericTextureFormat::R8G8_SRGB:
			case GenericTextureFormat::R8_SRGB:
				return std::numeric_limits<uint8_t>::min();

			case GenericTextureFormat::R8G8B8A8_SNORM:
			case GenericTextureFormat::B8G8R8A8_SNORM:
			case GenericTextureFormat::A8B8G8R8_SNORM_PACK32:
			case GenericTextureFormat::R8G8B8_SNORM:
			case GenericTextureFormat::B8G8R8_SNORM:
			case GenericTextureFormat::R8G8_SNORM:
			case GenericTextureFormat::R8_SNORM:
			case GenericTextureFormat::R8G8B8A8_SSCALED:
			case GenericTextureFormat::B8G8R8A8_SSCALED:
			case GenericTextureFormat::A8B8G8R8_SSCALED_PACK32:
			case GenericTextureFormat::R8G8B8_SSCALED:
			case GenericTextureFormat::B8G8R8_SSCALED:
			case GenericTextureFormat::R8G8_SSCALED:
			case GenericTextureFormat::R8_SSCALED:
			case GenericTextureFormat::R8G8B8A8_SINT:
			case GenericTextureFormat::B8G8R8A8_SINT:
			case GenericTextureFormat::A8B8G8R8_SINT_PACK32:
			case GenericTextureFormat::R8G8B8_SINT:
			case GenericTextureFormat::B8G8R8_SINT:
			case GenericTextureFormat::R8G8_SINT:
			case GenericTextureFormat::R8_SINT:
				return std::numeric_limits<int8_t>::min();

			case GenericTextureFormat::A2R10G10B10_UNORM_PACK32:
			case GenericTextureFormat::A2R10G10B10_USCALED_PACK32:
			case GenericTextureFormat::A2R10G10B10_UINT_PACK32:
			case GenericTextureFormat::A2B10G10R10_UNORM_PACK32:
			case GenericTextureFormat::A2B10G10R10_USCALED_PACK32:
			case GenericTextureFormat::A2B10G10R10_UINT_PACK32:
			case GenericTextureFormat::X8_D24_UNORM_PACK32:
			case GenericTextureFormat::D24_UNORM_S8_UINT:
			case GenericTextureFormat::D16_UNORM_S8_UINT:
			case GenericTextureFormat::R4G4B4A4_UNORM_PACK16:
			case GenericTextureFormat::B4G4R4A4_UNORM_PACK16:
			case GenericTextureFormat::R4G4_UNORM_PACK8:
			case GenericTextureFormat::R5G6B5_UNORM_PACK16:
			case GenericTextureFormat::B5G6R5_UNORM_PACK16:
			case GenericTextureFormat::R5G5B5A1_UNORM_PACK16:
			case GenericTextureFormat::B5G5R5A1_UNORM_PACK16:
			case GenericTextureFormat::A1R5G5B5_UNORM_PACK16:
//			case GenericTextureFormat::B10G11R11_UFLOAT_PACK32:
//			case GenericTextureFormat::E5B9G9R9_UFLOAT_PACK32:
				return 0.0;

			case GenericTextureFormat::UNDEFINED:
			case GenericTextureFormat::BC2_UNORM_BLOCK:
			case GenericTextureFormat::BC2_SRGB_BLOCK:
			case GenericTextureFormat::BC3_UNORM_BLOCK:
			case GenericTextureFormat::BC3_SRGB_BLOCK:
			case GenericTextureFormat::BC5_UNORM_BLOCK:
			case GenericTextureFormat::BC6H_UFLOAT_BLOCK:
			case GenericTextureFormat::BC7_UNORM_BLOCK:
			case GenericTextureFormat::BC7_SRGB_BLOCK:
			case GenericTextureFormat::BC1_RGB_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGB_SRGB_BLOCK:
			case GenericTextureFormat::BC1_RGBA_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGBA_SRGB_BLOCK:
			case GenericTextureFormat::BC4_UNORM_BLOCK:
				return 0.0;

			case GenericTextureFormat::BC4_SNORM_BLOCK:
			case GenericTextureFormat::BC5_SNORM_BLOCK:
				return 0.0; // TODO

			default:
				LOG_S(ERROR) << "min " << name(fmt_) << " not handled";
				return 0;
		}
	}
	constexpr static auto min(GenericTextureFormat const fmt_, Channel channel_) -> double
	{
		return GtfCracker::min(fmt_, swizzle(fmt_,channel_));
	}

	constexpr static auto swizzleFormat(GenericTextureFormat const fmt_) -> SwizzleFormat
	{
		switch(fmt_)
		{
			case GenericTextureFormat::UNDEFINED:
			case GenericTextureFormat::R64G64B64A64_UINT:
			case GenericTextureFormat::R64G64B64_UINT:
			case GenericTextureFormat::R64G64_UINT:
			case GenericTextureFormat::R64_UINT:
			case GenericTextureFormat::R64G64B64A64_SINT:
			case GenericTextureFormat::R64G64B64_SINT:
			case GenericTextureFormat::R64G64_SINT:
			case GenericTextureFormat::R64_SINT:
			case GenericTextureFormat::R64G64B64_SFLOAT:
			case GenericTextureFormat::R64G64B64A64_SFLOAT:
			case GenericTextureFormat::R64G64_SFLOAT:
			case GenericTextureFormat::R64_SFLOAT:
			case GenericTextureFormat::R32G32B32A32_UINT:
			case GenericTextureFormat::R32G32B32_UINT:
			case GenericTextureFormat::R32G32_UINT:
			case GenericTextureFormat::R32_UINT:
			case GenericTextureFormat::R32G32B32A32_SINT:
			case GenericTextureFormat::R32G32B32_SINT:
			case GenericTextureFormat::R32G32_SINT:
			case GenericTextureFormat::R32_SINT:
			case GenericTextureFormat::R32G32B32A32_SFLOAT:
			case GenericTextureFormat::R32G32B32_SFLOAT:
			case GenericTextureFormat::R32G32_SFLOAT:
			case GenericTextureFormat::R32_SFLOAT:
			case GenericTextureFormat::D32_SFLOAT:
			case GenericTextureFormat::R16G16B16A16_UNORM:
			case GenericTextureFormat::R16G16B16_UNORM:
			case GenericTextureFormat::R16G16_UNORM:
			case GenericTextureFormat::R16_UNORM:
			case GenericTextureFormat::D16_UNORM:
			case GenericTextureFormat::R16G16B16A16_USCALED:
			case GenericTextureFormat::R16G16B16_USCALED:
			case GenericTextureFormat::R16G16_USCALED:
			case GenericTextureFormat::R16_USCALED:
			case GenericTextureFormat::R16G16B16A16_UINT:
			case GenericTextureFormat::R16G16B16_UINT:
			case GenericTextureFormat::R16G16_UINT:
			case GenericTextureFormat::R16_UINT:
			case GenericTextureFormat::R16G16B16A16_SNORM:
			case GenericTextureFormat::R16G16B16_SNORM:
			case GenericTextureFormat::R16G16_SNORM:
			case GenericTextureFormat::R16_SNORM:
			case GenericTextureFormat::R16G16B16A16_SSCALED:
			case GenericTextureFormat::R16G16B16_SSCALED:
			case GenericTextureFormat::R16G16_SSCALED:
			case GenericTextureFormat::R16_SSCALED:
			case GenericTextureFormat::R16G16B16A16_SINT:
			case GenericTextureFormat::R16G16B16_SINT:
			case GenericTextureFormat::R16G16_SINT:
			case GenericTextureFormat::R16_SINT:
			case GenericTextureFormat::R16G16B16A16_SFLOAT:
			case GenericTextureFormat::R16G16B16_SFLOAT:
			case GenericTextureFormat::R16G16_SFLOAT:
			case GenericTextureFormat::R16_SFLOAT:
			case GenericTextureFormat::D32_SFLOAT_S8_UINT:
			case GenericTextureFormat::R8G8B8A8_UNORM:
			case GenericTextureFormat::R8G8B8A8_USCALED:
			case GenericTextureFormat::R8G8B8A8_UINT:
			case GenericTextureFormat::R8G8B8_USCALED:
			case GenericTextureFormat::R8G8B8_UNORM:
			case GenericTextureFormat::R8G8B8_UINT:
			case GenericTextureFormat::R8G8B8A8_SRGB:
			case GenericTextureFormat::R8G8_UNORM:
			case GenericTextureFormat::R8G8_USCALED:
			case GenericTextureFormat::R8_USCALED:
			case GenericTextureFormat::R8_UNORM:
			case GenericTextureFormat::R8G8_UINT:
			case GenericTextureFormat::R8_UINT:
			case GenericTextureFormat::S8_UINT:
			case GenericTextureFormat::R8G8B8_SRGB:
			case GenericTextureFormat::R8G8_SRGB:
			case GenericTextureFormat::R8_SRGB:
			case GenericTextureFormat::R8G8B8A8_SNORM:
			case GenericTextureFormat::R8G8B8_SNORM:
			case GenericTextureFormat::R8G8_SNORM:
			case GenericTextureFormat::R8_SNORM:
			case GenericTextureFormat::R8G8B8A8_SSCALED:
			case GenericTextureFormat::R8G8_SSCALED:
			case GenericTextureFormat::R8_SSCALED:
			case GenericTextureFormat::R8G8B8A8_SINT:
			case GenericTextureFormat::R8G8B8_SINT:
			case GenericTextureFormat::R8G8_SINT:
			case GenericTextureFormat::R8_SINT:
			case GenericTextureFormat::R8G8B8_SSCALED:
			case GenericTextureFormat::R4G4B4A4_UNORM_PACK16:
			case GenericTextureFormat::X8_D24_UNORM_PACK32:
			case GenericTextureFormat::D24_UNORM_S8_UINT:
			case GenericTextureFormat::D16_UNORM_S8_UINT:
			case GenericTextureFormat::R4G4_UNORM_PACK8:
			case GenericTextureFormat::R5G6B5_UNORM_PACK16:
			case GenericTextureFormat::R5G5B5A1_UNORM_PACK16:
				return RGBA;

			case GenericTextureFormat::A2R10G10B10_UNORM_PACK32:
			case GenericTextureFormat::A2R10G10B10_USCALED_PACK32:
			case GenericTextureFormat::A2R10G10B10_UINT_PACK32:
			case GenericTextureFormat::A1R5G5B5_UNORM_PACK16:
				return ARGB;

			case GenericTextureFormat::A2B10G10R10_UNORM_PACK32:
			case GenericTextureFormat::A2B10G10R10_USCALED_PACK32:
			case GenericTextureFormat::A2B10G10R10_UINT_PACK32:
			case GenericTextureFormat::A8B8G8R8_SRGB_PACK32:
			case GenericTextureFormat::A8B8G8R8_USCALED_PACK32:
			case GenericTextureFormat::A8B8G8R8_UNORM_PACK32:
			case GenericTextureFormat::A8B8G8R8_UINT_PACK32:
			case GenericTextureFormat::A8B8G8R8_SNORM_PACK32:
			case GenericTextureFormat::A8B8G8R8_SSCALED_PACK32:
			case GenericTextureFormat::A8B8G8R8_SINT_PACK32:
//			case GenericTextureFormat::E5B9G9R9_UFLOAT_PACK32:
				return ABGR;

			case GenericTextureFormat::B8G8R8A8_UNORM:
			case GenericTextureFormat::B8G8R8A8_USCALED:
			case GenericTextureFormat::B8G8R8A8_UINT:
			case GenericTextureFormat::B8G8R8A8_SRGB:
			case GenericTextureFormat::B8G8R8A8_SNORM:
			case GenericTextureFormat::B8G8R8A8_SSCALED:
			case GenericTextureFormat::B8G8R8A8_SINT:
			case GenericTextureFormat::B4G4R4A4_UNORM_PACK16:
			case GenericTextureFormat::B5G5R5A1_UNORM_PACK16:
				return BGRA;

			case GenericTextureFormat::B8G8R8_SNORM:
			case GenericTextureFormat::B8G8R8_UNORM:
			case GenericTextureFormat::B8G8R8_USCALED:
			case GenericTextureFormat::B8G8R8_UINT:
			case GenericTextureFormat::B8G8R8_SSCALED:
			case GenericTextureFormat::B8G8R8_SINT:
			case GenericTextureFormat::B5G6R5_UNORM_PACK16:
			case GenericTextureFormat::B8G8R8_SRGB:
//			case GenericTextureFormat::B10G11R11_UFLOAT_PACK32:
				return BGRA;

			case GenericTextureFormat::BC2_UNORM_BLOCK:
			case GenericTextureFormat::BC2_SRGB_BLOCK:
			case GenericTextureFormat::BC3_UNORM_BLOCK:
			case GenericTextureFormat::BC3_SRGB_BLOCK:
			case GenericTextureFormat::BC5_UNORM_BLOCK:
			case GenericTextureFormat::BC5_SNORM_BLOCK:
			case GenericTextureFormat::BC6H_UFLOAT_BLOCK:
			case GenericTextureFormat::BC6H_SFLOAT_BLOCK:
			case GenericTextureFormat::BC7_UNORM_BLOCK:
			case GenericTextureFormat::BC7_SRGB_BLOCK:
			case GenericTextureFormat::BC1_RGB_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGB_SRGB_BLOCK:
			case GenericTextureFormat::BC1_RGBA_UNORM_BLOCK:
			case GenericTextureFormat::BC1_RGBA_SRGB_BLOCK:
			case GenericTextureFormat::BC4_UNORM_BLOCK:
			case GenericTextureFormat::BC4_SNORM_BLOCK:
				return RGBA;
			default:
				LOG_S(ERROR) << "swizzleFormat " << name(fmt_) << " not handled";
				return RGBA;
		}
	}

};
} // end namespace
#endif