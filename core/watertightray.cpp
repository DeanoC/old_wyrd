
/// \file	core\watertightray.cpp
/// \brief	Implements the watertightray class.
/// \remark	Copyright (c) 2018 Dean Calver. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
/// \remark Journal of Computer Graphics Techniques
///	\remark Watertight Ray/Triangle Intersection
///	\remark Vol. 2, No. 1, 2013
///	\remark http://jcgt.org

#include "core/core.h"
#include "watertightray.h"
#include <utility>
#include <algorithm>

namespace {
// conservative up and down rounding
static float p = 1.0f + 2e-23f;
static float m = 1.0f - 2e-23f;
float up(float a) { return a>0.0f ? a*p : a*m; }
float dn(float a) { return a>0.0f ? a*m : a*p; }
//∗ fast rounding for positive numbers
float Up(float a) { return a*p; }
float Dn(float a) { return a*m; }
Math::Vector3 Up(Math::Vector3 const& a) { return Math::Vector3(Up(a.x),Up(a.y),Up(a.z)); }
Math::Vector3 Dn(Math::Vector3 const& a) { return Math::Vector3(Dn(a.x),Dn(a.y),Dn(a.z)); }

uint32_t signmask(float x)
{
	union {
		float x;
		uint32_t ix;
	} conv;
	conv.x = x;
	return conv.ix & 0x80000000;
}
float xorf(float x, uint32_t o)
{
	union {
		float x;
		uint32_t ix;
	} conv;
	conv.x = x;
	conv.ix = conv.ix ^ o;

	return conv.x;
}

}

