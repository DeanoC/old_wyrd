#include "../catch.hpp"

#include "core/core.h"
#include "render/generictextureformat.h"
#include "render/gtfcracker.h"

TEST_CASE("GTFCracker isSigned", "[render]")
{
	using namespace Render;
#define GTF_START_MACRO std::vector<std::pair<std::string_view,bool>> names;
#define GTF_MOD_MACRO(x) \
    names.emplace_back(GtfCracker::name(GenericTextureFormat::x), GtfCracker::isSigned(GenericTextureFormat::x));
#define GTF_END_MACRO

#include "render/generictextureformat.h"

	for(auto const name : names)
	{
		if(false)
		{ LOG_S(INFO) << name.first; }
		if(name.second == true)
		{
			// special case the D32_S8 formats cos well they are mixed float/int
			// sign/unsigned formats which break my normal rule of no mixing.
			if(name.first.find("S8_UINT") == std::string_view::npos)
			{
				REQUIRE(name.first.find("_UINT") == std::string_view::npos);
			}
			REQUIRE(name.first.find("_USCALED") == std::string_view::npos);
			REQUIRE(name.first.find("_UNORM") == std::string_view::npos);
			REQUIRE(name.first.find("_SRGB") == std::string_view::npos);
			REQUIRE(name.first.find("_UFLOAT") == std::string_view::npos);
		} else
		{
			REQUIRE(name.first.find("_SINT") == std::string_view::npos);
			REQUIRE(name.first.find("_SSCALED") == std::string_view::npos);
			REQUIRE(name.first.find("_SNORM") == std::string_view::npos);
			REQUIRE(name.first.find("_SFLOAT") == std::string_view::npos);
		}
	}
}

TEST_CASE("GTFCracker isDepthStencil", "[render]")
{
	using namespace Render;
#define GTF_START_MACRO std::vector<std::tuple<std::string_view,bool, bool, bool>> names;
#define GTF_MOD_MACRO(x) \
    names.emplace_back(GtfCracker::name(GenericTextureFormat::x), \
    GtfCracker::isDepth(GenericTextureFormat::x), \
    GtfCracker::isStencil(GenericTextureFormat::x), \
    GtfCracker::isDepthStencil(GenericTextureFormat::x));

#define GTF_END_MACRO

#include "render/generictextureformat.h"

	for(auto const tup : names)
	{
		auto const[name, depth, stencil, depthstencil] = tup;

		if(false)
		{ LOG_S(INFO) << name; }
		if(depth == true)
		{
			REQUIRE((name.find("D16") != std::string_view::npos || name.find("D24") != std::string_view::npos ||
					 name.find("D32") != std::string_view::npos) == true);
		} else
		{
			REQUIRE((name.find("D16") != std::string_view::npos || name.find("D24") != std::string_view::npos ||
					 name.find("D32") != std::string_view::npos) == false);
		}
		if(stencil == true)
		{
			REQUIRE((name.find("S8") != std::string_view::npos) == true);
		} else
		{
			REQUIRE((name.find("S8") != std::string_view::npos) == false);
		}

		if(depthstencil == true)
		{
			REQUIRE((depth == true && stencil == true));
			REQUIRE(((name.find("D16") != std::string_view::npos || name.find("D24") != std::string_view::npos ||
					  name.find("D32") != std::string_view::npos) &&
					 (name.find("S8") != std::string_view::npos) == true));
		}
	}
}

TEST_CASE("GTFCracker isCompressed", "[render]")
{
	using namespace Render;
#define GTF_START_MACRO std::vector<std::pair<std::string_view,bool>> names;
#define GTF_MOD_MACRO(x) \
    names.emplace_back(GtfCracker::name(GenericTextureFormat::x), GtfCracker::isCompressed(GenericTextureFormat::x));
#define GTF_END_MACRO

#include "render/generictextureformat.h"

	for(auto const name : names)
	{
		if(false)
		{ LOG_S(INFO) << name.first; }
		if(name.second == true)
		{
			REQUIRE(name.first.find("_BLOCK") != std::string_view::npos);
		} else
		{
			REQUIRE(name.first.find("_BLOCK") == std::string_view::npos);
		}
	}
}

