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
#include "meshops/platonicsolids.h"
#include "meshops/basicmeshops.h"
#include "fmt/format.h"

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

	using vi = VertexIndex;

	VertexIndexContainer const faces[] = {
			{vi(0), vi(4), vi(1) },
			{vi(1), vi(4), vi(2) },
			{vi(2), vi(4), vi(3) },
			{vi(3), vi(4), vi(0) },
			{vi(0), vi(5), vi(1) },
			{vi(1), vi(5), vi(2) },
			{vi(2), vi(5), vi(3) },
			{vi(3), vi(5), vi(0) },
	};
	for(auto p : pos)
	{
		mesh->getVertices().add(p.x, p.y, p.z);
	}
	for(auto f : faces)
	{
		mesh->getPolygons().addPolygon(f);
	}
	mesh->updateEditState(MeshMod::Mesh::TopologyEdits);
	mesh->updateFromEdits();

	assert(mesh->getVertices().getCount() == 6);
	assert(mesh->getPolygons().getCount() == 8);
	assert(mesh->getHalfEdges().getCount() == 24);

	return std::move(mesh);

}

auto Shapes::createSphere(uint32_t subdivisionSteps_) -> std::unique_ptr<MeshMod::Mesh>
{
	using namespace MeshMod;
	using namespace Math;
	std::shared_ptr<Mesh> mesh = PlatonicSolids::createIcosahedron();

	for(auto i = 0u; i < subdivisionSteps_; ++i)
	{
		mesh = std::move(BasicMeshOps::tesselate4(std::move(mesh)));
	}

	// TODO fix weld functionality
	//	mesh->getVertices().removeAllSimilarPositions();

	mesh->updateEditState(MeshMod::Mesh::TopologyEdits);
	mesh->updateFromEdits();


	BasicMeshOps::spherize(mesh, 1.0f);
	mesh->updateEditState(MeshMod::Mesh::PositionEdits);
	mesh->updateFromEdits();

	return mesh->clone();
}

}