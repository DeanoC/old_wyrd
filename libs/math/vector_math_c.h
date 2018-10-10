//!-----------------------------------------------------
//!
//! \file vector_maths_c.h
//! contains the maths stuff in the default C impl
//! Matrices are row-major, Left Handed
//!
//!-----------------------------------------------------

#pragma once

#ifndef CORE_VECTOR_MATHS_C_H
#define CORE_VECTOR_MATHS_C_H

#include "core/core.h"
#include "cx/cx_math.h"
#include "math/scalar_math.h"

//! Maths functions live in here
namespace Math {
class Vector2
{
public:
	CALL explicit Vector2() {};
	CALL explicit Vector2(const float *fArr)
	{
		assert(fArr);
		x = fArr[0];
		y = fArr[1];
	};
	CALL constexpr Vector2(float _x, float _y) : x(_x), y(_y) {};

	// casting
	CALL float const *data() const { return &x; }

	CALL float *data() { return &x; }

	// assignment operators
	CALL constexpr Vector2& operator+=(const Vector2& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	CALL constexpr Vector2& operator-=(const Vector2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	};
	CALL constexpr Vector2& operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;
		return *this;
	};
	CALL constexpr Vector2& operator/=(float rhs)
	{
		y /= rhs;
		y /= rhs;
		return *this;
	};

	// unary operators
	CALL constexpr Vector2 operator+() const { return Vector2(x, y); }

	CALL constexpr Vector2 operator-() const { return Vector2(-x, -y); }

	// binary operators
	CALL constexpr Vector2 operator+(const Vector2& rhs) const { return Vector2(x + rhs.x, y + rhs.y); }

	CALL constexpr Vector2 operator-(const Vector2& rhs) const { return Vector2(x - rhs.x, y - rhs.y); };
	CALL constexpr Vector2 operator*(float rhs) const { return Vector2(x * rhs, y * rhs); };
	CALL constexpr Vector2 operator/(float rhs) const { return Vector2(x / rhs, y / rhs); };

	CALL constexpr float operator[](int index) const
	{
		assert(index >= 0 && index <= 1);
		switch(index)
		{
			case 0: return x;
			case 1: return y;
			default: return x;
		}
	}

	CALL constexpr float& operator[](int index)
	{
		assert(index >= 0 && index <= 1);
		switch(index)
		{
			case 0: return x;
			case 1: return y;
			default: return x;
		}
	}

	CALL constexpr friend Vector2 operator*(float lhs, const Vector2& rhs)
	{
		return Vector2(lhs * rhs.x, lhs * rhs.y);
	};

	CALL constexpr bool operator==(const Vector2& rhs) const { return x == rhs.x && y == rhs.y; }

	CALL constexpr bool operator!=(const Vector2& rhs) const { return x != rhs.x || y != rhs.y; };

	float x, y;
};

class Vector3
{
public:
	CALL explicit Vector3() {};
	CALL explicit Vector3(const float *fArr)
	{
		assert(fArr);
		x = fArr[0];
		y = fArr[1];
		z = fArr[2];
	};
	CALL constexpr Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};

	// casting
	CALL float const *data() const { return &x; }

	CALL float *data() { return &x; }

	// assignment operators
	CALL constexpr Vector3& operator+=(const Vector3& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	CALL constexpr Vector3& operator-=(const Vector3& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	};
	CALL constexpr Vector3& operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	};
	CALL constexpr Vector3& operator/=(float rhs)
	{
		y /= rhs;
		y /= rhs;
		z /= rhs;
		return *this;
	};

	// unary operators
	CALL constexpr Vector3 operator+() const { return Vector3(x, y, z); }

	CALL constexpr Vector3 operator-() const { return Vector3(-x, -y, -z); }

	// binary operators
	CALL constexpr Vector3 operator+(const Vector3& rhs) const { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }

	CALL constexpr Vector3 operator-(const Vector3& rhs) const { return Vector3(x - rhs.x, y - rhs.y, z - rhs.z); };
	CALL constexpr Vector3 operator*(float rhs) const { return Vector3(x * rhs, y * rhs, z * rhs); };
	CALL constexpr Vector3 operator/(float rhs) const { return Vector3(x / rhs, y / rhs, z / rhs); };

	CALL constexpr float operator[](int index) const
	{
		assert(index >= 0 && index <= 2);
		switch(index)
		{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			default: return x;
		}
	}

	CALL constexpr float& operator[](int index)
	{
		assert(index >= 0 && index <= 2);
		switch(index)
		{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			default: return x;
		}
	}

	CALL constexpr friend Vector3 operator*(float lhs, const Vector3& rhs)
	{
		return Vector3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
	};

	CALL constexpr bool operator==(const Vector3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }

	CALL constexpr bool operator!=(const Vector3& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; };

	float x, y, z;
};

class Vector4
{
public:
	CALL explicit Vector4() {};
	CALL explicit Vector4(const float *fArr)
	{
		assert(fArr);
		x = fArr[0];
		y = fArr[1];
		z = fArr[2];
		w = fArr[3];
	};
	CALL constexpr Vector4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {};

	// casting
	CALL float const *data() const { return &x; }

	CALL float *data() { return &x; }

	// assignment operators
	CALL constexpr Vector4& operator+=(const Vector4& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;
		return *this;
	}

