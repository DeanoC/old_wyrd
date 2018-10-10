#pragma once
#ifndef MESHOPS_PLATONICSOLIDS_H
#define MESHOPS_PLATONICSOLIDS_H

#include "meshops/meshops.h"
#include "geometry/aabb.h"
namespace MeshOps {

class PlatonicSolids {
public:
	static MeshMod::MeshPtr createTetrahedon();
	static MeshMod::MeshPtr createOctahedron();
	static MeshMod::MeshPtr createCube();
	static MeshMod::MeshPtr createIcosahedron();
	static MeshMod::MeshPtr createDodecahedron();

	static MeshMod::MeshPtr createBoxFrom(Geometry::AABB const& box);

private:
};

}

#endif //MESHOPS_PLATONICSOLIDS_H
