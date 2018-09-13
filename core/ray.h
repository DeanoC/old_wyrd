///-------------------------------------------------------------------------------------------------
/// \file	core\ray.h
/// \brief	Declares the ray class.
/// \remark	Copyright (c) 2011 Dean Calver. Public Domain.
/// \remark	mailto://deano@cloudpixies.com
/// \brief	A class representing a ray

#pragma once
#ifndef CORE_RAY_H
#define CORE_RAY_H

#include "vector_math.h"
#include "aabb.h"

namespace Core {

//! \class	Ray
//! \brief a infinite ray having an start and a direction.
class Ray {
public:
	Ray();
	Ray( Math::Vector3 const& origin, Math::Vector3 const& direction );

	Math::Vector3 const& getOrigin() const { return m_origin; }
	Math::Vector3 const& getDirection() const { return m_direction; }

	//! Ray-AABB intersection test.
	bool intersectsAABB( AABB const& bounds, float& min, float& max ) const;

	//! Ray-triangle intersection test.
	bool intersectsTriangle( 
		Math::Vector3 const& v0, Math::Vector3 const& v1, Math::Vector3 const& v2, 
		float& v, float& w, float& t 
		) const;
	bool intersectsTriangleBackFaceCull(
		Math::Vector3 const& v0, Math::Vector3 const& v1, Math::Vector3 const& v2,
		float& v, float& w, float& t
	) const;
private:
	Math::Vector3 m_origin;
	Math::Vector3 m_direction;
};

}// end namespace

#endif