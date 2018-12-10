//!-----------------------------------------------------
//!
//! \file vector_maths.h
//! contains the maths namespace stuff
//!
//!-----------------------------------------------------
#pragma once

#ifndef CORE_VECTOR_MATHS_H_
#define CORE_VECTOR_MATHS_H_

#include "core/core.h"
#include "math/scalar_math.h"

// moving over to glm, import them into Math namespace
#include "glm/glm.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "glm/gtc/matrix_integer.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"

namespace Math
{
	using namespace glm;
}

namespace Math {

class Plane
{
public:
	Plane() {};
	Plane(const float *fArr)
	{
		assert(fArr);
		a = fArr[0];
		b = fArr[1];
		c = fArr[2];
		d = fArr[3];
	};
	Plane(float a_, float b_, float c_, float d_) : a(a_), b(b_), c(c_), d(d_) {}

	Plane(vec3 n_, float d_) : a(n_.x), b(n_.y), c(n_.z), d(d_) {};

	// casting
	float const *data() const { return &a; }

	float *data() { return &a; }

	vec3 normal() const { return vec3(a, b, c); }

	constexpr bool operator==(const Plane& rhs) const
	{
		return a == rhs.a && b == rhs.b && c == rhs.c && d == rhs.d;
	}

	constexpr bool operator!=(const Plane& rhs) const
	{
		return a != rhs.a || b != rhs.b || c != rhs.c || d == rhs.d;
	};

