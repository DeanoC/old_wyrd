#pragma once
/** \file polygondata.h
   (c) 2002 Dean Calver
 */
#if !defined( MESH_MOD_POLYGONDATA_H_ )
#define MESH_MOD_POLYGONDATA_H_

#include "meshmod/meshmod.h"

namespace MeshMod { namespace PolygonData {

struct Polygon
{
	// one of the half edge (which one doesn't matter)
	HalfEdgeIndex anyHalfEdge = MM_INVALID_INDEX;

	static const std::string getName() { return "Polygon"; };
};

struct PlaneEquation
{
	Math::Plane planeEq;

	static const std::string getName() { return "PlaneEquation"; };
};

struct SortIndex
{
	size_t index;

	static const std::string getName() { return "SortIndex"; };
};

struct Material {
	MaterialIndex surfaceIndex;

	static const std::string getName() { return "Material"; };
};

} } // end namespace
#endif
