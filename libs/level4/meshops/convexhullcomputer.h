//
// Created by Computer on 01/08/2018.
//

#ifndef MESHOPS_CONVEXHULLCOMPUTER_H
#define MESHOPS_CONVEXHULLCOMPUTER_H

#include "core/core.h"
#include "meshmod/mesh.h"

namespace VHACD
{
	class IVHACD;
}

namespace MeshOps {

struct ConvexHullProgessCallback;

class ConvexHullParameters {
public:

	// overallProgress, stageProgress, operationProgress, stage, const operation);
	using ProgressCallback = void(float, float, float, char const*, char const*);

	ConvexHullParameters(void)
	{
		resolution = 100000;
		concavity = 0.001f;
		planeDownsampling = 4;
		convexhullDownsampling = 4;
		alpha = 0.05f;
		beta = 0.05f;
		pca = 0;
		mode = 0; // 0: voxel-based (recommended), 1: tetrahedron-based
		maxNumVerticesPerCH = 64;
		minVolumePerCH = 0.0001f;
		convexhullApproximation = true;
		maxConvexHulls = 1024;
		projectHullVertices = true; // This will project the output convex hull vertices onto the original source mesh to increase the floating point accuracy of the results
		convexHullProgressCallback = nullptr;
	}

	float concavity;
	float alpha;
	float beta;
	float minVolumePerCH;
	uint32_t resolution;
	uint32_t maxNumVerticesPerCH;
	uint32_t planeDownsampling;
	uint32_t convexhullDownsampling;
	uint32_t pca;
	uint32_t mode;
	uint32_t convexhullApproximation;
	uint32_t maxConvexHulls;
	bool projectHullVertices;
	ProgressCallback *convexHullProgressCallback;
};
class ConvexHullComputer
{
public:
	using ReturnType = std::pair<VHACD::IVHACD*, ConvexHullProgessCallback*>;
	using Ptr = std::shared_ptr<ReturnType>;
	static auto generate(MeshMod::Mesh::Ptr const& in_, ConvexHullParameters const& parameters_) -> std::vector<MeshMod::Mesh::Ptr>;
	static auto generateInline(MeshMod::Mesh::Ptr& in_) -> void;

	static auto createAsync(MeshMod::Mesh::Ptr const& in_, ConvexHullParameters const& parameters_)->Ptr;
	static auto isReady(Ptr ptr_) -> bool;
	static auto getResults(Ptr ptr_) -> std::vector<MeshMod::Mesh::Ptr>;

private:
	// dependent on whether createAsync or generate were called begin will be blocking or async
	static auto begin(Ptr ptr_, MeshMod::Mesh::Ptr const& in_, ConvexHullParameters const& parameters_) -> bool;

};

}
#endif //MESHOPS_CONVEXHULLCOMPUTER_H
