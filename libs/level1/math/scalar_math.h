//
// Created by Computer on 23/07/2018.
//

#ifndef CORE_SCALAR_MATH_H
#define CORE_SCALAR_MATH_H

#include "core/core.h"
#include <cmath>
#include <cstdlib>
#include <limits>

#if PLATFORM == WINDOWS
#if COMPILER_VERSION < MS_VS2017 && !defined( MSC_FLOAT_HACK )
#define MSC_FLOAT_HACK

namespace std {
    inline bool isnan( float a ) { return _isnan( a ) != 0; }
    inline bool isnan( double a ) { return _isnan( a ) != 0; }
    inline bool isfinite( float a ) { return _finite( a ) != 0; }
    inline bool isfinite( double a ) { return _finite( a ) != 0; }
    inline bool isnormal( float a )
    {
        switch( _fpclass( a ) )
        {
            case _FPCLASS_NN:
            case _FPCLASS_NZ:
            case _FPCLASS_PZ:
            case _FPCLASS_PN:
                return true;

            default:
                return false;
        }
    }
    inline bool isnormal( double a )
    {
        switch( _fpclass( a ) )
        {
            case _FPCLASS_NN:
            case _FPCLASS_NZ:
            case _FPCLASS_PZ:
            case _FPCLASS_PN:
                return true;

            default:
                return false;
        }
    }
} // namespace Core
#endif // def !VC2017
#endif // end WINDOWS

//! contains the maths constants stuff
//! originally from a boost library in the vault, modified for my use as it no longer seems supported

