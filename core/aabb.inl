/// \file	core\aabb.inl
/// \remark	Copyright (c) 2011 Dean Calver. Public Domain.
/// \remark	mailto://deano@cloudpixies.com

#include <limits>
/// \brief	Default constructor, AABB is invalid.
INLINE AABB::AABB() :
m_MinExtent( std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()),
m_MaxExtent( -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity())
{
}

/// \brief	Constructor, set via the minimum and maximum extents.
/// \param	min	The minimum extent of the box.
/// \param	max	The maximum extent of the box. 
INLINE AABB::AABB( Math::Vector3 const & min, Math::Vector3 const& max ) :
	m_MinExtent( min ),
	m_MaxExtent( max ) {
}

/// \brief	Constructor, set via the minimum and maximum extents.
/// \param	min	The minimum extent of the box.
/// \param	max	The maximum extent of the box. 
INLINE AABB::AABB( float const min[3], float const max[3] ) :
	m_MinExtent( min[0], min[1], min[2] ),
	m_MaxExtent( max[0], max[1], max[2] ) {
}

/// \brief	Constructor, from center and the half length
/// \param	center	The center of the box.
/// \param	halfLength	The half lengths of the extent of the box. 
INLINE AABB AABB::fromCenterAndHalfLength( Math::Vector3 const& center, Math::Vector3 const& halfLength ) {
	return AABB( center - halfLength, center + halfLength );
}

/// \fn	INLINE bool AABB::isValid() const
/// \brief	Query if this object is valid.
/// \return	true if valid, false if not.
INLINE bool AABB::isValid() const
{
	return Math::LengthSquared(m_MaxExtent - m_MinExtent) > 0.0f;
}

/// \fn	INLINE const Math::Vector3& AABB::getMinExtent() const
/// \brief	Gets the minimum extent.
/// \return	The minimum extent.
INLINE Math::Vector3 const& AABB::getMinExtent() const
{
	return m_MinExtent;
}

/// \fn	INLINE const Math::Vector3& AABB::getMaxExtent() const
/// \brief	Gets the maximum extent.
/// \return	The maximum extent.
INLINE Math::Vector3 const& AABB::getMaxExtent() const
{
	return m_MaxExtent;
}


/// \fn	INLINE Math::Vector3 AABB::getBoxCenter() const
/// \brief	Gets the box center.
/// \return	The box center.
INLINE Math::Vector3 AABB::getBoxCenter() const
{
	return (m_MaxExtent + m_MinExtent) * 0.5f;
}

/// \fn	INLINE Math::Vector3 AABB::getHalfLength() const
/// \brief	Gets the half length.
/// \return	The half length.
INLINE Math::Vector3 AABB::getHalfLength() const
{
	return (m_MaxExtent - m_MinExtent) * 0.5f;
}

/// \fn	INLINE void AABB::setBoxCenter( const Math::Vector3& center )
/// \brief	Sets a box center.
/// \param	center	The center.
INLINE void AABB::setBoxCenter( Math::Vector3 const& center )
{
	Math::Vector3 const oldCenter = getBoxCenter();
	m_MinExtent = (m_MinExtent - oldCenter) + center;
	m_MaxExtent = (m_MaxExtent - oldCenter) + center;
}

/// \fn	INLINE void AABB::expandBy( const Math::Vector3& pnt )
/// \brief	Expand (AKA union) by a pnt.
/// \param	pnt	The pnt.
INLINE void AABB::expandBy( Math::Vector3 const& pnt )
{
	m_MinExtent = Math::Min( m_MinExtent, pnt );
	m_MaxExtent = Math::Max( m_MaxExtent, pnt );
}

/// \fn	INLINE void AABB::expandBy( const AABB& aabb )
/// \brief	Expand (AKA union) this box by another aabb.
/// \param	aabb	The aabb.
INLINE void AABB::expandBy( AABB const& aabb )
{
	m_MinExtent = Math::Min( m_MinExtent, aabb.m_MinExtent );
	m_MaxExtent = Math::Max( m_MaxExtent, aabb.m_MaxExtent );
}

/// \fn	INLINE void AABB::unionWith( const AABB& aabb )
/// \brief	Union this box with the parameter.
/// \param	aabb	The aabb.
INLINE void AABB::unionWith( AABB const& aabb )
{
	m_MinExtent = Math::Min( m_MinExtent, aabb.m_MinExtent );
	m_MaxExtent = Math::Max( m_MaxExtent, aabb.m_MaxExtent );
}

/// \fn	INLINE void AABB::intersectWith( const AABB& aabb )
/// \brief	This box becomes the intersection of the param and itself.
/// \param	aabb	The aabb.
INLINE void AABB::intersectWith( AABB const& aabb )
{
	m_MinExtent = Math::Max( m_MinExtent, aabb.m_MinExtent );
	m_MaxExtent = Math::Min( m_MaxExtent, aabb.m_MaxExtent );
}

/// \fn	INLINE bool AABB::intersects( const AABB& aabb ) const
/// \brief	Intersects.
/// \todo	Fill in detailed method description.
/// \param	aabb	The aabb.
/// \return	true if it succeeds, false if it fails.
INLINE bool AABB::intersects( AABB const& aabb ) const
{
	Math::Vector3 const minI = Math::Max( m_MinExtent, aabb.m_MinExtent );
	Math::Vector3 const maxI = Math::Min( m_MaxExtent, aabb.m_MaxExtent );

	// TODO replace with simd compare and collapse result
	return( (minI[0] < maxI[0]) && (minI[1] < maxI[1]) && (minI[2] < maxI[2]) );
}

/// \fn	INLINE bool AABB::intersects( const Math::Vector3& _point ) const
/// \brief	Intersects.
/// \param	_point	point to test.
/// \return	true if it succeeds, false if it fails.
INLINE bool AABB::intersects( Math::Vector3 const& _point ) const
{
	return( (_point[0] > m_MinExtent[0] && _point[0] < m_MaxExtent[0]) &&
			(_point[1] > m_MinExtent[1] && _point[1] < m_MaxExtent[1]) &&
			(_point[2] > m_MinExtent[2] && _point[2] < m_MaxExtent[2]) );
}

/// \fn	INLINE bool AABB::intersects( const Math::Plane& plane ) const
/// \brief	Intersects.
/// \param	_point	point to test.
/// \return	true if it succeeds, false if it fails.
INLINE bool AABB::intersects( Math::Plane const& p ) const
{
	using namespace Math;
	Vector3 c = getBoxCenter();
	Vector3 e = m_MaxExtent - c;

	// collapse extents onto the plane normal (given a projected radius)
	float r = e.x * std::abs(p.a) + e.y * std::abs(p.b) + e.z * std::abs(p.c);
	// now project the center point on the plane
	float s = DotPoint(p, c);

	// if its withing the radius they intersect
	return std::abs(s) <= r;
}
