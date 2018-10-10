#pragma once
#ifndef WYRD_GEOMETRY_AABB_H
#define WYRD_GEOMETRY_AABB_H

#include "math/vector_math.h"

namespace Geometry {

/// \class AABB
/// \brief Axis Aligned Bounding Box. 
class AABB {
public:
	/// Default constructor. AABB is invalid.
	CALL explicit AABB();

	/// Constructor.
	/// \param	min	The minimum extents as a vector.
	/// \param	max	The maximum extents as a vector.
	CALL explicit AABB( Math::Vector3 const& min, Math::Vector3 const& max );

	/// Constructor.
	/// \param	min	The minimum extents as a float array.
	/// \param	max	The maximum extents as a float array.
	CALL explicit AABB( float const min[3], float const max[3] );

#if PLATFORM != NVGPU
	static AABB fromCenterAndHalfLength( Math::Vector3 const& center, Math::Vector3 const& halfLength );
#endif

	/// Query if this object is valid.
	/// \return	true if valid, false if not.
	CALL bool isValid() const;

	/// Gets the minimum extent.
	/// \return	The minimum extent.
	CALL Math::Vector3 const& getMinExtent() const;

	/// Gets the maximum extent.
	/// \return	The maximum extent.
	CALL Math::Vector3 const & getMaxExtent() const;

	/// Gets the box center.
	/// \return	The box center.
	CALL Math::Vector3 getBoxCenter() const;

	/// Gets the half length.
	/// \return	The half length.
	CALL Math::Vector3 getHalfLength() const;

	/// Sets a box center.
	/// \param	center	The center.
	CALL void setBoxCenter( Math::Vector3 const & center );

	/// expand (AKA union) by a pnt. 
	/// \param	pnt	The pnt.
	CALL void expandBy( Math::Vector3 const& pnt );

	/// expand (AKA union) by another aabb.
	/// \param	aabb	The aabb.
	CALL void expandBy( AABB const& aabb );

	/// Union this box with the parameter.
	/// \param	aabb	The aabb.
	CALL void unionWith( AABB const& aabb );

	/// This box becomes the intersection of the param and itself.
	/// \param	aabb	The aabb.
	CALL void intersectWith( AABB const& aabb );

	/// Query if this object intersects the given aabb.
	/// \param	aabb	The aabb.
	/// \return	true if it succeeds, false if it fails.
	CALL bool intersects( AABB const& aabb ) const;

	/// Query if this object intersects the point.
	/// \param	_point	The point.
	/// \return	true if it succeeds, false if it fails.
	CALL bool intersects( Math::Vector3 const& _point ) const;

	/// Query if this object intersects the plane.
	/// \param	_point	The plane.
	/// \return	true if it succeeds, false if it fails.
	CALL bool intersects( Math::Plane const& _point ) const;

	/// Query if this object intersects the triangle represented by the 3 vertices.
	/// \param	v0	1st vertex of the triangle.
	/// \param	v1	2nd vertex of the triangle.
	/// \param	v1	3rd vertex of the triangle.
	/// \return	true if it succeeds, false if it fails.
	CALL bool intersects( Math::Vector3 const& v0,  Math::Vector3 const& v1, Math::Vector3 const& v2 ) const;

	/// Transforms this AABB by the affine matrix parameter.
	/// \param	matrix	An Affine matrix.
	/// \return	The transformed AABB.
	CALL AABB transformAffine( Math::Matrix4x4 const& matrix ) const;

	CALL AABB transform( Math::Matrix4x4 const& matrix ) const;

private:
	Math::Vector3 m_MinExtent;	//!< Minimum extent of the box
	Math::Vector3 m_MaxExtent;	//!< Maximum extent of the box
};

#	include "aabb.inl"

}	//namespace Core


#endif
