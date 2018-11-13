#pragma once
#ifndef WYRD_MATH_COLOURSPACE_H
#define WYRD_MATH_COLOURSPACE_H

#include "core/core.h"
#include "math/scalar_math.h"
#include "cmath"

namespace Math {

constexpr auto LinearRGB2sRGB_channel( float x_ ) -> float
{
	float x = Math::Clamp(x_, 0.0f, 255.0f);
	float const a = 0.055f;

	if(x <= 0.0031308f)
	{
		x = x * 12.92f;
	} else
	{
		x = (1.0f + a) * std::pow(x, 1.0f/2.4f) - a;
	}

	return x;
}

constexpr auto sRGB2LinearRGB_channel( float x_ ) -> float
{
	float x = Math::Clamp(x_, 0.0f, 255.0f);
	float const a = 0.055f;

	if(x_ <= 0.04045f)
	{
		x = x / 12.92f;
	} else
	{
		x = std::pow((x + a) / (1.0f + a), 2.4f);
	}
	return x;
}


constexpr auto LinearRGB2sRGB( float r_, float g_, float b_ ) -> std::tuple<float, float, float>
{
	return { LinearRGB2sRGB_channel(r_), LinearRGB2sRGB_channel(g_), LinearRGB2sRGB_channel(b_) };
}

constexpr auto sRGB2LinearRGB( float r_, float g_, float b_ ) -> std::tuple<float, float, float>
{
	return { sRGB2LinearRGB_channel(r_), sRGB2LinearRGB_channel(g_), sRGB2LinearRGB_channel(b_) };
}

}

#endif //WYRD_MATH_COLOURSPACE_H