TEST_CASE("GTFCracker isFloat", "[render]")
{
	using namespace Render;
#define GTF_START_MACRO std::vector<std::pair<std::string_view,bool>> names;
#define GTF_MOD_MACRO(x) \
    names.emplace_back(GtfCracker::name(GenericTextureFormat::x), GtfCracker::isFloat(GenericTextureFormat::x));
#define GTF_END_MACRO

#include "render/generictextureformat.h"

	for(auto const name : names)
	{
		if(false)
		{ LOG_S(INFO) << name.first; }
		if(name.second == true)
		{
			REQUIRE(name.first.find("FLOAT") != std::string_view::npos);
		} else
		{
			REQUIRE(name.first.find("FLOAT") == std::string_view::npos);
		}
	}
}

TEST_CASE("GTFCracker isNormalised", "[render]")
{
	using namespace Render;
#define GTF_START_MACRO std::vector<std::pair<std::string_view,bool>> names;
#define GTF_MOD_MACRO(x) \
    names.emplace_back(GtfCracker::name(GenericTextureFormat::x), GtfCracker::isNormalised(GenericTextureFormat::x));
#define GTF_END_MACRO

#include "render/generictextureformat.h"

	for(auto const name : names)
	{
		if(false)
		{ LOG_S(INFO) << name.first; }
		if(name.second == true)
		{
			REQUIRE(name.first.find("NORM") != std::string_view::npos);
		} else
		{
			REQUIRE(name.first.find("NORM") == std::string_view::npos);
		}
	}
}
TEST_CASE("GTFCracker isSRGB", "[render]")
{
	using namespace Render;
#define GTF_START_MACRO std::vector<std::pair<std::string_view,bool>> names;
#define GTF_MOD_MACRO(x) \
    names.emplace_back(GtfCracker::name(GenericTextureFormat::x), GtfCracker::isSRGB(GenericTextureFormat::x));
#define GTF_END_MACRO

#include "render/generictextureformat.h"

	for(auto const name : names)
	{
		if(false)
		{ LOG_S(INFO) << name.first; }
		if(name.second == true)
		{
			REQUIRE(name.first.find("SRGB") != std::string_view::npos);
		} else
		{
			REQUIRE(name.first.find("SRGB") == std::string_view::npos);
		}
	}
}


TEST_CASE("GenericTextureFormat", "[render]")
{
	using namespace Render;
	// just a sampling of various format not extensive
	REQUIRE_FALSE(GtfCracker::isDepthStencil(GenericTextureFormat::A8B8G8R8_UNORM_PACK32));
	REQUIRE(GtfCracker::isDepthStencil(GenericTextureFormat::D24_UNORM_S8_UINT));
	REQUIRE(GtfCracker::channelCount(GenericTextureFormat::D24_UNORM_S8_UINT) == 2);
	REQUIRE(GtfCracker::channelCount(GenericTextureFormat::A8B8G8R8_UNORM_PACK32) == 4);
	REQUIRE(GtfCracker::channelBitWidth(GenericTextureFormat::D24_UNORM_S8_UINT, 0) == 24);
	REQUIRE(GtfCracker::channelBitWidth(GenericTextureFormat::D24_UNORM_S8_UINT, 1) == 8);
	REQUIRE(GtfCracker::channelBitWidth(GenericTextureFormat::A8B8G8R8_UNORM_PACK32) == 8);
	REQUIRE_FALSE(GtfCracker::isCompressed(GenericTextureFormat::A8B8G8R8_UNORM_PACK32));
	REQUIRE(GtfCracker::isCompressed(GenericTextureFormat::BC1_RGB_UNORM_BLOCK));

	for(auto i = 0u; i < GenericTextureFormatEnumCount; ++i)
	{
		if(i == 0) continue; // ignore undefined
		REQUIRE(GtfCracker::channelCount((GenericTextureFormat) i));
		REQUIRE(GtfCracker::channelBitWidth((GenericTextureFormat) i));
		REQUIRE(GtfCracker::bitWidth((GenericTextureFormat) i));
	}
}