namespace Math {

template<typename T, typename = typename std::enable_if<std::is_floating_point<T>{}>::type>
static constexpr auto pi() -> T { return T(3.14159265358979323846264338327950L); }

template<typename T, typename = typename std::enable_if<std::is_floating_point<T>{}>::type>
static constexpr auto pi_over_2() -> T { return pi<T>() / T(2); }

template<typename T>
static constexpr auto square(T const val_) -> T { return val_ * val_; }

template<typename T, typename = typename std::enable_if<std::is_floating_point<T>{}>::type>
static constexpr auto degreesToRadians(T const val_) -> T
{
	return val_ * (pi<T>() / T(180));
}

template<typename T, typename = typename std::enable_if<std::is_floating_point<T>{}>::type>
static constexpr auto radiansToDegrees(T const val_) -> T
{
	return (T(180) * val_) / pi<T>();
}

template<typename T, typename = typename std::enable_if<std::is_floating_point<T>{}>::type>
constexpr auto ApproxEqual(T const a_, T const b_, T const eps_ = T(1e-5)) -> bool
{
	return std::fabs(a_ - b_) < eps_;
}

template<typename T, typename = typename std::enable_if<std::is_floating_point<T>{}>::type>
CALL constexpr T Reciprocal(T a) { return T(1.0) / a; }

template<typename T>
CALL constexpr int Sign(T val) { return (T(0) < val) - (val < T(0)); }


//! Length^2 of a 1D Vector for orthogonality
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>{}>::type>
CALL constexpr T LengthSquared(T a) { return a * a; }

//! Length of a 1D Vector for orthogonality
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>{}>::type>
CALL constexpr T Length(T a) { return a; }

template<typename T, typename = typename std::enable_if<std::is_floating_point<T>{}>::type>
CALL constexpr T ReciprocalSqrt(T a) { return T(1.0) / std::sqrt(a); }

template<typename T>
CALL constexpr T Max(const T a, const T b) { return (a > b) ? a : b; }

template<typename T>
CALL constexpr T Min(const T a, const T b) { return (a < b) ? a : b; }

template<typename T>
CALL constexpr T 	Clamp(const T a, const T mi, const T ma) { return Max(mi, Min(a, ma)); }

constexpr uint8_t s_LogTable256[] = {0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
									 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
									 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
									 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
									 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
									 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
									 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
									 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
									 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
									 7, 7, 7, 7};


/// \brief	return Log2 of v.
/// return log2 of an int. this is equivalent to finding the highest bit that has been set
/// or the number to shift 1 left by to get the nearest lower power of 2
/// \param	v	The number to get the log2 of.
/// \return	log2(v).
CALL constexpr unsigned int log2(unsigned int v)
{
	unsigned int r = 0;     // r will be lg(v)
	unsigned int t = 0, tt = 0; // temporaries

	if((tt = v >> 16) != 0)
	{
		r = ((t = tt >> 8) != 0) ? 24 + ((unsigned int) s_LogTable256[t]) : 16 + ((unsigned int) s_LogTable256[tt]);
	} else
	{
		r = ((t = v >> 8) != 0) ? 8 + ((unsigned int) s_LogTable256[t]) : ((unsigned int) s_LogTable256[v]);
	}
	return r;
}

// From Chunk Walbourns code from DirectXTexConvert.cpp
// e5b9g9r9 are positive only shared exponent float formats
inline uint32_t floats2e5b9g9r9(float const in_[3])
{

	constexpr float maxf9 = float(0x1FF << 7);
	constexpr float minf9 = float(1.f / (1 << 16));

	float x = (in_[0] >= 0.f) ? ((in_[0] > maxf9) ? maxf9 : in_[0]) : 0.f;
	float y = (in_[1] >= 0.f) ? ((in_[1] > maxf9) ? maxf9 : in_[1]) : 0.f;
	float z = (in_[2] >= 0.f) ? ((in_[2] > maxf9) ? maxf9 : in_[2]) : 0.f;

	const float max_xy = (x > y) ? x : y;
	const float max_xyz = (max_xy > z) ? max_xy : z;
	const float maxColor = (max_xyz > minf9) ? max_xyz : minf9;

	union
	{
		float f;
		int32_t i;
	} fi;
	union
	{
		struct
		{
			uint32_t xm : 9;
			uint32_t ym : 9;
			uint32_t zm : 9;
			uint32_t e : 5;
		};
		uint32_t v;
	} ei;

	fi.f = maxColor;
	fi.i += 0x00004000; // round up leaving 9 bits in fraction (including assumed 1)

	// Fix applied from DirectXMath 3.10
	uint32_t exp = fi.i >> 23;
	ei.e = exp - 0x6f;

	fi.i = 0x83000000 - (exp << 23);

	ei.xm = static_cast<uint32_t>(lroundf(x * fi.f));
	ei.ym = static_cast<uint32_t>(lroundf(y * fi.f));
	ei.zm = static_cast<uint32_t>(lroundf(z * fi.f));
	return ei.v;
}

inline std::tuple<float, float, float> e5b9g9r92floats(uint32_t const in_)
{
	union
	{
		struct
		{
			uint32_t xm : 9;
			uint32_t ym : 9;
			uint32_t zm : 9;
			uint32_t e : 5;
		};
		uint32_t v;
	} ei;
	ei.v = in_;

	float exp = 0.0f;
	if(ei.e == 0)
	{
		// subnormal
		// for now flush to zero
		return { 0.0f, 0.0f, 0.0f };

		/* TODO finish this
		// not sure if each channel can be denormalised seperately...
		int32_t e = 1;
		uint32_t mr = ei.xm;
		uint32_t mg = ei.ym;
		uint32_t mb = ei.zm;
		do {
			e--;
			mr <<= 1;
		} while((mr & 0x400) == 0);
		mr &= 0x400;
		*/
	} else if(ei.e == 0x1F)
	{
		// infinity or nan
		if(ei.xm == 0 || ei.ym == 0 || ei.zm == 0)
		{
			return { std::numeric_limits<float>::quiet_NaN(),
					 std::numeric_limits<float>::quiet_NaN(),
					 std::numeric_limits<float>::quiet_NaN() };
		}
		return { std::numeric_limits<float>::infinity(),
				 std::numeric_limits<float>::infinity(),
				 std::numeric_limits<float>::infinity() };
	} else
	{
		exp = float(1 << ei.e);
	}

	float r = (1.0f + (float(ei.xm)/float(1 << 9))) * exp;
	float g = (1.0f + (float(ei.ym)/float(1 << 9))) * exp;
	float b = (1.0f + (float(ei.zm)/float(1 << 9))) * exp;
	return { r, g, b };
}

// we have the entire half.hpp for a complete C++11 half type but often
// you just want conversion. so here are Rygorous's one. These may be
// hw accelerated in future on HW that has a CPU instruction

// the half to float and vice versa is from Rygorous publid domain code
// the float to half is his fast round to even
// he has SSE/SIMD versions if required

// Same, but rounding ties to nearest even instead of towards +inf
// Ryg's float_to_half_fast3_rtne
static uint16_t float2half(float f_)
{
	union FP32
	{
		uint32_t u;
		float f;
		struct
		{
			uint32_t Mantissa : 23;
			uint32_t Exponent : 8;
			uint32_t Sign : 1;
		};
	};

	union FP16
	{
		uint16_t u;
		struct
		{
			uint32_t Mantissa : 10;
			uint32_t Exponent : 5;
			uint32_t Sign : 1;
		};
	};

	FP32 f32infty = { 255 << 23 };
	FP32 f16max   = { (127 + 16) << 23 };
	FP32 denorm_magic = { ((127 - 15) + (23 - 10) + 1) << 23 };
	uint32_t sign_mask = 0x80000000u;
	FP16 o = { 0 };
	FP32 f;
	f.f = f_;

	uint32_t sign = f.u & sign_mask;
	f.u ^= sign;

	// NOTE all the integer compares in this function can be safely
	// compiled into signed compares since all operands are below
	// 0x80000000. Important if you want fast straight SSE2 code
	// (since there's no unsigned PCMPGTD).

	if (f.u >= f16max.u) // result is Inf or NaN (all exponent bits set)
		o.u = (f.u > f32infty.u) ? 0x7e00 : 0x7c00; // NaN->qNaN and Inf->Inf
	else // (De)normalized number or zero
	{
		if (f.u < (113 << 23)) // resulting FP16 is subnormal or zero
		{
			// use a magic value to align our 10 mantissa bits at the bottom of
			// the float. as long as FP addition is round-to-nearest-even this
			// just works.
			f.f += denorm_magic.f;

			// and one integer subtract of the bias later, we have our final float!
			o.u = f.u - denorm_magic.u;
		}
		else
		{
			uint32_t mant_odd = (f.u >> 13) & 1; // resulting mantissa is odd

			// update exponent, rounding bias part 1
			f.u += 0xc8000fff;
			// rounding bias part 2
			f.u += mant_odd;
			// take the bits!
			o.u = f.u >> 13;
		}
	}

	o.u |= sign >> 16;
	return o.u;
}

// from half->float code
static float half2float(uint16_t h_)
{
	union FP32
	{
		uint32_t u;
		float f;
		struct
		{
			uint32_t Mantissa : 23;
			uint32_t Exponent : 8;
			uint32_t Sign : 1;
		};
	};

	union FP16
	{
		uint16_t u;
		struct
		{
			uint32_t Mantissa : 10;
			uint32_t Exponent : 5;
			uint32_t Sign : 1;
		};
	};

	static const FP32 magic = { 113 << 23 };
	static const uint32_t shifted_exp = 0x7c00 << 13; // exponent mask after shift
	FP16 h;
	h.u = h_;
	FP32 o;

	o.u = (h.u & 0x7fff) << 13;     // exponent/mantissa bits
	uint32_t exp = shifted_exp & o.u;   // just the exponent
	o.u += (127 - 15) << 23;        // exponent adjust

	// handle exponent special cases
	if (exp == shifted_exp) // Inf/NaN?
		o.u += (128 - 16) << 23;    // extra exp adjust
	else if (exp == 0) // Zero/Denormal?
	{
		o.u += 1 << 23;             // extra exp adjust
		o.f -= magic.f;             // renormalize
	}

	o.u |= (h.u & 0x8000) << 16;    // sign bit
	return o.f;
}

}   // namespace Maths

#endif //CORE_SCALAR_MATH_H