	CALL constexpr Vector4& operator-=(const Vector4& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		w -= rhs.w;
		return *this;
	};
	CALL constexpr Vector4& operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		w *= rhs;
		return *this;
	};
	CALL constexpr Vector4& operator/=(float rhs)
	{
		y /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;
		return *this;
	};

	// unary operators
	CALL constexpr Vector4 operator+() const { return Vector4(x, y, z, w); }

	CALL constexpr Vector4 operator-() const { return Vector4(-x, -y, -z, -w); }

	// binary operators
	CALL constexpr Vector4 operator+(const Vector4& rhs) const
	{
		return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}

	CALL constexpr Vector4 operator-(const Vector4& rhs) const
	{
		return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w + rhs.w);
	};
	CALL constexpr Vector4 operator*(float rhs) const { return Vector4(x * rhs, y * rhs, z * rhs, w * rhs); };
	CALL constexpr Vector4 operator/(float rhs) const { return Vector4(x / rhs, y / rhs, z / rhs, w * rhs); };

	CALL constexpr float operator[](int index) const
	{
		assert(index >= 0 && index <= 3);
		switch(index)
		{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			case 3: return w;
			default: return x;
		}
	}

	CALL constexpr float& operator[](int index)
	{
		assert(index >= 0 && index <= 3);
		switch(index)
		{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			case 3: return w;
			default: return x;
		}
	}

	CALL constexpr friend Vector4 operator*(float lhs, const Vector4& rhs)
	{
		return Vector4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
	};

	CALL constexpr bool operator==(const Vector4& rhs) const
	{
		return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
	}

	CALL constexpr bool operator!=(const Vector4& rhs) const
	{
		return x != rhs.x || y != rhs.y || z != rhs.z || w == rhs.w;
	};

	float x, y, z, w;
};

class Plane
{
public:
#if PLATFORM != NVGPU
	CALL Plane() {};
	CALL Plane(const float *fArr)
	{
		assert(fArr);
		a = fArr[0];
		b = fArr[1];
		c = fArr[2];
		d = fArr[3];
	};
	CALL constexpr Plane(float a_, float b_, float c_, float d_) : a(a_), b(b_), c(c_), d(d_) {};
	CALL constexpr Plane(Math::Vector3 n_, float d_) : a(n_.x), b(n_.y), c(n_.z), d(d_) {};
#endif
	// casting
	CALL float const *data() const { return &a; }

	CALL float *data() { return &a; }

	CALL constexpr Math::Vector3 normal() const { return Math::Vector3(a, b, c); }

	CALL constexpr bool operator==(const Plane& rhs) const
	{
		return a == rhs.a && b == rhs.b && c == rhs.c && d == rhs.d;
	}

	CALL constexpr bool operator!=(const Plane& rhs) const
	{
		return a != rhs.a || b != rhs.b || c != rhs.c || d == rhs.d;
	};

	float a, b, c, d;
};

class Matrix4x4
{
public:
#if PLATFORM != NVGPU
	CALL Matrix4x4() {};
	CALL Matrix4x4(const float *fArr)
	{
		assert(fArr);
		memcpy(&_11, fArr, sizeof(Matrix4x4));
	}

	CALL constexpr Matrix4x4(const Matrix4x4& rhs)
			: _11(rhs._11), _12(rhs._12), _13(rhs._13), _14(rhs._14), _21(rhs._21), _22(rhs._22), _23(rhs._23),
			  _24(rhs._24), _31(rhs._31), _32(rhs._32), _33(rhs._33), _34(rhs._34), _41(rhs._41), _42(rhs._42),
			  _43(rhs._43), _44(rhs._44) {}

	CALL constexpr Matrix4x4(float f11, float f12, float f13, float f14, float f21, float f22, float f23, float f24,
							 float f31, float f32, float f33, float f34, float f41, float f42, float f43, float f44)
			: _11(f11), _12(f12), _13(f13), _14(f14), _21(f21), _22(f22), _23(f23), _24(f24), _31(f31), _32(f32),
			  _33(f33), _34(f34), _41(f41), _42(f42), _43(f43), _44(f44) {}

#endif

	// access grants
	CALL constexpr float& operator()(unsigned int Row, unsigned int Col) { return m[(Row * 4) + Col]; }

	CALL constexpr float operator()(unsigned int Row, unsigned int Col) const { return m[(Row * 4) + Col]; }

	// casting operators
	CALL float const *data() const { return &_11; }

	CALL float *data() { return &_11; }

	// assignment operators
	CALL Matrix4x4& operator*=(const Matrix4x4& rhs)
	{
		*this = *this * rhs;
		return *this;
	}

	CALL constexpr Matrix4x4& operator+=(const Matrix4x4& rhs)
	{
		_11 += rhs._11;
		_12 += rhs._12;
		_13 += rhs._13;
		_14 += rhs._14;
		_21 += rhs._21;
		_22 += rhs._22;
		_23 += rhs._23;
		_24 += rhs._24;
		_31 += rhs._31;
		_32 += rhs._32;
		_33 += rhs._33;
		_34 += rhs._34;
		_41 += rhs._41;
		_42 += rhs._42;
		_43 += rhs._43;
		_44 += rhs._44;
		return *this;
	}

	CALL constexpr Matrix4x4& operator-=(const Matrix4x4& rhs)
	{
		_11 -= rhs._11;
		_12 -= rhs._12;
		_13 -= rhs._13;
		_14 -= rhs._14;
		_21 -= rhs._21;
		_22 -= rhs._22;
		_23 -= rhs._23;
		_24 -= rhs._24;
		_31 -= rhs._31;
		_32 -= rhs._32;
		_33 -= rhs._33;
		_34 -= rhs._34;
		_41 -= rhs._41;
		_42 -= rhs._42;
		_43 -= rhs._43;
		_44 -= rhs._44;
		return *this;
	}