/// \namespace	Core
namespace Core {
WaterTightRay::WaterTightRay()
{}

WaterTightRay::WaterTightRay( Math::Vector3 const& origin_, Math::Vector3 const& direction_ )
		: origin( origin_ ),
		  dir( direction_ ),
		  rdir( Math::Reciprocal(direction_))
{
	// calculate dimension where the ray direction is maximal
	kz = Math::MajorAxis( dir );
	kx = kz + 1;
	if(kx == 3) kx = 0;
	ky = kx + 1;
	if(ky == 3) ky = 0;

	// swap kx and ky dimension to preserve winding direction of triangles
	if(dir[kz] < 0.0f) std::swap( kx, ky );

	// calculate shear constants
	shear.x = dir[kx] / dir[kz];
	shear.y = dir[ky] / dir[kz];
	shear.z = 1.0f / dir[kz];

}

bool WaterTightRay::intersectsAABB( AABB const& aabbbox, float& min, float& max ) const
{
	using namespace Math;

	// Calculate the offset to the near and far planes for
	// the kx , ky , and kz dimension for a box stored in the
	// order lower _ x , lower _ y , lower _z , upper_x , upper_y , upper_z
	// in memory.

	// TODO store this AABB precalc somewhere if lots of rays interesting same box
	float box[6] = { aabbbox.getMinExtent().x,
					 aabbbox.getMinExtent().y,
					 aabbbox.getMinExtent().z,
					 aabbbox.getMaxExtent().x,
					 aabbbox.getMaxExtent().y,
					 aabbbox.getMaxExtent().z,
	};
	int nearID[3] = { 0,1, 2};
	int farID[3] = { 3, 4, 5 };

	int nearX = nearID[kx], farX = farID[kx];
	int nearY = nearID[ky], farY = farID[ky];
	int nearZ = nearID[kz], farZ = farID[kz];

	if (dir[kx] < 0.0f) std::swap(nearX,farX);
	if (dir[ky] < 0.0f) std::swap(nearY,farY);
	if (dir[kz] < 0.0f) std::swap(nearZ,farZ);

	// Calculate corrected origin for near and far−plane
	// distance calculations. Each floating−point operation
	// is forced to be rounded in to the correct direction.
	static float const eps = 5.0f * 2e-24f;
	Vector3 lower = Dn(Abs(origin-aabbbox.getMinExtent()));
	Vector3 upper = Up(Abs(origin-aabbbox.getMaxExtent()));
	float max_z = Max(lower[kz],upper[kz]);
	float err_near_x = Up(lower[kx]+max_z);
	float err_near_y = Up(lower[ky]+max_z);
	float org_near_x = up(origin[kx]+Up(eps*err_near_x));
	float org_near_y = up(origin[ky]+Up(eps*err_near_y));
	float org_near_z = origin[kz];
	float err_far_x = Up(upper[kx]+max_z);
	float err_far_y = Up(upper[ky]+max_z);
	float org_far_x = dn(origin[kx]-Up(eps*err_far_x));
	float org_far_y = dn(origin[ky]-Up(eps*err_far_y));
	float org_far_z = origin[kz];

	if (dir[kx] < 0.0f) std::swap(org_near_x,org_far_x);
	if (dir[ky] < 0.0f) std::swap(org_near_y,org_far_y);

	// Calculate corrected reciprocal direction for near− and far−plane
	// distance calculations. We correct with one additional ulp to
	// also correctly round the subtraction inside the traversal loop.
	// This works only because the ray is only allowed to hit geometry
	// in front of it.
	float rdir_near_x = Dn(Dn(rdir[kx]));
	float rdir_near_y = Dn(Dn(rdir[ky]));
	float rdir_near_z = Dn(Dn(rdir[kz]));
	float rdir_far_x = Up(Up(rdir[kx]));
	float rdir_far_y = Up(Up(rdir[ky]));
	float rdir_far_z = Up(Up(rdir[kz]));

	float tNearX = (box[nearX] - org_near_x) * rdir_near_x;
	float tNearY = (box[nearY] - org_near_y) * rdir_near_y;
	float tNearZ = (box[nearZ] - org_near_z) * rdir_near_z;
	float tFarX = (box[farX ] - org_far_x ) * rdir_far_x;
	float tFarY = (box[farY ] - org_far_y ) * rdir_far_y;
	float tFarZ = (box[farZ ] - org_far_z ) * rdir_far_z;
	min = std::max(tNearX, std::max(tNearY,tNearZ));
	max = std::min(tFarX, std::min(tFarY ,tFarZ));
	return min <= max;
}

bool WaterTightRay::intersectsTriangleBackFaceCull(
	Math::Vector3 const& v0, Math::Vector3 const& v1, Math::Vector3 const& v2,
	float& v, float& w, float& t
) const
{
	using namespace Math;
	// calculate vertices relative to ray origin
	Vector3 const A = v0 - origin;
	Vector3 const B = v1 - origin;
	Vector3 const C = v2 - origin;
	// perform shear and scale of vertices ∗
	float const Ax = A[kx] - shear.x * A[kz];
	float const Ay = A[ky] - shear.y * A[kz];
	float const Bx = B[kx] - shear.x * B[kz];
	float const By = B[ky] - shear.y * B[kz];
	float const Cx = C[kx] - shear.x * C[kz];
	float const Cy = C[ky] - shear.y * C[kz];

	// calculate scaled barycentric coordinates
	float U = Cx * By - Cy * Bx;
	float V = Ax * Cy - Ay * Cx;
	float W = Bx * Ay - By * Ax;

	// fall back to test against edges using doubles

	if (U == 0.0f || V == 0.0f || W == 0.0f)
	{
		double CxBy = (double)Cx * (double)By;
		double CyBx = (double)Cy * (double)Bx;
		U = (float)(CxBy - CyBx);
		double AxCy = (double)Ax * (double)Cy;
		double AyCx = (double)Ay * (double)Cx;
		V = (float)(AxCy - AyCx);
		double BxAy = (double)Bx * (double)Ay;
		double ByAx = (double)By * (double)Ax;
		W = (float)(BxAy - ByAx);
	}

	// Perform edge tests. Moving this test before
	// and at the end of the previous conditional
	// gives higher performance.
	if (U<0.0f || V<0.0f || W<0.0f)
	{
		return false;
	}

	// calculate determinant
	float det = U + V + W;
	if (det == 0.0f) return false;

	// Calculate scaled z−coordinates of vertices
	// and use them to calculate the hit distance.
	float const Az = shear.z * A[kz];
	float const Bz = shear.z * B[kz];
	float const Cz = shear.z * C[kz];
	float const T = U * Az + V * Bz + W * Cz;

	if (T < 0.0f)
	{
		return false;
	}

	// normalize U, V, W, and T
	float const rcpDet = 1.0f / det;
	v = V * rcpDet;
	w = W * rcpDet;
	t = T * rcpDet;

	return true;
}
bool WaterTightRay::intersectsTriangle(
		Math::Vector3 const& v0, Math::Vector3 const& v1, Math::Vector3 const& v2,
		float& v, float& w, float& t
) const
{
	using namespace Math;
	// calculate vertices relative to ray origin
	Vector3 const A = v0 - origin;
	Vector3 const B = v1 - origin;
	Vector3 const C = v2 - origin;
	// perform shear and scale of vertices ∗
	float const Ax = A[kx] - shear.x * A[kz];
	float const Ay = A[ky] - shear.y * A[kz];
	float const Bx = B[kx] - shear.x * B[kz];
	float const By = B[ky] - shear.y * B[kz];
	float const Cx = C[kx] - shear.x * C[kz];
	float const Cy = C[ky] - shear.y * C[kz];

	// calculate scaled barycentric coordinates
	float U = Cx * By - Cy * Bx;
	float V = Ax * Cy - Ay * Cx;
	float W = Bx * Ay - By * Ax;

	// fall back to test against edges using doubles

	if(U == 0.0f || V == 0.0f || W == 0.0f)
	{
		double CxBy = (double) Cx * (double) By;
		double CyBx = (double) Cy * (double) Bx;
		U = (float) (CxBy - CyBx);
		double AxCy = (double) Ax * (double) Cy;
		double AyCx = (double) Ay * (double) Cx;
		V = (float) (AxCy - AyCx);
		double BxAy = (double) Bx * (double) Ay;
		double ByAx = (double) By * (double) Ax;
		W = (float) (BxAy - ByAx);
	}

	// Perform edge tests. Moving this test before
	// and at the end of the previous conditional
	// gives higher performance.
	if ((U<0.0f || V<0.0f || W<0.0f) &&
		(U>0.0f || V>0.0f || W>0.0f)) return false;

	// calculate determinant
	float det = U + V + W;
	if(det == 0.0f) return false;

	// Calculate scaled z−coordinates of vertices
	// and use them to calculate the hit distance.
	float const Az = shear.z * A[kz];
	float const Bz = shear.z * B[kz];
	float const Cz = shear.z * C[kz];
	float const T = U * Az + V * Bz + W * Cz;

	uint32_t det_sign = signmask(det);
	if (xorf(T, det_sign) < 0.0f)
	{
		return false;
	}

	// normalize U, V, W, and T
	float const rcpDet = 1.0f / det;
	v = V * rcpDet;
	w = W * rcpDet;
	t = T * rcpDet;

	return true;
}

}
