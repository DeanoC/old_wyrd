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
	VertexIndexContainer faces[4] = {
			{0, 1, 2},
			{2, 3, 0},
			{2, 1, 3},
			{3, 1, 0},
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
	assert(mesh->getHalfEdges().getCount()/2 == 6);

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
	static const VertexIndexContainer faces[] = {
			{0, 3, 5},
			{0, 5, 2},
			{4, 3, 0},
			{4, 0, 2},
			{5, 3, 1},
			{5, 1, 2},
			{4, 1, 3},
			{4, 2, 1},
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
	assert(mesh->getHalfEdges().getCount()/2 == 12);

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
	static const VertexIndexContainer faces[] = {
			{0, 1, 2, 3},
			{7, 6, 5, 4},
			{4, 0, 3, 7},
			{5, 6, 2, 1},
			{5, 1, 0, 4},
			{2, 6, 7, 3}
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
	assert(mesh->getHalfEdges().getCount()/2 == 12);

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

	static const VertexIndexContainer faces[] = {
			{0, 1, 2},
			{0, 2, 9},
			{0, 7, 8},
			{0, 8, 1},
			{0, 9, 7},
			{1, 6, 3},
			{1, 8, 6},
			{1, 3, 2},
			{2, 3, 5},
			{2, 5, 9},
			{3, 4, 5},
			{3, 6, 4},
			{4, 6, 10},
			{4, 10, 11},
			{5, 4, 11},
			{5, 11, 9},
			{6, 8, 10},
			{7, 9, 11},
			{7, 10, 8},
			{7, 11, 10 },

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
	assert(mesh->getHalfEdges().getCount()/2 == 30);

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
	const VertexIndexContainer faces[] = {
		{ 0, 1, 2, 3 },
		{ 7, 6, 5, 4 },
		{ 4, 0, 3, 7 },
		{ 5, 6, 2, 1 },
		{ 5, 1, 0, 4 },
		{ 2, 6, 7, 3 }
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
	assert(mesh->getHalfEdges().getCount() / 2 == 12);

	return std::move(mesh);
}

}