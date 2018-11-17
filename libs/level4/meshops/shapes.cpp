#include "core/core.h"
#include "meshops/shapes.h"
#include "math/vector_math.h"
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

namespace MeshOps {

auto Shapes::createDiamond() -> std::unique_ptr<MeshMod::Mesh>
{
	using namespace MeshMod;
	using namespace Math;
	auto mesh = std::make_unique<Mesh>("Diamond");

	static vec3 const pos[] = {
			{-0.5f, 0,  -0.5f},
			{0.5f,  0,  -0.5f},
			{0.5f,  0,  0.5f},
			{-0.5f, 0,  0.5f},

			{0,     1,  0},
			{0,     -1, 0},
	};

	VertexIndexContainer const faces[] = {
			{0, 4, 1},
			{1, 4, 2},
			{2, 4, 3},
			{3, 4, 0},
			{0, 5, 1},
			{1, 5, 2},
			{2, 5, 3},
			{3, 5, 0},
	};
	for(auto p : pos)
	{
		mesh->getVertices().add(p.x, p.y, p.z);
	}
	for(auto f : faces)
	{
		mesh->getPolygons().add(f);
	}
	mesh->updateEditState(MeshMod::Mesh::TopologyEdits);
	mesh->updateFromEdits();

	assert(mesh->getVertices().getCount() == 6);
	assert(mesh->getPolygons().getCount() == 8);
	assert(mesh->getHalfEdges().getCount() / 2 == 12);

	return std::move(mesh);

}

}