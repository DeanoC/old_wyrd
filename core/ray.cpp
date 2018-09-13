/// \file	core\ray.cpp
/// \brief	Implements the ray class.
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
#include "core/core.h"
#include "ray.h"
#include <utility>
#include <algorithm>

namespace Core
{
Ray::Ray() {}

Ray::Ray( Math::Vector3 const& origin, Math::Vector3 const& direction )
  : m_origin( origin ), 
	m_direction( direction )
{
}

/*! Based on the "slabs" method by Kay and Kayjia.	
 */
bool Ray::intersectsAABB( AABB const& bounds, float& min, float& max ) const
{
	static float const zero( 0 );
	static float const one( 1 );

	// start off with a big slab
	min = -FLT_MAX;
	max = FLT_MAX;

	// clip to each axis
	Math::Vector3 p1 = bounds.getMinExtent() - m_origin;
	Math::Vector3 p2 = bounds.getMaxExtent() - m_origin;
	for( int axis = 0; axis < 3; ++axis )
	{
		// compute the ray extents (i.e. slab) along this axis
		float rcp = one/m_direction[axis];
		float c1 = p1[axis]*rcp;
		float c2 = p2[axis]*rcp;

		// check for stuff blowing up (when ray perpendicular to an axis)
		if( std::isfinite( c1 ) && std::isfinite( c2 ) )
		{
			// if we get decent slab bounds then clip the ray extents
			if( c1 > c2 )
				std::swap( c1, c2 );
			min = std::max( min, c1 );
			max = std::min( max, c2 );
	
			// check validity
			if( min > max )
				return false;
		}
		else
		{
			// infinite slab, so just check if the origin is contained
			if( p1[axis] > zero || p2[axis] < zero )
				return false;
		}
	}

	// intersected
	return true;
}

/*! Moller and Trumbore fast culled ray-triangle intersection.
*/ 
bool Ray::intersectsTriangle(
	Math::Vector3 const& v0, Math::Vector3 const& v1, Math::Vector3 const& v2,
	float& v, float& w, float& t 
) const
{
	static float const zero( 0 );
	static float const one( 1 );

	// find vectors for two edges sharing v0
	Math::Vector3 edge1 = v1 - v0;
	Math::Vector3 edge2 = v2 - v0;

	// begin calculating determinant - also used to calculate v parameter
	Math::Vector3 pvec = Cross( m_direction, edge2 );

	// if determinant is zero, ray lies in plane of triangle
	float const det = Dot( edge1, pvec );

	// pre-compute the reciprocal
	float const det_inv = one/det;

	// calculate the distance from v0 to the ray origin
	Math::Vector3 tvec = m_origin - v0;

	// calculate v parameter and test bounds
	v = Dot( tvec, pvec )*det_inv;
	if( zero < v && v <= one )
	{
		// prepare to test w parameter
		Math::Vector3 qvec = Cross( tvec, edge1 );

		// calculate w parameter and test bounds
		w = Dot( m_direction, qvec )*det_inv;
		if( zero < w && w + v <= one )
		{
			// ray intersects triangle, calculate t
			t = Dot( edge2, qvec )*det_inv;
			return true;
		}
	}

	// no intersection
	return false;
}

bool Ray::intersectsTriangleBackFaceCull(
	Math::Vector3 const& v0, Math::Vector3 const& v1, Math::Vector3 const& v2,
	float& v, float& w, float& t
) const
{
	static float const zero(0);
	static float const one(1);

	// find vectors for two edges sharing v0
	Math::Vector3 edge1 = v1 - v0;
	Math::Vector3 edge2 = v2 - v0;

	// begin calculating determinant - also used to calculate v parameter
	Math::Vector3 pvec = Cross(m_direction, edge2);

	// if determinant is zero, ray lies in plane of triangle
	float const det = Dot(edge1, pvec);

	// cull front-facing triangles
	if (det >= zero)
		return false;

	// pre-compute the reciprocal
	float const det_inv = one / det;

	// calculate the distance from v0 to the ray origin
	Math::Vector3 tvec = m_origin - v0;

	// calculate v parameter and test bounds
	v = Dot(tvec, pvec)*det_inv;
	if (zero < v && v <= one)
	{
		// prepare to test w parameter
		Math::Vector3 qvec = Cross(tvec, edge1);

		// calculate w parameter and test bounds
		w = Dot(m_direction, qvec)*det_inv;
		if (zero < w && w + v <= one)
		{
			// ray intersects triangle, calculate t
			t = Dot(edge2, qvec)*det_inv;
			return true;
		}
	}

	// no intersection
	return false;
}

}