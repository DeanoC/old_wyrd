#include "../catch.hpp"

#include "core/core.h"
#include "render/generictextureformat.h"
#include "render/gtfcracker.h"
#include "render/image.h"

auto ImageTester(size_t w_, size_t h_, size_t d_, size_t s_, Render::GenericTextureFormat fmt_, bool doLog_) -> void
{
	using namespace Render;
	using namespace Catch::literals;
	if(fmt_ == GenericTextureFormat::UNDEFINED) return;

	auto img = GenericImage::Create(w_, h_, d_, s_, fmt_);
	if(doLog_)
	{
		LOG_S(INFO) << "Testing " << GtfCracker::name(fmt_) << " @ "
					<< w_ << " x " << h_ << " x " << d_ << " x " << s_;
	}
	REQUIRE(img);
	REQUIRE(img->width == w_);
	REQUIRE(img->height == h_);
	REQUIRE(img->depth == d_);
	REQUIRE(img->slices == s_);
	REQUIRE(img->format == fmt_);
	REQUIRE(img->dataSize == (w_ * h_ * d_ * s_ * (GtfCracker::bitWidth(fmt_) / 8)));
	REQUIRE(img->data() != nullptr);
	for(auto i = 0u; i < img->dataSize; ++i)
	{
		REQUIRE(img->data()[i] == 0);
	}

	double const mins[4] = {
			GtfCracker::min(img->format, Channel::R),
			GtfCracker::min(img->format, Channel::G),
			GtfCracker::min(img->format, Channel::B),
			GtfCracker::min(img->format, Channel::A)
	};
	double const maxs[4] = {
			GtfCracker::max(img->format, Channel::R),
			GtfCracker::max(img->format, Channel::G),
			GtfCracker::max(img->format, Channel::B),
			GtfCracker::max(img->format, Channel::A)
	};

	// special handling
	// this formats will emit failures due to 'lossy'ness over the ranage

	// skip block compressed testing the pixel read/write tests
	if(GtfCracker::isCompressed(img->format)) return;

	// sRGB is a form of lossy compression need to rethink tests in this case
	if(GtfCracker::isSRGB(img->format)) return;

	// small floats are also lossy over the ranges we test against
	if(GtfCracker::isFloat(img->format) && GtfCracker::channelBitWidth(img->format, Channel::R) <= 16) return;

	// mixed normalised / unnormalised format which the tester doesn't handle yet
	if(img->format == GenericTextureFormat::D24_UNORM_S8_UINT) return;
	if(img->format == GenericTextureFormat::D16_UNORM_S8_UINT) return;

	for(auto s = 0u; s < img->slices; ++s)
	{
		for(auto z = 0u; z < img->depth; ++z)
		{
			for(auto y = 0u; y < img->height; ++y)
			{
				for(auto x = 0u; x < img->width; ++x)
				{
					GenericImage::Pixel pixel = {double(x), double(y), double(z), double(s)};
					if(GtfCracker::isSigned(img->format))
					{
						pixel.r = mins[0] + pixel.r;
						pixel.g = mins[1] + pixel.g;
						pixel.b = mins[2] + pixel.b;
						pixel.a = mins[3] + pixel.a;
					}

					pixel.clamp(mins, maxs);

					if(GtfCracker::isNormalised(img->format))
					{
						pixel.r = pixel.r / maxs[0];
						pixel.g = pixel.g / maxs[1];
						pixel.b = pixel.b / maxs[2];
						pixel.a = pixel.a / maxs[3];
					}
					if(doLog_)
					{
						LOG_S(INFO) << "set<RGBA> " << x << " " <<   y << " " <<   z << " " <<   s << " = "
									<< pixel.r << " " << pixel.g << " " << pixel.b << " " << pixel.a;
					}
					img->setPixelAt(pixel, x, y, z, s);
				}
			}
		}
	}
	for(auto s = 0u; s < img->slices; ++s)
	{

		for(auto z = 0u; z < img->depth; ++z)
		{

			for(auto y = 0u; y < img->height; ++y)
			{

				for(auto x = 0u; x < img->width; ++x)
				{

					auto v = img->pixelAt(x, y, z, s);
					if(doLog_)
					{
						LOG_S(INFO) << "get<RGBA>" << x << " " <<   y << " " <<   z << " " <<   s << " = "
									<< v.r << " " << v.g << " " << v.b << " " << v.a;
					}


					GenericImage::Pixel pixel = {double(x), double(y), double(z), double(s)};
					if(GtfCracker::isSigned(img->format))
					{
						pixel.r = mins[0] + pixel.r;
						pixel.g = mins[1] + pixel.g;
						pixel.b = mins[2] + pixel.b;
						pixel.a = mins[3] + pixel.a;
					}

					pixel.clamp(mins, maxs);

					if(GtfCracker::isNormalised(img->format))
					{
						pixel.r = pixel.r / maxs[0];
						pixel.g = pixel.g / maxs[1];
						pixel.b = pixel.b / maxs[2];
						pixel.a = pixel.a / maxs[3];
					}
					// delibrate fallthrough
					switch(GtfCracker::channelCount(fmt_))
					{
						case 4: REQUIRE(v.a == Approx(pixel.a));
						case 3: REQUIRE(v.b == Approx(pixel.b));
						case 2: REQUIRE(v.g == Approx(pixel.g));
						case 1: REQUIRE(v.r == Approx(pixel.r));
							break;
						default:
							REQUIRE(GtfCracker::channelCount(fmt_) <= 4);
					}
				}
			}
		}
	}
}

#define GENERIC_IMAGE_TEST_CASE(FMT, W, H, D, S, DOLOG) \
TEST_CASE( "Image 2D "#FMT" "#W"_"#H"_"#D"_"#S, "[render]" ) \
{ \
    using namespace Render; \
    ImageTester((W), (H), (D), (S), GenericTextureFormat:: FMT, DOLOG); \
}
#define DO_BASIC_TESTS 1

#if DO_BASIC_TESTS == 1

// basic suffix tests
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_UINT, 10, 1, 1, 1, false)
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_SINT, 10, 1, 1, 1, false)
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_USCALED, 10, 1, 1, 1, false)
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_SSCALED, 10, 1, 1, 1, false)
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_UNORM, 10, 1, 1, 1, false)
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_SNORM, 10, 1, 1, 1, false)

// basic dimension tests
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_UINT, 10, 10, 1, 1, false)
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_UINT, 10, 10, 10, 1, false)
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_UINT, 10, 10, 10, 10, false)
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_UINT, 10, 1, 10, 1, false)
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_UINT, 10, 1, 1, 10, false)
GENERIC_IMAGE_TEST_CASE(R8G8B8A8_UINT, 10, 10, 1, 10, false)

#endif

// format tests
#define TEST_ALL_FORMATS 0
#if TEST_ALL_FORMATS == 1
GENERIC_IMAGE_TEST_CASE(D24_UNORM_S8_UINT, 16, 1, 1, 1, true)
#else
#define GTF_START_MACRO
#define GTF_MOD_MACRO(x) GENERIC_IMAGE_TEST_CASE(x, 256, 2, 2, 2, false)
#define GTF_END_MACRO
#include "render/generictextureformat.h"
#endif

//GENERIC_IMAGE_TEST_CASE(R4G4_UNORM_PACK8, 10, 10, 1, 1, true)
