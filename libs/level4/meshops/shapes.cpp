#include "core/core.h"
#include "meshops/shapes.h"
#include "math/vector_math.h"
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
#include "meshops/platonicsolids.h"
#include "meshops/basicmeshops.h"
#include "fmt/format.h"

namespace MeshOps {

auto Shapes::CreateDiamond() -> std::unique_ptr<MeshMod::Mesh>
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
			{vi(0), vi(4), vi(1)},
			{vi(1), vi(4), vi(2)},
			{vi(2), vi(4), vi(3)},
			{vi(3), vi(4), vi(0)},
			{vi(0), vi(5), vi(1)},
			{vi(1), vi(5), vi(2)},
			{vi(2), vi(5), vi(3)},
			{vi(3), vi(5), vi(0)},
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

auto Shapes::CreateSphere(uint32_t subdivisionSteps_) -> std::unique_ptr<MeshMod::Mesh>
{
	using namespace MeshMod;
	using namespace Math;
	std::shared_ptr<Mesh> mesh = PlatonicSolids::CreateIcosahedron();

	for(auto i = 0u; i < subdivisionSteps_; ++i)
	{
		mesh = std::move(BasicMeshOps::tesselate4(std::move(mesh)));
	}

	BasicMeshOps::spherize(mesh, 1.0f);
	mesh->updateEditState(MeshMod::Mesh::PositionEdits);
	mesh->updateFromEdits();

	return mesh->clone();
}

auto Shapes::CreateAABB(Geometry::AABB const& aabb_) -> std::unique_ptr<MeshMod::Mesh>
{
	using namespace MeshMod;
	using namespace Math;
	auto mesh = std::make_unique<Mesh>("Box");

	Math::vec3 const minBox = aabb_.getMinExtent();
	Math::vec3 const maxBox = aabb_.getMaxExtent();

	vec3 const pos[]{
			{minBox.x, maxBox.y, minBox.z},
			{minBox.x, minBox.y, minBox.z},
			{maxBox.x, minBox.y, minBox.z},
			{maxBox.x, maxBox.y, minBox.z},

			{minBox.x, maxBox.y, maxBox.z},
			{minBox.x, minBox.y, maxBox.z},
			{maxBox.x, minBox.y, maxBox.z},
			{maxBox.x, maxBox.y, maxBox.z},
	};
	using vi = VertexIndex;
	const VertexIndexContainer faces[] = {
			{vi(0), vi(1), vi(2), vi(3)},
			{vi(7), vi(6), vi(5), vi(4)},
			{vi(4), vi(0), vi(3), vi(7)},
			{vi(5), vi(6), vi(2), vi(1)},
			{vi(5), vi(1), vi(0), vi(4)},
			{vi(2), vi(6), vi(7), vi(3)}
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

	assert(mesh->getVertices().getCount() == 8);
	assert(mesh->getPolygons().getCount() == 6);
	assert(mesh->getHalfEdges().getCount() == 24);

	return std::move(mesh);
}

auto Shapes::CreateSquare(Math::vec3 const& center_, Math::vec3 const& forward_) -> std::unique_ptr<MeshMod::Mesh>
{
	using namespace MeshMod;
	using namespace Math;
	auto mesh = std::make_unique<Mesh>("Square");

	vec3 up{0, 1, 0};
	float dot = Math::dot(forward_, up);
	if(ApproxEqual(dot, 1.0f))
	{
		up = {0, 0, 1};
	}
	vec3 forward = forward_ * 0.5f;
	vec3 right = cross(forward_, up) * 0.5f;
	up = up * 0.5f;
	vec3 const tl = center_ + -right + up;
	vec3 const tr = center_ + right + up;
	vec3 const ul = center_ + -right - up;
	vec3 const ur = center_ + right - up;

	vec3 const pos[]{ tl, tr, ur, ul };

	using vi = VertexIndex;
	VertexIndexContainer const faces[] = {
			{vi(0), vi(1), vi(2), vi(3)},
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

	return std::move(mesh);
}

}