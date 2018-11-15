
#include "core/core.h"
#include "ray.h"
#include <utility>
#include <algorithm>
#include <limits>

namespace Geometry
{
Ray::Ray() {}

Ray::Ray( Math::vec3 const& origin, Math::vec3 const& direction )
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
	min = -std::numeric_limits<float>::max();
	max = std::numeric_limits<float>::max();

	// clip to each axis
	Math::vec3 p1 = bounds.getMinExtent() - m_origin;
	Math::vec3 p2 = bounds.getMaxExtent() - m_origin;
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
	Math::vec3 const& v0, Math::vec3 const& v1, Math::vec3 const& v2,
	float& v, float& w, float& t 
) const
{
	static float const zero( 0 );
	static float const one( 1 );

	// find vectors for two edges sharing v0
	Math::vec3 const edge1 = v1 - v0;
	Math::vec3 const edge2 = v2 - v0;

	// begin calculating determinant - also used to calculate v parameter
	Math::vec3 const pvec = cross( m_direction, edge2 );

	// if determinant is zero, ray lies in plane of triangle
	float const det = dot( edge1, pvec );

	// pre-compute the reciprocal
	float const det_inv = one/det;

	// calculate the distance from v0 to the ray origin
	Math::vec3 const  tvec = m_origin - v0;

	// calculate v parameter and test bounds
	v = dot( tvec, pvec )*det_inv;
	if( zero < v && v <= one )
	{
		// prepare to test w parameter
		Math::vec3 const qvec = cross( tvec, edge1 );

		// calculate w parameter and test bounds
		w = dot( m_direction, qvec )*det_inv;
		if( zero < w && w + v <= one )
		{
			// ray intersects triangle, calculate t
			t = dot( edge2, qvec )*det_inv;
			return true;
		}
	}

	// no intersection
	return false;
}

bool Ray::intersectsTriangleBackFaceCull(
	Math::vec3 const& v0, Math::vec3 const& v1, Math::vec3 const& v2,
	float& v, float& w, float& t
) const
{
	static float const zero(0);
	static float const one(1);

	// find vectors for two edges sharing v0
	Math::vec3 const edge1 = v1 - v0;
	Math::vec3 const edge2 = v2 - v0;

	// begin calculating determinant - also used to calculate v parameter
	Math::vec3 const pvec = cross(m_direction, edge2);

	// if determinant is zero, ray lies in plane of triangle
	float const det = dot(edge1, pvec);

	// cull front-facing triangles
	if (det >= zero)
		return false;

	// pre-compute the reciprocal
	float const det_inv = one / det;

	// calculate the distance from v0 to the ray origin
	Math::vec3 const tvec = m_origin - v0;

	// calculate v parameter and test bounds
	v = dot(tvec, pvec)*det_inv;
	if (zero < v && v <= one)
	{
		// prepare to test w parameter
		Math::vec3 const qvec = cross(tvec, edge1);

		// calculate w parameter and test bounds
		w = dot(m_direction, qvec)*det_inv;
		if (zero < w && w + v <= one)
		{
			// ray intersects triangle, calculate t
			t = dot(edge2, qvec)*det_inv;
			return true;
		}
	}

	// no intersection
	return false;
}

}