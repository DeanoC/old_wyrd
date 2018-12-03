#include "core/core.h"
#include "meshops/meshsorter.h"
#include "meshmod/scene.h"
#include "meshmod/sceneobject.h"
#include "meshmod/vertices.h"
#include "meshmod/halfedges.h"
#include "meshmod/polygons.h"
#include "meshmod/mesh.h"
#include "meshmod/vertexdata/vertexdata.h"
#include "meshmod/vertexdata/uvvertex.h"
#include "meshmod/vertexdata/positionvertex.h"
#include "meshmod/vertexdata/pointrepvertex.h"
#include "meshmod/halfedgedata/halfedgecontainers.h"
#include "meshmod/polygonsdata/polygoncontainers.h"
#include <cassert>
#include <algorithm>
#include <set>

namespace MeshOps {

MeshSorter::MeshSorter(std::shared_ptr<MeshMod::Mesh> const& mesh_) :
		mesh(mesh_)
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
	MeshMod::VertexData::Axis axis;

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
	auto& sortAttr = polygons.getOrAddAttribute<PolygonData::SortIndices>( "MaterialIndex" );

	// fill sort mapper with identity mapping
	polygons.visitAll([&sortAttr](PolygonIndex const pi_){
		sortAttr[pi_].index = pi_;
	});

	// if we have no surface materinal indices ther is no sorting to do
	if( polygons.hasAttribute<PolygonData::Materials>()) {
		MatSort_Local sorter;
		sorter.surfElement = polygons.getPolygonsContainer().getElement<PolygonData::Materials>();
		std::sort( sortAttr.begin(), sortAttr.end(), sorter );
	}

	return polygons.getPolygonsContainer().getElement<PolygonData::SortIndices>("MaterialIndex");
}

/**
*/
std::shared_ptr<MeshMod::VertexData::SortIndices> MeshSorter::sortVerticesByAxis(MeshMod::VertexData::Axis axis)
{
	using namespace MeshMod;
	// we create a sort mapper, that which vertex would be here if sort by
	// indicated type
	// i.e. Sorted.vertex0 = Vertex[ SortMapper[0] ]

	static const char* axisNames[] = { "X axis", "Y axis", "Z axis" };

	auto& vertices = mesh->getVertices();
	auto& sortAttr = vertices.getOrAddAttribute<VertexData::SortIndices>(axisNames[(uint8_t) axis]);

	// fill sort mapper with identity mapping
	vertices.visitAll([&sortAttr](VertexIndex const vi_){
		sortAttr[vi_].index = vi_;
	});

	AxisSortCompare sorter;
	sorter.posElement = vertices.getVerticesContainer().getElement<VertexData::Positions>();
	std::sort( sortAttr.begin(), sortAttr.end(), sorter );

	return vertices.getVerticesContainer().getElement<VertexData::SortIndices>(axisNames[(uint8_t) axis]);

}


}