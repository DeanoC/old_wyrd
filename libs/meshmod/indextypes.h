#pragma once
/** \file MeshModIndexTypes.h
	MeshMod is largely index based, therefore these are the basic
	unit used by this library
   (c) 2006 Dean Calver
 */

#if !defined( MESH_MOD_INDEX_TYPES_H_ )
#define MESH_MOD_INDEX_TYPES_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <vector>

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{

	//---------------------------------------------------------------------------
	// Classes and structures
	//---------------------------------------------------------------------------
	//! vertex index type.
	typedef uint32_t VertexIndex;
	//! half edge index type.
	typedef uint32_t HalfEdgeIndex;
	//! face index type
	typedef uint32_t PolygonIndex;
	//! material index type
	typedef uint32_t MaterialIndex;
	//! scene node index
	typedef uint32_t SceneNodeIndex;

	//! a list of vertex indices
	typedef std::vector<VertexIndex> VertexIndexContainer;
	//! a list of half edge indices
	typedef std::vector<HalfEdgeIndex> HalfEdgeIndexContainer;
	//! a list of face indices
	typedef std::vector<PolygonIndex> PolygonIndexContainer;

	//! value used to indicate an index is not assigned/deleted/invalid etc.
	static const uint32_t MM_INVALID_INDEX = (~0);

} // end namespace

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Externals
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// End Header file
//---------------------------------------------------------------------------
#endif
