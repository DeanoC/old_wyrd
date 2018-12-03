#include "core/core.h"
#include "geometry/aabb.h"
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
#include "math/vector_math.h"
#include "meshops/platonicsolids.h"

namespace MeshOps {
auto PlatonicSolids::createTetrahedon() -> std::unique_ptr<MeshMod::Mesh>
{
	using namespace MeshMod;
	using namespace Math;
	auto mesh = std::make_unique<Mesh>("Tetrahedron");

	vec3 pos[4] = {
			{1,  1,  1},
			{1,  -1, 1},
			{-1, 1,  1},
			{-1, 1,  -1},
	};
	using vi = VertexIndex;
	VertexIndexContainer faces[4] = {
		{ vi(0), vi(1), vi(2) },
		{ vi(2), vi(3), vi(0) },
		{ vi(2), vi(1), vi(3) },
		{ vi(3), vi(1), vi(0) },
	};
	for(auto p : pos)
	{
		mesh->getVertices().add(p.x, p.y, p.z);
	}
	for(auto f : faces)
	{
		mesh->getPolygons().add(f);
	}
	mesh->updateEditState(Mesh::TopologyEdits);

	assert(mesh->getVertices().getCount() == 4);
	assert(mesh->getPolygons().getCount() == 4);
	assert(mesh->getHalfEdges().getCount() == 12);

	return std::move(mesh);
}

auto PlatonicSolids::createOctahedron() -> std::unique_ptr<MeshMod::Mesh>
{
	using namespace MeshMod;
	using namespace Math;
	auto mesh = std::make_unique<Mesh>("Octahedron");

	static const vec3 pos[] = {
			{-1, 0,  0},
			{1,  0,  0},
			{0,  -1, 0},
			{0,  1,  0},
			{0,  0,  -1},
			{0,  0,  1},
	};
	using vi = VertexIndex;
	static const VertexIndexContainer faces[] = {
		{ vi(0), vi(3), vi(5) },
		{ vi(0), vi(5), vi(2) },
		{ vi(4), vi(3), vi(0) },
		{ vi(4), vi(0), vi(2) },
		{ vi(5), vi(3), vi(1) },
		{ vi(5), vi(1), vi(2) },
		{ vi(4), vi(1), vi(3) },
		{ vi(4), vi(2), vi(1) },
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
	assert(mesh->getHalfEdges().getCount() == 24);

	return std::move(mesh);
}

auto PlatonicSolids::createCube() -> std::unique_ptr<MeshMod::Mesh>
{
	using namespace MeshMod;
	using namespace Math;
	auto mesh = std::make_unique<Mesh>("Cube");

	static const vec3 pos[] = {
		{-1,  1, -1},
		{-1, -1, -1},
		{ 1, -1, -1},
		{ 1,  1, -1},

		{-1,  1,  1},
		{-1, -1,  1},
		{ 1, -1,  1},
		{ 1,  1,  1},
	};
	using vi = VertexIndex;
	static const VertexIndexContainer faces[] = {
		{ vi(0), vi(1), vi(2), vi(3) },
		{ vi(7), vi(6), vi(5), vi(4) },
		{ vi(4), vi(0), vi(3), vi(7) },
		{ vi(5), vi(6), vi(2), vi(1) },
		{ vi(5), vi(1), vi(0), vi(4) },
		{ vi(2), vi(6), vi(7), vi(3) }
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

	assert(mesh->getVertices().getCount() == 8);
	assert(mesh->getPolygons().getCount()  == 6);
	assert(mesh->getHalfEdges().getCount() == 24);

	return std::move(mesh);

}

auto PlatonicSolids::createIcosahedron() -> std::unique_ptr<MeshMod::Mesh>
{
	using namespace MeshMod;
	using namespace Math;

	using namespace MeshMod;
	using namespace Math;
	auto mesh = std::make_unique<Mesh>("Icosahedron");

	// Phi - the square root of 5 plus 1 divided by 2
	double phi = (1.0 + sqrt(5.0)) * 0.5;

	float a = 0.5f;
	float b = (float)(1.0 / (2.0f * phi));

	// scale a bit TODO do this properly
	a = a * 2;
	b = b * 2;

	static const vec3 pos[] = {
		{0,  b,  -a},
		{b,  a,  0},
		{-b, a,  0},
		{0,  b,  a},
		{0,  -b, a},
		{-a, 0,  b},
		{a,  0,  b},
		{0,  -b, -a},
		{a,  0,  -b},
		{-a, 0,  -b},
		{b,  -a, 0},
		{-b, -a, 0}
	};

	using vi = VertexIndex;
	static const VertexIndexContainer faces[] = {
		{ vi(0), vi(1), vi(2) },
		{ vi(0), vi(2), vi(9) },
		{ vi(0), vi(7), vi(8) },
		{ vi(0), vi(8), vi(1) },
		{ vi(0), vi(9), vi(7) },
		{ vi(1), vi(6), vi(3) },
		{ vi(1), vi(8), vi(6) },
		{ vi(1), vi(3), vi(2) },
		{ vi(2), vi(3), vi(5) },
		{ vi(2), vi(5), vi(9) },
		{ vi(3), vi(4), vi(5) },
		{ vi(3), vi(6), vi(4) },
		{ vi(4), vi(6), vi(10) },
		{ vi(4), vi(10),vi(11) },
		{ vi(5), vi(4), vi(11) },
		{ vi(5), vi(11),vi(9) },
		{ vi(6), vi(8), vi(10) },
		{ vi(7), vi(9), vi(11) },
		{ vi(7), vi(10),vi(8) },
		{ vi(7), vi(11),vi(10) },
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

	assert(mesh->getVertices().getCount() == 12);
	assert(mesh->getPolygons().getCount() == 20);
	assert(mesh->getHalfEdges().getCount() == 60);

	return mesh;
}


auto PlatonicSolids::createBoxFrom(Geometry::AABB const& aabb) -> std::unique_ptr<MeshMod::Mesh>
{
	using namespace MeshMod;
	using namespace Math;
	auto mesh = std::make_unique<Mesh>("Box");

	Math::vec3 const minBox = aabb.getMinExtent();
	Math::vec3 const maxBox = aabb.getMaxExtent();

	const vec3 pos[] {
		{ minBox.x, maxBox.y, minBox.z },
		{ minBox.x, minBox.y, minBox.z },
		{ maxBox.x, minBox.y, minBox.z },
		{ maxBox.x, maxBox.y, minBox.z },

		{ minBox.x, maxBox.y,  maxBox.z },
		{ minBox.x, minBox.y,  maxBox.z },
		{ maxBox.x, minBox.y,  maxBox.z },
		{ maxBox.x, maxBox.y,  maxBox.z },
	};
	using vi = VertexIndex;
	const VertexIndexContainer faces[] = {
		{ vi(0), vi(1), vi(2), vi(3) },
		{ vi(7), vi(6), vi(5), vi(4) },
		{ vi(4), vi(0), vi(3), vi(7) },
		{ vi(5), vi(6), vi(2), vi(1) },
		{ vi(5), vi(1), vi(0), vi(4) },
		{ vi(2), vi(6), vi(7), vi(3) }
	};
	for (auto p : pos)
	{
		mesh->getVertices().add(p.x, p.y, p.z);
	}
	for (auto f : faces)
	{
		mesh->getPolygons().add(f);
	}
	mesh->updateEditState(MeshMod::Mesh::TopologyEdits);
	mesh->updateFromEdits();

	assert(mesh->getVertices().getCount() == 8);
	assert(mesh->getPolygons().getCount() == 6);
	assert(mesh->getHalfEdges().getCount() == 24);

	return std::move(mesh);
}

}