	CALL constexpr Matrix4x4& operator*=(float rhs)
	{
		_11 *= rhs;
		_12 *= rhs;
		_13 *= rhs;
		_14 *= rhs;
		_21 *= rhs;
		_22 *= rhs;
		_23 *= rhs;
		_24 *= rhs;
		_31 *= rhs;
		_32 *= rhs;
		_33 *= rhs;
		_34 *= rhs;
		_41 *= rhs;
		_42 *= rhs;
		_43 *= rhs;
		_44 *= rhs;
		return *this;
	}

	CALL constexpr Matrix4x4& operator/=(float rhs)
	{
		float recip = 1.0f / rhs;
		return operator*=(recip);
	}

	// unary operators
	CALL constexpr Matrix4x4 operator+() const { return Matrix4x4(*this); }

#if PLATFORM != NVGPU
	CALL constexpr Matrix4x4 operator-() const
	{
		return Matrix4x4(-_11,
						 -_12,
						 -_13,
						 -_14,
						 -_21,
						 -_22,
						 -_23,
						 -_24,
						 -_31,
						 -_32,
						 -_33,
						 -_34,
						 -_41,
						 -_42,
						 -_43,
						 -_44);
	}

#endif


	// binary operators
	CALL Matrix4x4 operator*(const Matrix4x4& rhs) const
	{
		Matrix4x4 ret;
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				ret(i, j) = 0;
				for(int k = 0; k < 4; k++)
					ret(i, j) += (*this)(i, k) * rhs(k, j);
			}
		}
		return ret;
	}

	CALL constexpr Matrix4x4 operator+(const Matrix4x4& rhs) const
	{
		return (Matrix4x4(*this) += rhs);
	}

	CALL constexpr Matrix4x4 operator-(const Matrix4x4& rhs) const
	{
		return (Matrix4x4(*this) -= rhs);
	}

	CALL constexpr Matrix4x4 operator*(float rhs) const
	{
		return (Matrix4x4(*this) *= rhs);
	}

	CALL constexpr Matrix4x4 operator/(float rhs) const
	{
		return (Matrix4x4(*this) /= rhs);
	}

	CALL constexpr friend Matrix4x4 operator*(float lhs, const Matrix4x4& rhs)
	{
		return (Matrix4x4(rhs) *= lhs);
	}

	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;

		};
		float m[16];
	};

};

class Quaternion
{
public:
	CALL Quaternion() {}

	CALL Quaternion(const float *fArr)
	{
		assert(fArr);
		x = fArr[0];
		y = fArr[1];
		z = fArr[2];
		w = fArr[3];
	}

