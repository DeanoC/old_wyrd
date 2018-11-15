#pragma once
#ifndef NATIVESNAPSHOT_TACTICALMAP_STITCHER_H
#define NATIVESNAPSHOT_TACTICALMAP_STITCHER_H

#include "core/core.h"
#include "math/vector_math.h"
#include "geometry/aabb.h"
#include <memory>
#include <vector>
#include "tacticalmap/tacticalmap.h"

class TacticalMapStitcher : public ITacticalMapStitcher
{
public:
	void addTacticalMapInstance(TacticalMap::ConstPtr map_, Math::vec3 const position_, int rotationInDegrees_, int mapParcelId_) override;
	TacticalMap::Ptr build() override;
private:
	Math::vec3 rotateVector(Math::vec3 const& v, int rotationInDegrees_);
	Geometry::AABB rotateAABB(Geometry::AABB const& v, int rotationInDegrees_);
	using Instances = std::tuple<TacticalMap::ConstPtr, Math::vec3, int, unsigned int>;
	std::vector<Instances> instances;
};

#endif //NATIVESNAPSHOT_TACTICALMAP_STITCHER_H
