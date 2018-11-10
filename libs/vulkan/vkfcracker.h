#pragma once
#ifndef WYRD_VULKAN_VKFORMATCRACKER_H
#define WYRD_VULKAN_VKFORMATCRACKER_H

#include "core/core.h"
#include "render/generictextureformat.h"
#include "render/gtfcracker.h"
#include "vulkan/api.h"

namespace Vulkan {

struct VkfCracker
{
	static constexpr auto toGeneric( VkFormat const fmt_ ) -> Render::GenericTextureFormat
	{
		Render::GenericTextureFormat generic = Render::GenericTextureFormat::UNDEFINED;

#define GTF_START_MACRO switch(fmt_) { default: break;
#define GTF_MOD_MACRO( x ) case VK_FORMAT_##x: generic = Render::GenericTextureFormat::x; break;
#define GTF_END_MACRO }

#include "render/generictextureformat.h"

		return generic;
	}

	static constexpr auto fromGeneric( Render::GenericTextureFormat const fmt_ ) -> VkFormat
	{
		VkFormat generic = VK_FORMAT_UNDEFINED;

#define GTF_START_MACRO switch(fmt_) { default: break;
#define GTF_MOD_MACRO(x) case Render::GenericTextureFormat::x: generic =  VK_FORMAT_##x; break;
#define GTF_END_MACRO }

#include "render/generictextureformat.h"

		return generic;
	}

	constexpr static auto getName( VkFormat const fmt_ ) -> std::string_view
	{
#define GTF_START_MACRO switch(fmt_) { default: break;
#define GTF_MOD_MACRO(x) case VK_FORMAT_##x: return "VK_FORMAT_"#x;
#define GTF_END_MACRO \
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return "VK_FORMAT_A2R10G10B10_SNORM_PACK32"; \
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32"; \
		case VK_FORMAT_A2R10G10B10_SINT_PACK32: return "VK_FORMAT_A2R10G10B10_SINT_PACK32"; \
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return "VK_FORMAT_A2B10G10R10_SNORM_PACK32"; \
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32"; \
		case VK_FORMAT_A2B10G10R10_SINT_PACK32: return "VK_FORMAT_A2B10G10R10_SINT_PACK32"; \
		};
#include "render/generictextureformat.h"
		// TODO handle the rest of the vk textures names for debugging purposes
		return "";
	}

	constexpr static auto isDepth(VkFormat const fmt_) -> bool
	{
		// vulkan only formats
		/*		switch(fmt_)
				{
					default: break;
				}*/
		return Render::GtfCracker::isDepth(toGeneric(fmt_));
	}
	constexpr static auto isStencil( VkFormat  const fmt_ ) -> bool
	{
		// vulkan only formats
		/*		switch(fmt_)
				{
					default: break;
				}*/
		return Render::GtfCracker::isStencil(toGeneric(fmt_));
	}
	constexpr static auto isDepthStencil( VkFormat const fmt_ ) -> bool
	{
		// vulkan only formats
		/*		switch(fmt_)
				{
					default: break;
				}*/
		return Render::GtfCracker::isDepthStencil(toGeneric(fmt_));
	}

	constexpr static auto isCompressed( VkFormat const fmt_ ) -> unsigned int
	{
		// vulkan only formats
		/*		switch(fmt_)
				{
					default: break;
				}*/
		return Render::GtfCracker::isCompressed(toGeneric(fmt_));
	}
	constexpr static auto isFloat( VkFormat const fmt_ ) -> bool
	{
		// vulkan only formats
		/*		switch(fmt_)
				{
					default: break;
				}*/
		return Render::GtfCracker::isFloat(toGeneric(fmt_));
	}
	constexpr static auto isNormalised( VkFormat const fmt_ ) -> bool
	{
		// vulkan only formats
		switch(fmt_)
		{
			case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
				return true;
			default: break;
		}
		return Render::GtfCracker::isNormalised(toGeneric(fmt_));
	}

	constexpr static auto channelCount( VkFormat const fmt_ ) -> unsigned int
	{
		// vulkan only formats
		switch(fmt_)
		{
			case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
			case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
			case VK_FORMAT_A2R10G10B10_SINT_PACK32:
			case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
			case VK_FORMAT_A2B10G10R10_SINT_PACK32:
				return 4;
			default: break;
		}
		return Render::GtfCracker::channelCount(toGeneric(fmt_));
	}
	constexpr static auto channelBitWidth( VkFormat const fmt_, int channel_ = 0 ) -> unsigned int
	{
		// vulkan only formats
		switch(fmt_)
		{
			case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
			case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
			case VK_FORMAT_A2R10G10B10_SINT_PACK32:
			case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
			case VK_FORMAT_A2B10G10R10_SINT_PACK32:
				if(channel_ == 0) return 2;
				else return 10;
			default: break;
		}
		return Render::GtfCracker::channelBitWidth(toGeneric(fmt_), channel_);
	}
	constexpr static auto bitWidth( VkFormat const fmt_ ) -> unsigned int
	{
		// vulkan only formats
		switch(fmt_)
		{
			case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
			case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
			case VK_FORMAT_A2R10G10B10_SINT_PACK32:
			case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
			case VK_FORMAT_A2B10G10R10_SINT_PACK32:
				return 32;
			default: break;
		}
		return Render::GtfCracker::bitWidth(toGeneric(fmt_));
	}


};

}
#endif //WYRD_VKFORMATCRACKER_H