	CALL constexpr Quaternion(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {};

	// casting
	CALL float const *data() const { return &x; }

	CALL float *data() { return &x; }

	// assignment operators
	CALL constexpr Quaternion& operator+=(const Quaternion& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;
		return *this;
	}

	CALL constexpr Quaternion& operator-=(const Quaternion& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		w -= rhs.w;
		return *this;
	}

	CALL constexpr Quaternion& operator*=(const Quaternion& rhs)
	{
		*this = *this * rhs;
		return *this;
	}

	CALL constexpr Quaternion& operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		w *= rhs;
		return *this;
	};
	CALL constexpr Quaternion& operator/=(float rhs)
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;
		return *this;
	};

	// unary operators
	CALL constexpr Quaternion operator+() const { return Quaternion(x, y, z, w); }

	CALL constexpr Quaternion operator-() const { return Quaternion(-x, -y, -z, -w); }

	// binary operators
	CALL constexpr Quaternion operator+(const Quaternion& rhs) const
	{
		return Quaternion(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}

	CALL constexpr Quaternion operator-(const Quaternion& rhs) const
	{
		return Quaternion(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	}

	CALL constexpr Quaternion operator*(const Quaternion& rhs) const
	{
		float qx = rhs.x, qy = rhs.y, qz = rhs.z, qw = rhs.w;
		float rw = w * qw - x * qx - y * qy - z * qz;
		float rx = w * qx + x * qw + y * qz - z * qy;
		float ry = w * qy + y * qw + z * qx - x * qz;
		float rz = w * qz + z * qw + x * qy - y * qx;
		return Quaternion(rx, ry, rz, rw);
	}

	CALL constexpr Quaternion operator*(float rhs) const { return Quaternion(x * rhs, y * rhs, z * rhs, w * rhs); }

	CALL constexpr Quaternion operator/(float rhs) const { return Quaternion(x / rhs, y / rhs, z / rhs, w / rhs); };

	CALL constexpr friend Quaternion operator*(float lhs, const Quaternion& rhs) { return (Quaternion(rhs) *= lhs); }

	CALL constexpr bool operator==(const Quaternion& rhs) const
	{
		return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
	}

	CALL constexpr bool operator!=(const Quaternion& rhs) const
	{
		return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
	}

	float x, y, z, w;
};

CALL constexpr Vector2 Promote(float x, float y) { return Vector2(x, y); }

CALL constexpr Vector3 Promote(const Vector2& v, float z) { return Vector3(v.x, v.y, z); }

CALL constexpr Vector4 Promote(const Vector3& v, float w) { return Vector4(v.x, v.y, v.z, w); }

CALL constexpr float Demote(Vector2 const& v) { return v.x; }

CALL constexpr Vector2 Demote(Vector3 const& v) { return Vector2(v.x, v.y); }

CALL constexpr Vector3 Demote(Vector4 const& v) { return Vector3(v.x, v.y, v.z); }

CALL constexpr float Project(Vector2 const& v) { return v.x / v.y; }

CALL constexpr Vector2 Project(Vector3 const& v) { return Vector2(v.x, v.y) / v.z; }

CALL constexpr Vector3 Project(Vector4 const& v) { return Vector3(v.x, v.y, v.z) / v.w; }

CALL constexpr auto ApproxEqual(Vector2 const& lhs, Vector2 const& rhs, float epsilon = 1e-5f) -> bool
{
	return ApproxEqual(lhs.x, rhs.x, epsilon) && ApproxEqual(lhs.y, rhs.y, epsilon);
}

CALL constexpr auto ApproxEqual(Vector3 const& lhs, Vector3 const& rhs, float epsilon = 1e-5f) -> bool
{
	return ApproxEqual(Demote(lhs), Demote(rhs)) && ApproxEqual(lhs.z, rhs.z, epsilon);
}

CALL constexpr auto ApproxEqual(Vector4 const& lhs, Vector4 const& rhs, float epsilon = 1e-5f) -> bool
{
	return ApproxEqual(Demote(lhs), Demote(rhs)) && ApproxEqual(lhs.w, rhs.w, epsilon);
}

//! Dot product of 2 2D Vectors
CALL constexpr float Dot(const Vector2& vecA, const Vector2& vecB)
{
	return vecA.x * vecB.x + vecA.y * vecB.y;
}

//! Dot product of 2 3D Vectors
CALL constexpr float Dot(const Vector3& vecA, const Vector3& vecB)
{
	return vecA.x * vecB.x + vecA.y * vecB.y + vecA.z * vecB.z;
}
//! Dot product of 2 4D Vectors
CALL constexpr float Dot(const Vector4& vecA, const Vector4& vecB)
{
	return vecA.x * vecB.x + vecA.y * vecB.y + vecA.z * vecB.z + vecA.w * vecB.w;
}

//! Dot Product of 2 Quaternions
CALL constexpr float Dot(const Quaternion& quatA, const Quaternion& quatB)
{
	return quatA.x * quatB.x + quatA.y * quatB.y + quatA.z * quatB.z + quatA.w * quatB.w;
}

//! ax + by + cz + dw
CALL constexpr float Dot(const Plane& plane, const Vector4& vec)
{
	return plane.a * vec.x + plane.b * vec.y + plane.c * vec.z + plane.d * vec.w;
}

//! ax + by + cz + d
CALL constexpr float DotPoint(const Plane& plane, const Vector3& vec)
{
	return plane.a * vec.x + plane.b * vec.y + plane.c * vec.z + plane.d;
}

//!< ax + by + cz
CALL constexpr float DotNormal(const Plane& plane, const Vector3& vec)
{
	return plane.a * vec.x + plane.b * vec.y + plane.c * vec.z;
}

CALL constexpr auto ComponentMultiply(Vector2 const a_, Vector2 const b_) -> Vector2
{
	return {a_.x * b_.x, a_.y * b_.y};
}

CALL constexpr auto ComponentMultiply(Vector3 const a_, Vector3 const b_) -> Vector3
{
	return {a_.x * b_.x, a_.y * b_.y, a_.z * b_.z};
}

CALL constexpr auto ComponentMultiply(Vector4 const a_, Vector4 const b_) -> Vector4
{
	return {a_.x * b_.x, a_.y * b_.y, a_.z * b_.z, a_.w * b_.w};
}

CALL constexpr auto Reciprocal(Vector2 const a_) -> Vector2
{
	return {1.0f / a_.x, 1.0f / a_.y};
}

CALL constexpr auto Reciprocal(Vector3 const a_) -> Vector3
{
	return {1.0f / a_.x, 1.0f / a_.y, 1.0f / a_.z};
}

CALL constexpr auto Reciprocal(Vector4 const a_) -> Vector4
{
	return {1.0f / a_.x, 1.0f / a_.y, 1.0f / a_.z, 1.0f / a_.w};
}

CALL constexpr auto Sqrt(Vector2 const a_) -> Vector2
{
	return {cx::sqrt(a_.x), cx::sqrt(a_.y)};
}

CALL constexpr auto Sqrt(Vector3 const a_) -> Vector3
{
	return {cx::sqrt(a_.x), cx::sqrt(a_.y), cx::sqrt(a_.z)};
}

CALL constexpr auto Sqrt(Vector4 const a_) -> Vector4
{
	return {cx::sqrt(a_.x), cx::sqrt(a_.y), cx::sqrt(a_.z), cx::sqrt(a_.w)};
}

CALL constexpr auto ReciprocalSqrt(Vector2 const a_) -> Vector2
{
	return {1.0f / cx::sqrt(a_.x), 1.0f / cx::sqrt(a_.y)};
}

CALL constexpr auto ReciprocalSqrt(Vector3 const a_) -> Vector3
{
	return {1.0f / cx::sqrt(a_.x), 1.0f / cx::sqrt(a_.y), 1.0f / cx::sqrt(a_.z)};
}

CALL constexpr auto ReciprocalSqrt(Vector4 const a_) -> Vector4
{
	return {1.0f / cx::sqrt(a_.x), 1.0f / cx::sqrt(a_.y), 1.0f / cx::sqrt(a_.z), 1.0f / cx::sqrt(a_.w)};
}

CALL constexpr auto LengthSquared(Vector2 const a_) -> float { return Dot(a_, a_); }    //!< Length^2 of a 2D Vector
CALL constexpr auto LengthSquared(Vector3 const a_) -> float { return Dot(a_, a_); }    //!< Length^2 of a 3D Vector
CALL constexpr auto LengthSquared(Vector4 const a_) -> float { return Dot(a_, a_); }    //!< Length^2 of a 4D Vector
CALL constexpr auto LengthSquared(Quaternion const q_) -> float { return Dot(q_, q_); }    //!< Length^2 of a 4D Vector

CALL constexpr auto Length(Vector2 const a_) -> float { return cx::sqrt(Dot(a_, a_)); }    //!< Length^2 of a 2D Vector
CALL constexpr auto Length(Vector3 const a_) -> float { return cx::sqrt(Dot(a_, a_)); }    //!< Length^2 of a 3D Vector
CALL constexpr auto Length(Vector4 const a_) -> float { return cx::sqrt(Dot(a_, a_)); }    //!< Length^2 of a 4D Vector
CALL constexpr auto Length(Quaternion& q) -> float { return cx::sqrt(Dot(q, q)); }    //!< Length^2 of a 4D Vector

CALL constexpr float Cross(const Vector2& vecA, const Vector2& vecB)
{
	return vecA.x * vecB.y - vecA.y * vecB.x;
}   //!< Cross product of 2 2D Vectors (CCW this is actually vecA(x,y,0) cross vecB(x,y,0) )
CALL constexpr Vector3 Cross(const Vector3& vecA, const Vector3& vecB)
{
	return Vector3(vecA.y * vecB.z - vecA.z * vecB.y,
				   vecA.z * vecB.x - vecA.x * vecB.z,
				   vecA.x * vecB.y - vecA.y * vecB.x);
}   //!< Cross product of 2 3D Vectors

template<typename T>
CALL constexpr T Normalise(const T& vec) { return T(vec / Length(vec)); }        //!< returns a normalise version of vec

CALL constexpr Plane Normalise(const Plane& plane)
{
	float len = Length(Vector3(plane.a, plane.b, plane.c));
	return Plane{plane.a / len, plane.b / len, plane.c / len, plane.d / len};
}

template<typename T>
CALL constexpr T Lerp(const T& vecA, const T& vecB, float t) { return vecA + t * (vecB - vecA); }
//	inline Quaternion Slerp( const Quaternion& quatA, const Quaternion& quatB, float t ){ Quaternion quat; D3DXQuaternionSlerp( &quat, &quatA, &quatB, t ); return quat; }


CALL constexpr Vector2 Abs(const Vector2& vector) { return Vector2(cx::abs(vector.x), cx::abs(vector.y)); }

CALL constexpr Vector2 Max(const Vector2& vecA, const Vector2& vecB)
{
	return Vector2(vecA.x > vecB.x ? vecA.x : vecB.x, vecA.y > vecB.y ? vecA.y : vecB.y);
}

CALL constexpr Vector2 Min(const Vector2& vecA, const Vector2& vecB)
{
	return Vector2(vecA.x < vecB.x ? vecA.x : vecB.x, vecA.y < vecB.y ? vecA.y : vecB.y);
}

CALL constexpr Vector3 Abs(const Vector3& vector)
{
	return Vector3(cx::abs(vector.x), cx::abs(vector.y), cx::abs(vector.z));
}

CALL constexpr Vector3 Max(const Vector3& vecA, const Vector3& vecB)
{
	return Vector3(vecA.x > vecB.x ? vecA.x : vecB.x,
				   vecA.y > vecB.y ? vecA.y : vecB.y,
				   vecA.z > vecB.z ? vecA.z : vecB.z);
}

CALL constexpr Vector3 Min(const Vector3& vecA, const Vector3& vecB)
{
	return Vector3(vecA.x < vecB.x ? vecA.x : vecB.x,
				   vecA.y < vecB.y ? vecA.y : vecB.y,
				   vecA.z < vecB.z ? vecA.z : vecB.z);
}

CALL constexpr Vector4 Abs(const Vector4& vector)
{
	return Vector4(cx::abs(vector.x), cx::abs(vector.y), cx::abs(vector.z), cx::abs(vector.w));
}

CALL constexpr Vector4 Max(const Vector4& vecA, const Vector4& vecB)
{
	return Vector4(vecA.x > vecB.x ? vecA.x : vecB.x,
				   vecA.y > vecB.y ? vecA.y : vecB.y,
				   vecA.z > vecB.z ? vecA.z : vecB.z,
				   vecA.w > vecB.w ? vecA.w : vecB.w);
}

CALL constexpr Vector4 Min(const Vector4& vecA, const Vector4& vecB)
{
	return Vector4(vecA.x < vecB.x ? vecA.x : vecB.x,
				   vecA.y < vecB.y ? vecA.y : vecB.y,
				   vecA.z < vecB.z ? vecA.z : vecB.z,
				   vecA.w < vecB.w ? vecA.w : vecB.w);
}

CALL constexpr int HorizMaxIndex(Vector2 const& v_) { return (v_.x > v_.y) ? 0 : 1; }

CALL constexpr int HorizMaxIndex(Vector3 const& v_)
{
	return (v_.x > v_.y) ? (v_.x > v_.z) ? 0 : 2 : (v_.y > v_.z) ? 1 : 2;
}

CALL constexpr float HorizMax(Vector2 const& v_) { return v_[HorizMaxIndex(v_)]; }

CALL constexpr float HorizMax(Vector3 const& v_) { return v_[HorizMaxIndex(v_)]; }

CALL constexpr int MajorAxis(Vector2 const& v_)
{
	Vector2 v = Abs(v_);
	return HorizMaxIndex(v);
}

CALL constexpr int MajorAxis(Vector3 const& v_)
{
	Vector3 v = Abs(v_);
	return HorizMaxIndex(v);
}

CALL constexpr Vector4 Transform(const Vector4& vec, const Matrix4x4& matrix)
{
	return Vector4(matrix(0, 0) * vec.x + matrix(1, 0) * vec.y + matrix(2, 0) * vec.z + matrix(3, 0) * vec.w,
				   matrix(0, 1) * vec.x + matrix(1, 1) * vec.y + matrix(2, 1) * vec.z + matrix(3, 1) * vec.w,
				   matrix(0, 2) * vec.x + matrix(1, 2) * vec.y + matrix(2, 2) * vec.z + matrix(3, 2) * vec.w,
				   matrix(0, 3) * vec.x + matrix(1, 3) * vec.y + matrix(2, 3) * vec.z + matrix(3, 3) * vec.w);
}

CALL constexpr Vector4 Transform(const Vector2& vec, const Matrix4x4& matrix)
{  //!< return Vector4(vec,0,1) * matrix
	return Vector4(matrix(0, 0) * vec.x + matrix(1, 0) * vec.y + matrix(3, 0),
				   matrix(0, 1) * vec.x + matrix(1, 1) * vec.y + matrix(3, 1),
				   matrix(0, 2) * vec.x + matrix(1, 2) * vec.y + matrix(3, 2),
				   matrix(0, 3) * vec.x + matrix(1, 3) * vec.y + matrix(3, 3));
}

CALL constexpr Vector4 Transform(const Vector3& vec, const Matrix4x4& matrix)
{   //!< return Vector4(vec,1) * matrix
	return Vector4(matrix(0, 0) * vec.x + matrix(1, 0) * vec.y + matrix(2, 0) * vec.z + matrix(3, 0),
				   matrix(0, 1) * vec.x + matrix(1, 1) * vec.y + matrix(2, 1) * vec.z + matrix(3, 1),
				   matrix(0, 2) * vec.x + matrix(1, 2) * vec.y + matrix(2, 2) * vec.z + matrix(3, 2),
				   matrix(0, 3) * vec.x + matrix(1, 3) * vec.y + matrix(2, 3) * vec.z + matrix(3, 3));
}

CALL constexpr Vector2 TransformAndProject(const Vector2& vec, const Matrix4x4& matrix)
{
	Vector4 out = Transform(Math::Vector4(vec.x, vec.y, 0, 1), matrix);
	return Vector2(out.x / out.w, out.y / out.w);
} //!< return Project(Vector4(vec,0,1) * matrix)
CALL constexpr Vector3 TransformAndProject(const Vector3& vec, const Matrix4x4& matrix)
{
	Vector4 out = Transform(Math::Vector4(vec.x, vec.y, vec.z, 1), matrix);
	return Vector3(out.x / out.w, out.y / out.w, out.z / out.w);
} //!< return Project(Vector4(vec,1) * matrix)
CALL constexpr Vector2 TransformAndDropZ(const Vector2& vec, const Matrix4x4& matrix)
{
	Vector4 out = Transform(Math::Vector4(vec.x, vec.y, 0, 1), matrix);
	return Vector2(out.x, out.y);
} //!< return (Vector4(vec,0,1) * matrix).xy
CALL constexpr Vector3 TransformAndDropW(const Vector3& vec, const Matrix4x4& matrix)
{
	return Vector3(matrix(0, 0) * vec.x + matrix(1, 0) * vec.y + matrix(2, 0) * vec.z + matrix(3, 0),
				   matrix(0, 1) * vec.x + matrix(1, 1) * vec.y + matrix(2, 1) * vec.z + matrix(3, 1),
				   matrix(0, 2) * vec.x + matrix(1, 2) * vec.y + matrix(2, 2) * vec.z + matrix(3, 2));
}

CALL constexpr Vector2 TransformNormal(const Vector2& vec, const Matrix4x4& matrix)
{
	Vector4 out = Transform(Vector4(vec.x, vec.y, 0, 0), matrix);
	return Vector2(out.x, out.y);
} //!< return Vector4(vec,0,0) * matrix
CALL constexpr Vector3 TransformNormal(const Vector3& vec, const Matrix4x4& matrix)
{
	Vector4 out = Transform(Vector4(vec.x, vec.y, vec.z, 0), matrix);
	return Vector3(out.x, out.y, out.z);
} //!< return Vector4(vec,0) * matrix

#if PLATFORM != NVGPU
CALL constexpr Matrix4x4 IdentityMatrix() { return Matrix4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1); }

