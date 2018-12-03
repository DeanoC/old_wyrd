#include "core/core.h"
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
#include "convexhullcomputer.h"
#include "meshops/VHACD_Lib/public/VHACD.h"
#include <functional>
#include <thread>


namespace MeshOps {

struct ConvexHullProgessCallback : public VHACD::IVHACD::IUserCallback
{
	ConvexHullProgessCallback(ConvexHullParameters::ProgressCallback* callback_) : callback(callback_) {}

	void Update(const double overallProgress,
		const double stageProgress,
		const double operationProgress,
		const char* const stage,
		const char* const operation) final
	{
		if (callback)
		{
			callback((float)overallProgress, (float)stageProgress, (float)operationProgress, stage, operation);
		}
	}

	ConvexHullParameters::ProgressCallback* callback;
};
}

namespace
{
void ReturnDestroyer(MeshOps::ConvexHullComputer::ReturnType* b)
{
	if (b)
	{
		b->first->Release();
		if (b->second)
		{
			delete b->second;
		}
		delete b;
	}
}
}

namespace MeshOps {

auto ConvexHullComputer::generate(std::shared_ptr<MeshMod::Mesh> const& in_, ConvexHullParameters const& parameters_) -> std::vector<std::shared_ptr<MeshMod::Mesh>>
{
	auto ivhacd = std::shared_ptr<ReturnType>(new ReturnType(VHACD::CreateVHACD(), nullptr), ReturnDestroyer);

	if (begin(ivhacd, in_, parameters_) == false)
	{
		return {};
	}
	return getResults(ivhacd);
}

void ConvexHullComputer::generateInline( std::shared_ptr<MeshMod::Mesh>& in_ )
{
	MeshOps::ConvexHullParameters defaults;
	auto out = generate(in_, defaults);
	assert(out.size() == 1);
	in_.reset(out[0].get());
}

auto ConvexHullComputer::createAsync(std::shared_ptr<MeshMod::Mesh> const& in_,
										ConvexHullParameters const& parameters_
										) -> std::shared_ptr<ReturnType>
{
	ConvexHullProgessCallback* callback = nullptr;
	if (parameters_.convexHullProgressCallback)
	{
		callback = new ConvexHullProgessCallback{ parameters_.convexHullProgressCallback };
	}
	auto ivhacd = std::shared_ptr<ReturnType>(new ReturnType(VHACD::CreateVHACD_ASYNC(), callback), ReturnDestroyer);

	if (begin(ivhacd, in_, parameters_) == false)
	{
		return {};
	}

	return ivhacd;
}

auto ConvexHullComputer::isReady(std::shared_ptr<ReturnType> ptr_) -> bool
{
	if (ptr_)
	{
		return ptr_->first->IsReady();
	}
	return false;
}

auto ConvexHullComputer::getResults(std::shared_ptr<ReturnType> ptr_) -> std::vector<std::shared_ptr<MeshMod::Mesh>>
{
	if (ptr_)
	{
		// block if called before its ready
		while (ptr_->first->IsReady() == false)
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(16ms);
		}
		unsigned int nConvexHulls = ptr_->first->GetNConvexHulls();
		if (nConvexHulls == 0)
			return {};

		using namespace MeshMod;
		std::vector<std::shared_ptr<Mesh>> outArray(nConvexHulls);
		for (auto j = 0u; j < nConvexHulls; ++j)
		{
			VHACD::IVHACD::ConvexHull ch;
			ptr_->first->GetConvexHull(j, ch);

			auto out = std::make_shared<Mesh>("ConvexHull_" + std::to_string(j));
			for (auto i = 0u; i < ch.m_nPoints * 3; i += 3)
			{
				float x = (float)ch.m_points[i + 0];
				float y = (float)ch.m_points[i + 1];
				float z = (float)ch.m_points[i + 2];
				out->getVertices().add(x, y, z);
			}

			VertexIndexContainer triIndices(ch.m_nTriangles * 3);
			for (auto i = 0u; i < ch.m_nTriangles * 3; i += 3)
			{
				triIndices[i + 0] = VertexIndex(ch.m_triangles[i + 0]);
				triIndices[i + 1] = VertexIndex(ch.m_triangles[i + 1]);
				triIndices[i + 2] = VertexIndex(ch.m_triangles[i + 2]);
			}
			out->getPolygons().add(triIndices);

			out->updateFromEdits();
			outArray[j] = out;
		}
		return outArray;
	}
	return {};
}

auto ConvexHullComputer::begin(std::shared_ptr<ReturnType> ptr_, std::shared_ptr<MeshMod::Mesh> const& in_, ConvexHullParameters const& parameters_) -> bool
{
	using namespace MeshMod;

	auto const& vertices = in_->getVertices();
	auto const& polygons = in_->getPolygons();

	auto const& positions = vertices.positions();
	std::vector<float> points;
	points.resize(size_t(vertices.getCount()) * 3);

	for (auto const& pos : positions)
	{
		size_t i = size_t(positions.distance(pos)) * 3;
		points[i + 0] = pos.x;
		points[i + 1] = pos.y;
		points[i + 2] = pos.z;
	}

	VertexIndexContainer vertexIndices;
	for (auto i = 0u; i < polygons.getCount(); ++i)
	{
		polygons.getVertexIndices(PolygonIndex(i), vertexIndices);
	}

	VHACD::IVHACD::Parameters params;
	params.m_concavity = parameters_.concavity;
	params.m_alpha = parameters_.alpha;
	params.m_beta = parameters_.beta;
	params.m_minVolumePerCH = parameters_.minVolumePerCH;
	params.m_resolution = parameters_.resolution;
	params.m_maxNumVerticesPerCH = parameters_.maxNumVerticesPerCH;
	params.m_planeDownsampling = parameters_.planeDownsampling;
	params.m_convexhullDownsampling = parameters_.convexhullDownsampling;
	params.m_pca = parameters_.pca;
	params.m_mode = parameters_.mode;
	params.m_convexhullApproximation = parameters_.convexhullApproximation;
	params.m_maxConvexHulls = parameters_.maxConvexHulls;
	params.m_projectHullVertices = parameters_.projectHullVertices;

	bool res = ptr_->first->Compute(points.data(),
		(unsigned int)points.size() / 3,
		(unsigned int*)vertexIndices.data(),
		(unsigned int)vertexIndices.size() / 3, params);

	return res;
}

}