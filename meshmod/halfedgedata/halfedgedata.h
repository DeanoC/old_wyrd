#pragma once
/** \file halfedgedata.h
   A half edge type.
   (c) 2002 Dean Calver
 */

#if !defined( MESH_MOD_HALFEDGEDATA_H_ )
#define MESH_MOD_HALFEDGEDATA_H_

#include "meshmod/meshmod.h"
#include "meshmod/varicontainer.h"
#include "meshmod/varielements.h"

namespace MeshMod { namespace HalfEdgeData {
struct HalfEdge
{
	//! index of vertex that starts this half edge
	VertexIndex startVertexIndex = MM_INVALID_INDEX;
	//! index of vertex that ends this half edge
	VertexIndex endVertexIndex = MM_INVALID_INDEX;
	//! polygon this edge is connected to
	PolygonIndex polygonIndex = MM_INVALID_INDEX;

	//! the other half edge this half edges is paired with
	HalfEdgeIndex pair = MM_INVALID_INDEX;

	//! previous half edge in polygon (counter clockwise)
	HalfEdgeIndex prev = MM_INVALID_INDEX;

	//! next half edge in polygon (clockwise)
	HalfEdgeIndex next = MM_INVALID_INDEX;

	static const std::string getName() { return "HalfEdge"; };
};
}} // end namespace

#endif //MESH_HALFEDGEDATA_H
