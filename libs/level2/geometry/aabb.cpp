#include "core/core.h"
#include "geometry/aabb.h"
#include <algorithm>

namespace Geometry {

/// \brief	Transforms this AABB by the affine matrix parameter.
/// \param	matrix	An affine transform matrix.
/// \return	Transformed AABB.
AABB AABB::transformAffine( Math::Matrix4x4 const& matrix ) const
{
	// start search at the transform point
	AABB ret( Math::GetTranslation( matrix ), Math::GetTranslation( matrix ));

	// Find extreme points by considering product of
	// min and max with each component of trans.
	for(int j = 0; j < 3; ++j)
	{
		for(int i = 0; i < 3; ++i)
		{
			const float a = matrix( i, j ) * m_MinExtent[i];
			const float b = matrix( i, j ) * m_MaxExtent[i];

			if(a < b)
			{
				ret.m_MinExtent[j] += a;
				ret.m_MaxExtent[j] += b;
			} else
			{
				ret.m_MinExtent[j] += b;
				ret.m_MaxExtent[j] += a;
			}
		}
	}

	return ret;
}

/// \brief	Transforms this AABB by the parameter.
/// \param	matrix	The transform matrix.
/// \return	Transformed AABB.
AABB AABB::transform( const Math::Matrix4x4& matrix ) const
{

	AABB ret;

	for(int i = 0; i < 8; ++i)
	{
		const bool maxX = !!(i & 0x1);
		const bool maxY = !!(i & 0x2);
		const bool maxZ = !!(i & 0x4);
		Math::Vector3 pt;
		pt[0] = maxX ? m_MaxExtent[0] : m_MinExtent[0];
		pt[1] = maxY ? m_MaxExtent[1] : m_MinExtent[1];
		pt[2] = maxZ ? m_MaxExtent[2] : m_MinExtent[2];

		pt = Math::TransformAndProject( pt, matrix );
		ret.expandBy( pt );
	}

	return ret;
}

// AABB-triangle overlap test code
// by Tomas Akenine-Möller
static bool planeBoxOverlap( Math::Vector3 normal, Math::Vector3 vert, Math::Vector3 maxbox )    // -NJMP-
{
	Math::Vector3 vmin, vmax;

	for(auto q = 0u; q < 3; q++)
	{
		float v = vert[q];                    // -NJMP-
		if(normal[q] > 0.0f)
		{
			vmin[q] = -maxbox[q] - v;    // -NJMP-
			vmax[q] = maxbox[q] - v;    // -NJMP-
		} else
		{
			vmin[q] = maxbox[q] - v;    // -NJMP-
			vmax[q] = -maxbox[q] - v;    // -NJMP-
		}
	}

	if(Math::Dot( normal, vmin ) > 0.0f) return false;    // -NJMP-
	if(Math::Dot( normal, vmax ) >= 0.0f) return true;    // -NJMP-
	return false;
}

/// Query if this object intersects the triangle represented by the 3 vertices.
/// \brief	Transforms this AABB by the parameter.
/// \param	v0	1st vertex of the triangle.
/// \param	v1	2nd vertex of the triangle.
/// \param	v1	3rd vertex of the triangle.
/// \return	true if it succeeds, false if it fails.
bool AABB::intersects( Math::Vector3 const& tv0, Math::Vector3 const& tv1, Math::Vector3 const& tv2 ) const
{
	//   use separating axis theorem to test overlap between triangle and box
	//    need to test for overlap in these directions:
	//    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle
	//       we do not even need to test these)
	//    2) normal of the triangle
	//    3) crossproduct(edge from tri, {x,y,z}-directin)
	//       this gives 3x3=9 more tests
	using namespace Math;

	Vector3 const c = getBoxCenter();
	Vector3 const hl = getHalfLength();

	// This is the fastest branch on Sun
	// move everything so that the boxcenter is in (0,0,0)
	Vector3 const v0 = tv0 - c;
	Vector3 const v1 = tv1 - c;
	Vector3 const v2 = tv2 - c;

	// compute triangle edges
	Vector3 const e0 = v1 - v0;
	Vector3 const e1 = v2 - v1;
	Vector3 const e2 = v0 - v2;

	// Bullet 3:
	// test the 9 tests first (this was faster)

	{
		Vector3 fe = Abs( e0 );
		//	AXISTEST_X01( e0.z, e0.y, fez, fey );
		{
			float min = e0.z * v0.y - e0.y * v0.z;
			float max = e0.z * v2.y - e0.y * v2.z;
			if(min >= max) std::swap(min, max);

			float rad = fe.z * hl.y + fe.y * hl.z;
			if(min > rad || max < -rad) return false;
		}
		//AXISTEST_Y02( e0[Z], e0[X], fe.z, fe.x );
		{
			float min = -e0.z * v0.x + e0.x * v0.z;
			float max = -e0.z * v2.x + e0.x * v2.z;
			if(min >= max) std::swap(min, max);

			float rad = fe.z * hl.x + fe.x * hl.z;
			if(min > rad || max < -rad) return false;
		}
		//AXISTEST_Z12( e0[Y], e0[X], fe.y, fe.x );
		{
			float min = e0.y * v1.x - e0.x * v1.y;
			float max = e0.y * v2.x - e0.x * v2.y;
			if(min >= max) std::swap(min, max);

			float rad = fe.y * hl.x + fe.x * hl.y;
			if(min > rad || max < -rad) return false;
		}
	}

	{
		//AXISTEST_X01( e1[Z], e1[Y], fez, fey );
		Vector3 fe = Abs( e1 );
		{
			float min = e1.z * v0.y - e1.y * v0.z;
			float max = e1.z * v2.y - e1.y * v2.z;
			if(min >= max) std::swap( min, max );

			float rad = fe.z * hl.y + fe.y * hl.z;
			if(min > rad || max < -rad) return false;
		}
		//AXISTEST_Y02( e1[Z], e1[X], fe.z, fe.x );
		{
			float min = -e1.z * v0.x + e1.x * v0.z;
			float max = -e1.z * v2.x + e1.x * v2.z;
			if(min >= max) std::swap(min, max);

			float rad = fe.z * hl.x + fe.x * hl.z;
			if(min > rad || max < -rad) return false;

		}
		//AXISTEST_Z0( e1[Y], e1[X], fe.y, fe.x );
		{
			float min = e1.y * v0.x - e1.x * v0.y;
			float max = e1.y * v1.x - e1.x * v1.y;
			if(min >= max) std::swap(min, max);
			float rad = fe.y * hl.z + fe.x * hl.y;
			if(min > rad || max < -rad) return false;
		}
	}
	{
		Vector3 fe = Abs( e2 );
		// AXISTEST_X2( e2[Z], e2[Y], fez, fey );
		{
			float min = e2.z * v0.y - e2.y * v0.z;
			float max = e2.z * v1.y - e2.y * v1.z;
			if(min >= max) std::swap( min, max );

			float rad = fe.z * hl.y + fe.y * hl.z;
			if(min > rad || max < -rad) return false;
		}
		//		AXISTEST_Y1( e2[Z], e2[X], fez, fex );
		{
			float min = -e2.z * v0.x + e2.x * v0.z;
			float max = -e2.z * v1.x + e2.x * v1.z;
			if(min >= max) std::swap( min, max );
			float rad = fe.z * hl.x + fe.x * hl.z;
			if(min > rad || max < -rad) return false;
		}
		//AXISTEST_Z12( e2[Y], e2[X], fey, fex );
		{
			float min = e2.y * v1.x - e2.x * v1.y;
			float max = e2.y * v2.x - e2.x * v2.y;
			if(min >= max) std::swap(min, max);

			float rad = fe.y * hl.x + fe.x * hl.y;
			if(min > rad || max < -rad) return false;
		}
	}

	// Bullet 1:
	//  first test overlap in the {x,y,z}-directions
	//  find min, max of the triangle each direction, and test for overlap in
	//  that direction -- this is equivalent to testing a minimal AABB around
	//  the triangle against the AABB

	// test in XYZ-direction
	//	FINDMINMAX( v0[X], v1[X], v2[X], min, max );
	for( auto i = 0u; i < 3; ++i)
	{
		float min = v0[i];
		float max = v0[i];
    	if(v1[i] < min) min = v1[i];
    	if(v1[i] > max) max = v1[i];
		if(v2[i] < min) min = v2[i];
		if(v2[i] > max) max = v2[i];
		if(min > hl[i] || max < -hl[i]) return false;
	}

	// Bullet 2:
	//  test if the box intersects the plane of the triangle
	//  compute plane equation of triangle: normal*x+d=0
	Vector3 normal = Cross(e0,e1);

	if(!planeBoxOverlap( normal, v0, hl )) return false;

	// box and triangle overlaps
	return true;
}

} // end namespace