CALL constexpr Matrix4x4 TransposeMatrix(const Matrix4x4& mat)
{
	return Matrix4x4(mat(0, 0),
					 mat(1, 0),
					 mat(2, 0),
					 mat(3, 0),
					 mat(0, 1),
					 mat(1, 1),
					 mat(2, 1),
					 mat(3, 1),
					 mat(0, 2),
					 mat(1, 2),
					 mat(2, 2),
					 mat(3, 2),
					 mat(0, 3),
					 mat(1, 3),
					 mat(2, 3),
					 mat(3, 3));
}

#endif
CALL inline Matrix4x4 MultiplyMatrix(const Matrix4x4& a, const Matrix4x4& b) { return a * b; }

CALL constexpr Vector3 GetTranslation(const Matrix4x4& mat) { return Vector3(mat._41, mat._42, mat._43); }

CALL constexpr Vector3 GetXAxis(const Matrix4x4& mat) { return Vector3(mat._11, mat._12, mat._13); }

CALL constexpr Vector3 GetYAxis(const Matrix4x4& mat) { return Vector3(mat._21, mat._22, mat._23); }

CALL constexpr Vector3 GetZAxis(const Matrix4x4& mat) { return Vector3(mat._31, mat._32, mat._33); }

CALL constexpr Vector3 GetScale(const Matrix4x4& mat) { return Vector3(mat._11, mat._22, mat._33); }

