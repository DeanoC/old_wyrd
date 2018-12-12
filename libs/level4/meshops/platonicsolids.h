#pragma once
#ifndef WYRD_MESHOPS_PLATONICSOLIDS_H
#define WYRD_MESHOPS_PLATONICSOLIDS_H

#include "core/core.h"

namespace Geometry {
class AABB;
}
namespace MeshMod {
class Mesh;
}

namespace MeshOps {

class PlatonicSolids {
public:
	static auto CreateTetrahedon() -> std::unique_ptr<MeshMod::Mesh>;
	static auto CreateOctahedron() -> std::unique_ptr<MeshMod::Mesh>;
	static auto CreateCube() -> std::unique_ptr<MeshMod::Mesh>;
	static auto CreateIcosahedron() -> std::unique_ptr<MeshMod::Mesh>;
	// TODO	static auto CreateDodecahedron() -> std::unique_ptr<MeshMod::Mesh>;

private:
};

}

#endif //WYRD_MESHOPS_PLATONICSOLIDS_H
