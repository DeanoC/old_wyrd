#pragma once
#ifndef WYRD_MESHOP_SHAPES_H
#define WYRD_MESHOP_SHAPES_H

#include "core/core.h"

namespace MeshMod {
class Mesh;
}
namespace MeshOps {

class Shapes
{
public:
	static auto createDiamond() -> std::unique_ptr<MeshMod::Mesh>;
	static auto createSphere(uint32_t subdivisionSteps_ = 2u) -> std::unique_ptr<MeshMod::Mesh>;

};

}

#endif //WYRD_MESHOP_SHAPES_H
