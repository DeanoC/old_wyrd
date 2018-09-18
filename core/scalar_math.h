//
// Created by Computer on 23/07/2018.
//

#ifndef CORE_SCALAR_MATH_H
#define CORE_SCALAR_MATH_H

#include "core/core.h"
#include <math.h>
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

namespace Math
{

	template <typename T, typename = std::enable_if< std::is_floating_point<T>{} >::type>
    static constexpr T pi() { return static_cast<T>(3.14159265358979323846264338327950L); }

    template <typename T, typename = std::enable_if< std::is_floating_point<T>{} >::type>
    static constexpr T pi_over_2() { return pi<T>() / static_cast<T>(2); }

    template<typename T>
    static constexpr T square(T const val) { return val * val; }

	template<typename T, typename = std::enable_if< std::is_floating_point<T>{} >::type>
	static constexpr T degreesToRadians(T const val) { 
		return val * (pi<T>() / static_cast<T>(180));
	}

	template<typename T, typename = std::enable_if< std::is_floating_point<T>{} >::type>
	static constexpr T radiansToDegrees(T const val) {
		return static_cast<T>(180) / (val * pi<T>());
	}

}   // namespace Maths

#endif //CORE_SCALAR_MATH_H