CALL constexpr void SetTranslation(Matrix4x4& mat, float x, float y, float z)
{
	mat._41 = x;
	mat._42 = y;
	mat._43 = z;
}

CALL constexpr void SetTranslation(Matrix4x4& mat, const Vector3& pos) { SetTranslation(mat, pos.x, pos.y, pos.z); }

CALL constexpr void SetScale(Matrix4x4& mat, float x, float y, float z)
{
	mat._11 = x;
	mat._22 = y;
	mat._33 = z;
}

CALL constexpr void SetScale(Matrix4x4& mat, const Vector3& pos) { SetScale(mat, pos.x, pos.y, pos.z); }

CALL constexpr void SetXAxis(Matrix4x4& mat, const Vector3& axis)
{
	mat._11 = axis.x;
	mat._12 = axis.y;
	mat._13 = axis.z;
}

CALL constexpr void SetYAxis(Matrix4x4& mat, const Vector3& axis)
{
	mat._21 = axis.x;
	mat._22 = axis.y;
	mat._23 = axis.z;
}

CALL constexpr void SetZAxis(Matrix4x4& mat, const Vector3& axis)
{
	mat._31 = axis.x;
	mat._32 = axis.y;
	mat._33 = axis.z;
}

CALL constexpr Quaternion IdentityQuat() { return Quaternion(0, 0, 0, 1); }

