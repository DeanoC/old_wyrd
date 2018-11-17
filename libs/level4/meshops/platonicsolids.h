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
	static auto createTetrahedon() -> std::unique_ptr<MeshMod::Mesh>;
	static auto createOctahedron() -> std::unique_ptr<MeshMod::Mesh>;
	static auto createCube() -> std::unique_ptr<MeshMod::Mesh>;
	static auto createIcosahedron() -> std::unique_ptr<MeshMod::Mesh>;
	// TODO	static auto createDodecahedron() -> std::unique_ptr<MeshMod::Mesh>;

	static auto createBoxFrom(Geometry::AABB const& box) -> std::unique_ptr<MeshMod::Mesh>;

private:
};

}

#endif //WYRD_MESHOPS_PLATONICSOLIDS_H
