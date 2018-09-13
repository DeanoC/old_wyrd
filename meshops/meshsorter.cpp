/** \file meshsorter.cpp
   Geometry Optimiser Mesh sorting operations.
   (c) 2012 Deano Calver
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "meshops.h"
#include <cassert>
#include <algorithm>
#include <set>
#include <meshmod/vertices.h>
#include <meshmod/halfedges.h>
#include <meshmod/polygons.h>
#include <meshmod/mesh.h>
#include <meshmod/vertexdata/positionvertex.h>
#include <meshmod/vertexdata/pointrepvertex.h>
#include <meshmod/halfedgedata/halfedgecontainers.h>
#include "meshsorter.h"

namespace MeshOps {

MeshSorter::MeshSorter( const MeshMod::MeshPtr& _mesh ) :
	mesh( _mesh )
{
}

namespace {
class  MatSort_Local
{
public:
	std::shared_ptr<MeshMod::PolygonData::Materials> surfElement;

	/// function to sort surface by index
	bool operator() ( const MeshMod::PolygonData::SortIndex& a, const MeshMod::PolygonData::SortIndex& b) const {
		return ( (*surfElement)[a.index].surfaceIndex < (*surfElement)[b.index].surfaceIndex );
	}
};

//! sorts things based on the float value, you can then
//! get the original version via the index
struct AxisSortCompare {
	std::shared_ptr<MeshMod::VertexData::Positions> posElement;
	MeshMod::VertexData::Position::AXIS axis;

	/// function to sort by position[axis]
	bool operator() ( const MeshMod::VertexData::SortIndex& a, const MeshMod::VertexData::SortIndex& b) const {
		return ( (*posElement)[a.index].get(axis) < (*posElement)[b.index].get(axis) );
	}
};

}
/**
Sorts all face by face material index.
Adds a material index field and sorts it into material ordered list
*/
std::shared_ptr<MeshMod::PolygonData::SortIndices> MeshSorter::sortPolygonsByMaterialIndex() {
	using namespace MeshMod;
	// we create a sort mapper, that which face would be here if sort by
	// indicated type
	// i.e. Sorted.face0 = Polygon[ SortMapper[0] ]

	auto& polygons = mesh->getPolygons();
	PolygonElementsContainer& polygonCon = polygons.getPolygonsContainer();
	auto sortEle = polygonCon.getOrAddElements<PolygonData::SortIndices>( "MaterialIndex" );

	// fill sort mapper with identity mapping
	PolygonData::SortIndices::iterator idenIt = sortEle->elements.begin();
	while( idenIt != sortEle->elements.end() ) {
		(*idenIt).index = (unsigned int) std::distance(sortEle->elements.begin(), idenIt);
		++idenIt;
	}

	MatSort_Local sorter;
	sorter.surfElement = polygonCon.getElements<PolygonData::Materials>();

	// if we have no surface materinal indices ther is no sorting to do
	if( sorter.surfElement != 0) {
		std::sort( sortEle->elements.begin(), sortEle->elements.end(), sorter );
	}

	return sortEle;
}

/**
*/
std::shared_ptr<MeshMod::VertexData::SortIndices> MeshSorter::sortVerticesByAxis( MeshMod::VertexData::Position::AXIS axis ) {
	using namespace MeshMod;
	// we create a sort mapper, that which vertex would be here if sort by
	// indicated type
	// i.e. Sorted.vertex0 = Vertex[ SortMapper[0] ]

	static const char* axisNames[] = { "X axis", "Y axis", "Z axis" };

	VerticesElementsContainer& vertCon = mesh->getVertices().getVerticesContainer();
	auto sortEle = vertCon.getOrAddElements<VertexData::SortIndices>( axisNames[axis] );

	// fill sort mapper with identity mapping
	VertexData::SortIndices::iterator idenIt = sortEle->elements.begin();
	while( idenIt != sortEle->elements.end() ) {
		(*idenIt).index = (unsigned int) std::distance(sortEle->elements.begin(), idenIt);
		++idenIt;
	}

	AxisSortCompare sorter;
	sorter.posElement = vertCon.getElements<VertexData::Positions>();
	sorter.axis = axis;
	std::sort( sortEle->elements.begin(), sortEle->elements.end(), sorter );

	return sortEle;
}


}