CALL constexpr Matrix4x4 CreateXRotationMatrix(const float a)
{
	return Matrix4x4{1, 0, 0, 0, 0, cx::cos(a), cx::sin(a), 0, 0, -cx::sin(a), cx::cos(a), 0, 0, 0, 0, 1,};
}

CALL constexpr Matrix4x4 CreateYRotationMatrix(const float a)
{
	return Matrix4x4{cx::cos(a), 0, -cx::sin(a), 0, 0, 1, 0, 0, cx::sin(a), 0, cx::cos(a), 0, 0, 0, 0, 1,};
}

CALL constexpr Matrix4x4 CreateZRotationMatrix(const float a)
{
	return Matrix4x4{cx::cos(a), cx::sin(a), 0, 0, -cx::sin(a), cx::cos(a), 0, 0, 0, 1, 0, 0, 0, 0, 0, 1,};

}

CALL constexpr Matrix4x4 CreateScaleMatrix(const float sx, const float sy, const float sz)
{
	Matrix4x4 result(IdentityMatrix());
	SetScale(result, sx, sy, sz);
	return result;
}

CALL constexpr Matrix4x4 CreateTranslationMatrix(const float tx, const float ty, const float tz)
{
	Matrix4x4 result(IdentityMatrix());
	SetTranslation(result, tx, ty, tz);
	return result;
};
CALL constexpr Matrix4x4 CreateScaleMatrix(const Vector3& scale)
{
	return CreateScaleMatrix(scale.x, scale.y, scale.z);
}

CALL constexpr Matrix4x4 CreateTranslationMatrix(const Vector3& trans)
{
	return CreateTranslationMatrix(trans.x, trans.y, trans.z);
}

CALL inline Matrix4x4 CreateLookAtMatrix(const Math::Vector3& eye, const Math::Vector3& to, const Math::Vector3& up)
{
	Matrix4x4 result;
	const Math::Vector3 f = Normalise(to - eye);
	Math::Vector3 u = Normalise(up);
	const Math::Vector3 s = Normalise(Cross(u, f));
	u = Cross(f, s);

	result(0, 0) = s.x;
	result(0, 1) = u.x;
	result(0, 2) = f.x;
	result(0, 3) = 0;
	result(1, 0) = s.y;
	result(1, 1) = u.y;
	result(1, 2) = f.y;
	result(1, 3) = 0;
	result(2, 0) = s.z;
	result(2, 1) = u.z;
	result(2, 2) = f.z;
	result(2, 3) = 0;
	result(3, 0) = -Dot(s, eye);
	result(3, 1) = -Dot(u, eye);
	result(3, 2) = -Dot(f, eye);
	result(3, 3) = 1;

	return result;
}

//	inline Matrix4x4 CreateYawPitchRollRotationMatrix( const float yawRads, const float pitchRads, const float rollRads ){ Matrix4x4 result; D3DXMatrixRotationYawPitchRoll( &result, yawRads, pitchRads, rollRads ); return result; }
#if PLATFORM != NVGPU
CALL constexpr Matrix4x4 CreateRotationMatrix(const Quaternion& quat)
{
	const float q1 = quat.x;
	const float q2 = quat.y;
	const float q3 = quat.z;
	const float q0 = quat.w;
	const float q1_2 = q1 * q1;
	const float q2_2 = q2 * q2;
	const float q3_2 = q3 * q3;
	const float q0_2 = q0 * q0;
	return Matrix4x4(q0_2 + q1_2 - q2_2 - q3_2,
					 (2 * q1 * q2) + (2 * q0 * q3),
					 (2 * q1 * q3) - (2 * q0 * q2),
					 0,
					 (2 * q1 * q2) - (2 * q0 * q3),
					 q0_2 - q1_2 + q2_2 - q3_2,
					 (2 * q2 * q3) + (2 * q0 * q1),
					 0,
					 (2 * q1 * q3) + (2 * q0 * q2),
					 (2 * q2 * q3) - (2 * q0 * q1),
					 q0_2 - q1_2 - q2_2 + q3_2,
					 0,
					 0,
					 0,
					 0,
					 1);
}

