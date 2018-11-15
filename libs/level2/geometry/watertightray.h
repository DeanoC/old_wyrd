///-------------------------------------------------------------------------------------------------
/// \file	geometry\watertightray.h
/// \brief	Declares the watertightray class.
/// \remark	Copyright (c) 2018 Dean Calver. Public Domain.
/// \remark	mailto://deano@cloudpixies.com
/// \brief	A class representing a water tight ray, no gaps but slower than ray based on
/// \brief http://jcgt.org/published/0002/01/05/paper.pdf

#pragma once
#ifndef WYRD_GEOMETRY_WATERTIGHTRAY_H
#define WYRD_GEOMETRY_WATERTIGHTRAY_H

#include "math/vector_math.h"
#include "geometry/aabb.h"

namespace Geometry {

//! \class	WaterTightRay
//! \brief a infinite water tight ray having an start and a direction.
class WaterTightRay {
public:
	WaterTightRay();
	WaterTightRay( Math::vec3 const& origin, Math::vec3 const& direction );

	Math::vec3 const& getOrigin() const { return origin; }
	Math::vec3 const& getDirection() const { return dir; }

	//! Ray-AABB intersection test.
	bool intersectsAABB( AABB const& bounds, float& min, float& max ) const;

	//! Ray-triangle intersection test.
	bool intersectsTriangle(
			Math::vec3 const& v0, Math::vec3 const& v1, Math::vec3 const& v2,
			float& v, float& w, float& t
	) const;
	bool intersectsTriangleBackFaceCull(
		Math::vec3 const& v0, Math::vec3 const& v1, Math::vec3 const& v2,
		float& v, float& w, float& t
	) const;

private:
	Math::vec3 origin;
	Math::vec3 dir;
	Math::vec3 rdir;
	int kx, ky, kz;
	Math::vec3 shear;
};

}// end namespace

#endif