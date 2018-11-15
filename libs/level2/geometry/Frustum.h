#pragma once
#ifndef WYRD_GEOMETRY_FRUSTUM_H
#define WYRD_GEOMETRY_FRUSTUM_H

#include "math/vector_math.h"
#include "geometry/aabb.h"
#include <array>

namespace Geometry {

class Frustum {
public:
	enum class CullResult : uint8_t
	{
		Outside = 0,
		Crossing = 1,
		Inside = 2,
	};

	std::array<Math::Plane, 5> planes;
	Math::mat4x4 matrix;

	Frustum() {};

	Frustum( Math::mat4x4 const& _matrix )
	{
		createFromMatrix( _matrix );
	}

	//! distance from the frustum to a point
	//! if the point is inside, the return value will be < 0
	auto distanceToPoint( const Math::vec3& point ) -> float const {
		float closestPoint = FLT_MAX;
		for( int i=0; i < 6; ++i ) {
			closestPoint = Math::min( Math::DotPoint( planes[i], point ), closestPoint );
		}
		return closestPoint;
	}

	//!Taking an AABB min and max in world space, work out its interaction with the view frustum
	auto cullAABB( Geometry::AABB const& oAABB ) -> CullResult const {
		bool intersect = false;
		Math::vec3 vMin, vMax;

		for (int i=0; i<6; i++)
		{
			vMin = oAABB.getMinExtent();
			vMax = oAABB.getMaxExtent();

			if (planes[i].a > 0) {
				vMin.x = oAABB.getMaxExtent().x;
				vMax.x = oAABB.getMinExtent().x;
			}
			if (planes[i].b > 0) {
				vMin.y = oAABB.getMaxExtent().y;
				vMax.y = oAABB.getMinExtent().y;
			}
			if (planes[i].c > 0) {
				vMin.z = oAABB.getMaxExtent().z;
				vMax.z = oAABB.getMinExtent().z;
			}

			if(  Math::DotPoint(planes[i], vMin) < 0) {
				return CullResult::Outside;
			}
			if( Math::DotPoint(planes[i], vMax) <= 0) {
				intersect = true;
			}
		}

		return (intersect) ? CullResult::Crossing : CullResult::Inside;
	}

	auto createFromMatrix( const Math::mat4& _matrix ) -> void
	{
		matrix = _matrix;

		// Left clipping plane
		planes[0].a = matrix[0][3] + matrix[0][0];
		planes[0].b = matrix[1][3] + matrix[1][0];
		planes[0].c = matrix[2][3] + matrix[2][0];
		planes[0].d = matrix[3][3] + matrix[3][0];

		// Right clipping plane
		planes[1].a = matrix[0][3] - matrix[0][1];
		planes[1].b = matrix[1][3] - matrix[1][1];
		planes[1].c = matrix[2][3] - matrix[2][1];
		planes[1].d = matrix[3][3] - matrix[3][1];

		// Top clipping plane
		planes[2].a = matrix[0][3] - matrix[0][2];
		planes[2].b = matrix[1][3] - matrix[1][2];
		planes[2].c = matrix[2][3] - matrix[2][2];
		planes[2].d = matrix[3][3] - matrix[3][2];

		// Bottom clipping plane
		planes[3].a = matrix[0][3] + matrix[0][3];
		planes[3].b = matrix[1][3] + matrix[1][3];
		planes[3].c = matrix[2][3] + matrix[2][3];
		planes[3].d = matrix[3][3] + matrix[3][3];

		// Near clipping plane
		planes[4].a = matrix[0][2];
		planes[4].b = matrix[1][2];
		planes[4].c = matrix[2][2];
		planes[4].d = matrix[3][2];

		for ( int i = 0; i < 5; ++i )
		{
			planes[i] = Math::Normalise( planes[i] );
		}
	}

	auto getWorldSpacePoints( Math::vec3 pts[8] ) const -> void {
		//     e----h
		//	  /|   /|
		//	 / f--/-g
		/// /  / /  /
		//	a---d  /
		//	|/   |/
		//  b___c
		Math::vec3 a( -1, -1, 0 );
		Math::vec3 b( -1,  1, 0 );
		Math::vec3 c(  1,  1, 0 );
		Math::vec3 d(  1, -1, 0 );
		Math::vec3 e( -1, -1, 0.99999f );
		Math::vec3 f( -1,  1, 0.99999f );
		Math::vec3 g(  1,  1, 0.99999f );
		Math::vec3 h(  1, -1, 0.99999f );

		Math::mat4x4 inv = Math::inverse( matrix );

		pts[0] = Math::TransformAndProject( inv, a );
		pts[1] = Math::TransformAndProject( inv, b );
		pts[2] = Math::TransformAndProject( inv, c );
		pts[3] = Math::TransformAndProject( inv, d );
		pts[4] = Math::TransformAndProject( inv, e );
		pts[5] = Math::TransformAndProject( inv, f );
		pts[6] = Math::TransformAndProject( inv, g );
		pts[7] = Math::TransformAndProject( inv, h );

	}
};


} //namespace Geometry

#endif //WYRD_GEOMETRY_FRUSTUM_H