#endif

inline CALL Quaternion CreateRotationQuat(const Matrix4x4& m)
{
	float s = 0.0f;
	Quaternion q;

	float t = m._11 + m._22 + m._33;
	if(t > 0.0f)
	{
		s = sqrtf(t + 1.0f);
		q.w = s * 0.5f;
		s = 0.5f / s;

		q.x = (m._32 - m._23) * s;
		q.y = (m._13 - m._31) * s;
		q.z = (m._21 - m._12) * s;
	} else
	{
		int biggest = 0;
		if(m._11 > m._22)
		{
			if(m._33 > m._11)
				biggest = 2;//I;
			else
				biggest = 0;//A;
		} else
		{
			if(m._33 > m._11)
				biggest = 2;//I;
			else
				biggest = 1;//E;
		}

		switch(biggest)
		{
			case 0:s = sqrtf(m._11 - (m._22 + m._33) + 1.0f);
				if(s > 1e-8f)
				{

					q.x = s * 0.5f;
					s = 0.5f / s;
					q.w = (m._32 - m._23) * s;
					q.y = (m._12 + m._21) * s;
					q.z = (m._13 + m._31) * s;
					break;
				}
				// I
				s = sqrtf(m._33 - (m._11 + m._22) + 1.0f);
				if(s > 1e-8f)
				{
					q.z = s * 0.5f;
					s = 0.5f / s;
					q.w = (m._21 - m._12) * s;
					q.x = (m._31 + m._13) * s;
					q.y = (m._32 + m._23) * s;
					break;
				}
				// E
				s = sqrtf(m._22 - (m._33 + m._11) + 1.0f);
				if(s > 1e-8f)
				{
					q.y = s * 0.5f;
					s = 0.5f / s;
					q.w = (m._13 - m._31) * s;
					q.z = (m._23 + m._32) * s;
					q.x = (m._21 + m._12) * s;
					break;
				}
				break;

			case 1:s = sqrtf(m._22 - (m._33 + m._11) + 1.0f);
				if(s > 1e-8f)
				{
					q.y = s * 0.5f;
					s = 0.5f / s;
					q.w = (m._13 - m._31) * s;
					q.z = (m._23 + m._32) * s;
					q.x = (m._21 + m._12) * s;
					break;
				}
				// I
				s = sqrtf(m._33 - (m._11 + m._22) + 1.0f);
				if(s > 1e-8f)
				{
					q.z = s * 0.5f;
					s = 0.5f / s;
					q.w = (m._21 - m._12) * s;
					q.x = (m._31 + m._13) * s;
					q.y = (m._32 + m._23) * s;
					break;
				}
				// A
				s = sqrtf(m._11 - (m._22 + m._33) + 1.0f);
				if(s > 1e-8f)
				{
					q.x = s * 0.5f;
					s = 0.5f / s;
					q.w = (m._32 - m._23) * s;
					q.y = (m._12 + m._21) * s;
					q.z = (m._13 + m._31) * s;
					break;
				}
				break;

			case 2:s = sqrtf(m._33 - (m._11 + m._22) + 1.0f);
				if(s > 1e-8f)
				{
					q.z = s * 0.5f;
					s = 0.5f / s;
					q.w = (m._21 - m._12) * s;
					q.x = (m._31 + m._13) * s;
					q.y = (m._32 + m._23) * s;
					break;
				}
				// A
				s = sqrtf(m._11 - (m._22 + m._33) + 1.0f);
				if(s > 1e-8f)
				{
					q.x = s * 0.5f;
					s = 0.5f / s;
					q.w = (m._32 - m._23) * s;
					q.y = (m._12 + m._21) * s;
					q.z = (m._13 + m._31) * s;
					break;
				}
				// E
				s = sqrtf(m._22 - (m._33 + m._11) + 1.0f);
				if(s > 1e-8f)
				{
					q.y = s * 0.5f;
					s = 0.5f / s;
					q.w = (m._13 - m._31) * s;
					q.z = (m._23 + m._32) * s;
					q.x = (m._21 + m._12) * s;
					break;
				}
				break;
			default:assert(false);
		}
	}
	return q;
}

CALL constexpr Quaternion CreateRotationQuat(const Vector3& axis, float angle)
{
	return Quaternion(cx::sin(angle / 2) * axis.x,
					  cx::sin(angle / 2) * axis.y,
					  cx::sin(angle / 2) * axis.z,
					  cx::cos(angle / 2));
}
// from D3DX_DXGIFormatConvert.inl
CALL constexpr float float_to_SRGB(float val)
{
	if(val < 0.0031308f)
		val *= 12.92f;
	else
		val = 1.055f * powf(val, 1.0f / 2.4f) - 0.055f;
	return val;
}

CALL constexpr float SRGB_to_float_inexact(float val)
{
	if(val < 0.04045f)
		val /= 12.92f;
	else
		val = pow((val + 0.055f) / 1.055f, 2.4f);
	return val;
}

CALL float SRGB_to_float(uint32_t val);
CALL float SRGB_to_float(float val);

CALL inline bool ptInPoly(int nvert, const Math::Vector2 *vert, const Math::Vector2& test)
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

CALL Plane CreatePlaneFromPoints(size_t pointCount, Math::Vector3 const *points);


};

#endif // end CORE_VECTOR_MATHS_C_H
