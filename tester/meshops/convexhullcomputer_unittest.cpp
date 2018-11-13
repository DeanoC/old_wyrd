#include "tester/catch.hpp"

#include "core/core.h"
#include "meshmod/mesh.h"
#include "meshops/basicmeshops.h"
#include "meshops/platonicsolids.h"
#include "meshops/convexhullcomputer.h"

TEST_CASE("Convex hull - platonic solids", "[MeshOps/ConvexHullComputer]")
{
	auto cube = MeshOps::PlatonicSolids::createCube();
	REQUIRE(cube);
	MeshOps::BasicMeshOps::triangulate(cube);
	REQUIRE(cube);
	MeshOps::ConvexHullParameters defaults;
	auto cubeConvexHulls = MeshOps::ConvexHullComputer::generate(cube, defaults);

	REQUIRE(cubeConvexHulls.size() == 1);
	REQUIRE(cubeConvexHulls[0]);
	MeshMod::Mesh::Ptr cubeCVH = cubeConvexHulls[0];
	REQUIRE(cubeCVH->getVertices().getCount() == 8);
	REQUIRE(cubeCVH->getPolygons().getCount() == 12);
	REQUIRE(cubeCVH->getHalfEdges().getCount() / 2 == 18);
}