	float a, b, c, d;
};

//! ax + by + cz + dw
inline float Dot(const Plane& plane, const vec4& vec)
{
	return plane.a * vec.x + plane.b * vec.y + plane.c * vec.z + plane.d * vec.w;
}

//! ax + by + cz + d
inline float DotPoint(const Plane& plane, const vec3& vec)
{
	return plane.a * vec.x + plane.b * vec.y + plane.c * vec.z + plane.d;
}

//!< ax + by + cz
inline float DotNormal(const Plane& plane, const vec3& vec)
{
	return plane.a * vec.x + plane.b * vec.y + plane.c * vec.z;
}



inline vec2 Promote(float x, float y) { return vec2(x, y); }

inline vec3 Promote(const vec2& v, float z) { return vec3(v.x, v.y, z); }

inline vec4 Promote(const vec3& v, float w) { return vec4(v.x, v.y, v.z, w); }

inline float Demote(vec2 const& v) { return v.x; }

inline vec2 Demote(vec3 const& v) { return vec2(v.x, v.y); }

inline vec3 Demote(vec4 const& v) { return vec3(v.x, v.y, v.z); }

inline float Project(vec2 const& v) { return v.x / v.y; }

inline vec2 Project(vec3 const& v) { return vec2(v.x, v.y) / v.z; }

inline vec3 Project(vec4 const& v) { return vec3(v.x, v.y, v.z) / v.w; }

inline auto ApproxEqual(vec2 const& lhs, vec2 const& rhs, float epsilon = 1e-5f) -> bool
{
	return ApproxEqual(lhs.x, rhs.x, epsilon) && ApproxEqual(lhs.y, rhs.y, epsilon);
}

inline auto ApproxEqual(vec3 const& lhs, vec3 const& rhs, float epsilon = 1e-5f) -> bool
{
	return ApproxEqual(Demote(lhs), Demote(rhs)) && ApproxEqual(lhs.z, rhs.z, epsilon);
}

inline auto ApproxEqual(vec4 const& lhs, vec4 const& rhs, float epsilon = 1e-5f) -> bool
{
	return ApproxEqual(Demote(lhs), Demote(rhs)) && ApproxEqual(lhs.w, rhs.w, epsilon);
}



inline auto ComponentMultiply(vec2 const a_, vec2 const b_) -> vec2
{
	return {a_.x * b_.x, a_.y * b_.y};
}

inline auto ComponentMultiply(vec3 const a_, vec3 const b_) -> vec3
{
	return {a_.x * b_.x, a_.y * b_.y, a_.z * b_.z};
}

inline auto ComponentMultiply(vec4 const a_, vec4 const b_) -> vec4
{
	return {a_.x * b_.x, a_.y * b_.y, a_.z * b_.z, a_.w * b_.w};
}

inline auto Reciprocal(vec2 const a_) -> vec2
{
	return {1.0f / a_.x, 1.0f / a_.y};
}

inline auto Reciprocal(vec3 const a_) -> vec3
{
	return {1.0f / a_.x, 1.0f / a_.y, 1.0f / a_.z};
}

inline auto Reciprocal(vec4 const a_) -> vec4
{
	return {1.0f / a_.x, 1.0f / a_.y, 1.0f / a_.z, 1.0f / a_.w};
}

inline auto Sqrt(vec2 const a_) -> vec2
{
	return {std::sqrt(a_.x), std::sqrt(a_.y)};
}

inline auto Sqrt(vec3 const a_) -> vec3
{
	return {std::sqrt(a_.x), std::sqrt(a_.y), std::sqrt(a_.z)};
}

inline auto Sqrt(vec4 const a_) -> vec4
{
	return {std::sqrt(a_.x), std::sqrt(a_.y), std::sqrt(a_.z), std::sqrt(a_.w)};
}

inline auto ReciprocalSqrt(vec2 const a_) -> vec2
{
	return {1.0f / std::sqrt(a_.x), 1.0f / std::sqrt(a_.y)};
}

inline auto ReciprocalSqrt(vec3 const a_) -> vec3
{
	return {1.0f / std::sqrt(a_.x), 1.0f / std::sqrt(a_.y), 1.0f / std::sqrt(a_.z)};
}

inline auto ReciprocalSqrt(vec4 const a_) -> vec4
{
	return {1.0f / std::sqrt(a_.x), 1.0f / std::sqrt(a_.y), 1.0f / std::sqrt(a_.z), 1.0f / std::sqrt(a_.w)};
}

inline auto Length(vec2 const a_) -> float { return std::sqrt(dot(a_, a_)); }    //!< Length^2 of a 2D Vector
inline auto Length(vec3 const a_) -> float { return std::sqrt(dot(a_, a_)); }    //!< Length^2 of a 3D Vector
inline auto Length(vec4 const a_) -> float { return std::sqrt(dot(a_, a_)); }    //!< Length^2 of a 4D Vector


template<typename T>
inline T Normalise(const T& vec) { return T(vec / Length(vec)); }        //!< returns a normalise version of vec


template<typename T>
inline T Lerp(const T& vecA, const T& vecB, float t) { return vecA + t * (vecB - vecA); }

inline int HorizMaxIndex(vec2 const& v_) { return (v_.x > v_.y) ? 0 : 1; }

inline int HorizMaxIndex(vec3 const& v_)
{
	return (v_.x > v_.y) ? (v_.x > v_.z) ? 0 : 2 : (v_.y > v_.z) ? 1 : 2;
}

inline float HorizMax(vec2 const& v_) { return v_[HorizMaxIndex(v_)]; }

inline float HorizMax(vec3 const& v_) { return v_[HorizMaxIndex(v_)]; }

inline int MajorAxis(vec2 const& v_)
{
	vec2 v = abs(v_);
	return HorizMaxIndex(v);
}

inline int MajorAxis(vec3 const& v_)
{
	vec3 v = abs(v_);
	return HorizMaxIndex(v);
}

inline vec2 TransformAndProject(mat4x4 const& matrix, vec2 const& vec )
{
	vec4 out = matrix * vec4(vec.x, vec.y, 0, 1);
	return vec2(out.x / out.w, out.y / out.w);
}
inline vec3 TransformAndProject(mat4x4 const& matrix, vec3 const& vec )
{
	vec4 out = matrix * vec4(vec.x, vec.y, vec.z, 1);
	return vec3(out.x / out.w, out.y / out.w, out.z / out.w);
}

inline vec2 TransformNormal(mat4x4 const& matrix, vec2 const& vec)
{
	vec4 out = matrix * vec4(vec.x, vec.y, 0, 0);
	return vec2(out.x, out.y);
}
inline vec3 TransformNormal(mat4x4 const& matrix, vec3 const& vec)
{
	vec4 out = matrix * vec4(vec.x, vec.y, vec.z, 0);
	return vec3(out.x, out.y, out.z);
}
inline Plane Normalise(const Plane& plane)
{
	float len = Length(vec3(plane.a, plane.b, plane.c));
	return Plane{plane.a / len, plane.b / len, plane.c / len, plane.d / len};
}

inline auto GetTranslation(mat4x4 const& matrix) -> vec3
{
	return Demote(matrix[3]);
}

inline auto Vec2FromArray( float const* array_) -> vec2
{
	return vec2(array_[0], array_[1]);
}

inline auto Vec3FromArray( float const* array_) -> vec3
{
	return vec3(array_[0], array_[1], array_[2]);
}

inline auto Vec4FromArray( float const* array_) -> vec4
{
	return vec4(array_[0], array_[1], array_[2], array_[3]);
}

inline auto Mat4x4FromArray( float const* array_) -> mat4x4
{
	return mat4x4(
			array_[ 0], array_[ 1], array_[ 2], array_[ 3],
			array_[ 4], array_[ 5], array_[ 6], array_[ 7],
			array_[ 8], array_[ 9], array_[10], array_[11],
			array_[12], array_[13], array_[14], array_[15]
			);
}


inline uint64_t MortonCurve(const Math::vec3& coord, const Math::vec3& max)
{
	// make temp be coord in a normalised 0 to 1 range
	vec3 temp = ComponentMultiply(Math::Reciprocal(max * 2.0f), coord) + vec3(0.5f, 0.5f, 0.5f);
	temp = clamp(temp, vec3(0, 0, 0), vec3(1, 1, 1));

	constexpr int NUM_BITS = 21;
	temp = temp * (float) (1 << NUM_BITS); // each coord component gets 21 bits
	uint64_t x = (uint32_t) temp.x;   // Interleave bits of x and y and z
	uint64_t y = (uint32_t) temp.y;
	uint64_t z = (uint32_t) temp.z;
	uint64_t r = 0; // r gets the resulting 63-bit Morton Number.

	// unroll for more speed...
	for(int64_t i = 0; i < NUM_BITS; i++)
	{
		uint64_t anders = (1ULL << i);

		r |= ((x & anders) >> i) << ((i * 3) + 0) |
			 ((y & anders) >> i) << ((i * 3) + 1) |
			 ((z & anders) >> i) << ((i * 3) + 2);
	}
	return r;
}

// aka interleave
inline uint64_t MortonCurve(uint16_t x_, uint16_t y_)
{
	constexpr int NUM_BITS = 16;
	uint64_t r = 0;

	// unroll for more speed...
	for (int64_t i = 0; i < NUM_BITS; i++)
	{
		uint64_t anders = (1ULL << i);
		r |= ((x_ & anders) >> i) << ((i * 2) + 0) |
			((y_ & anders) >> i) << ((i * 2) + 1);
	}
	return r;
}
// aka interleave
inline uint64_t MortonCurve(int16_t x_, int16_t y_)
{
	return MortonCurve(*reinterpret_cast<uint16_t*>(&x_),
		*reinterpret_cast<uint16_t*>(&y_));
}

// aka interleave
inline uint64_t MortonCurve(int16_t x_, int16_t y_, int16_t z_)
{
	return MortonCurve(*reinterpret_cast<uint16_t*>(&x_),
		*reinterpret_cast<uint16_t*>(&y_),
		*reinterpret_cast<uint16_t*>(&z_));
}

// aka interleave
inline uint64_t MortonCurve(uint16_t x_, uint16_t y_, uint16_t z_)
{
	constexpr int NUM_BITS = 16;
	uint64_t r = 0;

	// unroll for more speed...
	for (int64_t i = 0; i < NUM_BITS; i++)
	{
		uint64_t anders = (1ULL << i);
		r |= ((x_ & anders) >> i) << ((i * 3) + 0) |
			((y_ & anders) >> i) << ((i * 3) + 1) |
			((z_ & anders) >> i) << ((i * 3) + 2);
	}
	return r;
}



// aka interleave
inline uint64_t MortonCurve(uint32_t x_, uint32_t y_)
{
	constexpr int NUM_BITS = 32;
	uint64_t r = 0;

	// unroll for more speed... or use identitys on bit hacks websites
	for (int64_t i = 0; i < NUM_BITS; i++)
	{
		uint64_t anders = (1ULL << i);

		r |= ((x_ & anders) >> i) << ((i * 2) + 0) |
			((y_ & anders) >> i) << ((i * 2) + 1);
	}
	return r;
}
// aka interleave
inline uint64_t MortonCurve(int32_t x_, int32_t y_)
{
	return MortonCurve(*reinterpret_cast<uint32_t*>(&x_),
		*reinterpret_cast<uint32_t*>(&y_));
}


// aka interleave lossy for 32bit 3D integer vectors
inline uint64_t MortonCurve(uint32_t x_, uint32_t y_, uint32_t z_)
{
	constexpr int NUM_BITS = 21;
	uint64_t r = 0;

	// unroll for more speed...
	for (int64_t i = 0; i < NUM_BITS; i++)
	{
		uint64_t anders = (1ULL << i);
		r |= ((x_ & anders) >> i) << ((i * 3) + 0) |
			((y_ & anders) >> i) << ((i * 3) + 1) |
			((z_ & anders) >> i) << ((i * 3) + 2);
	}
	return r;
}

// aka interleave
inline uint64_t MortonCurve(int32_t x_, int32_t y_, int32_t z_)
{
	return MortonCurve(*reinterpret_cast<uint32_t*>(&x_),
		*reinterpret_cast<uint32_t*>(&y_),
		*reinterpret_cast<uint32_t*>(&z_));
}
inline bool IsFinite(const vec2& a)
{
	return (std::isfinite(a.x) && std::isfinite(a.y));
}

inline bool IsFinite(const vec3& a)
{
	return (std::isfinite(a.x) && std::isfinite(a.y) && std::isfinite(a.z));
}

inline bool IsFinite(const vec4& a)
{
	return (std::isfinite(a.x) && std::isfinite(a.y) && std::isfinite(a.z) && std::isfinite(a.w));
}

Plane CreatePlaneFromPoints(size_t pointCount, vec3 const *points);

inline bool ptInPoly(int nvert, const Math::vec2 *vert, const Math::vec2& test)
{
	bool c = false;
	int i, j;
	for(i = 0, j = nvert - 1; i < nvert; j = i++)
	{
		if(((vert[i].y > test.y) != (vert[j].y > test.y)) &&
		   (test.x < (vert[j].x - vert[i].x) * (test.y - vert[i].y) / (vert[j].y - vert[i].y) + vert[i].x))
		{
			c = !c;
		}
	}
	return c;
}

}


#endif

