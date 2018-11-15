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
	explicit AABB();

	/// Constructor.
	/// \param	min	The minimum extents as a vector.
	/// \param	max	The maximum extents as a vector.
	explicit AABB( Math::vec3 const& min, Math::vec3 const& max );

	/// Constructor.
	/// \param	min	The minimum extents as a float array.
	/// \param	max	The maximum extents as a float array.
	explicit AABB( float const min[3], float const max[3] );

	static AABB fromCenterAndHalfLength( Math::vec3 const& center, Math::vec3 const& halfLength );

	/// Query if this object is valid.
	/// \return	true if valid, false if not.
	bool isValid() const;

	/// Gets the minimum extent.
	/// \return	The minimum extent.
	Math::vec3 const& getMinExtent() const;

	/// Gets the maximum extent.
	/// \return	The maximum extent.
	Math::vec3 const & getMaxExtent() const;

	/// Gets the box center.
	/// \return	The box center.
	Math::vec3 getBoxCenter() const;

	/// Gets the half length.
	/// \return	The half length.
	Math::vec3 getHalfLength() const;

	/// Sets a box center.
	/// \param	center	The center.
	void setBoxCenter( Math::vec3 const & center );

	/// expand (AKA union) by a pnt. 
	/// \param	pnt	The pnt.
	void expandBy( Math::vec3 const& pnt );

	/// expand (AKA union) by another aabb.
	/// \param	aabb	The aabb.
	void expandBy( AABB const& aabb );

	/// Union this box with the parameter.
	/// \param	aabb	The aabb.
	void unionWith( AABB const& aabb );

	/// This box becomes the intersection of the param and itself.
	/// \param	aabb	The aabb.
	void intersectWith( AABB const& aabb );

	/// Query if this object intersects the given aabb.
	/// \param	aabb	The aabb.
	/// \return	true if it succeeds, false if it fails.
	bool intersects( AABB const& aabb ) const;

	/// Query if this object intersects the point.
	/// \param	_point	The point.
	/// \return	true if it succeeds, false if it fails.
	bool intersects( Math::vec3 const& _point ) const;

	/// Query if this object intersects the plane.
	/// \param	_point	The plane.
	/// \return	true if it succeeds, false if it fails.
	bool intersects( Math::Plane const& _point ) const;

	/// Query if this object intersects the triangle represented by the 3 vertices.
	/// \param	v0	1st vertex of the triangle.
	/// \param	v1	2nd vertex of the triangle.
	/// \param	v1	3rd vertex of the triangle.
	/// \return	true if it succeeds, false if it fails.
	bool intersects( Math::vec3 const& v0,  Math::vec3 const& v1, Math::vec3 const& v2 ) const;

	/// Transforms this AABB by the affine matrix parameter.
	/// \param	matrix	An Affine matrix.
	/// \return	The transformed AABB.
	AABB transformAffine( Math::mat4x4 const& matrix ) const;

	AABB transform( Math::mat4x4 const& matrix ) const;

private:
	Math::vec3 m_MinExtent;	//!< Minimum extent of the box
	Math::vec3 m_MaxExtent;	//!< Maximum extent of the box
};

#	include "aabb.inl"

}	//namespace Core


#endif
