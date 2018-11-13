#pragma once
#ifndef MESHOPS_PLATONICSOLIDS_H
#define MESHOPS_PLATONICSOLIDS_H

#include "meshops/meshops.h"
#include "geometry/aabb.h"
namespace MeshOps {

class PlatonicSolids {
public:
	static MeshMod::Mesh::Ptr createTetrahedon();
	static MeshMod::Mesh::Ptr createOctahedron();
	static MeshMod::Mesh::Ptr createCube();
	static MeshMod::Mesh::Ptr createIcosahedron();
	static MeshMod::Mesh::Ptr createDodecahedron();

	static MeshMod::Mesh::Ptr createBoxFrom(Geometry::AABB const& box);

private:
};

}

#endif //MESHOPS_PLATONICSOLIDS_H
