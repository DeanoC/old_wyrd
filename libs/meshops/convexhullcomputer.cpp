#include "core/core.h"
#include "meshops.h"
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

auto ConvexHullComputer::generate( MeshMod::Mesh::Ptr const& in_, ConvexHullParameters const& parameters_) -> std::vector<MeshMod::Mesh::Ptr>
{
	auto ivhacd = std::shared_ptr<ReturnType>(new ReturnType(VHACD::CreateVHACD(), nullptr), ReturnDestroyer);

	if (begin(ivhacd, in_, parameters_) == false)
	{
		return {};
	}
	return getResults(ivhacd);
}

void ConvexHullComputer::generateInline( MeshMod::Mesh::Ptr& in_ )
{
	MeshOps::ConvexHullParameters defaults;
	auto out = generate(in_, defaults);
	assert(out.size() == 1);
	*(in_.get()) = *(out[0].get());
}

auto ConvexHullComputer::createAsync(	MeshMod::Mesh::Ptr const& in_, 
										ConvexHullParameters const& parameters_
										)->Ptr
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

auto ConvexHullComputer::isReady(Ptr ptr_) -> bool
{
	if (ptr_)
	{
		return ptr_->first->IsReady();
	}
	return false;
}

auto ConvexHullComputer::getResults(Ptr ptr_)->std::vector<MeshMod::Mesh::Ptr>
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
		std::vector<Mesh::Ptr> outArray(nConvexHulls);
		for (auto j = 0u; j < nConvexHulls; ++j)
		{
			VHACD::IVHACD::ConvexHull ch;
			ptr_->first->GetConvexHull(j, ch);

			Mesh::Ptr out = std::make_shared<Mesh>("ConvexHull_" + std::to_string(j));
			for (auto i = 0u; i < ch.m_nPoints * 3; i += 3)
			{
				float x = (float)ch.m_points[i + 0];
				float y = (float)ch.m_points[i + 1];
				float z = (float)ch.m_points[i + 2];
				out->getVertices().add(x, y, z);
			}
			for (auto i = 0u; i < ch.m_nTriangles * 3; i += 3)
			{
				VertexIndexContainer triIndices = {
						ch.m_triangles[i + 0],
						ch.m_triangles[i + 1],
						ch.m_triangles[i + 2],
				};
				out->getPolygons().add(triIndices);
			}

			out->updateFromEdits();
			outArray[j] = out;
		}
		return outArray;
	}
	return {};
}

auto ConvexHullComputer::begin(Ptr ptr_, MeshMod::Mesh::Ptr const& in_, ConvexHullParameters const& parameters_) -> bool
{
	using namespace MeshMod;

	auto const& vertices = in_->getVertices();
	auto const& polygons = in_->getPolygons();

	auto const& positions = vertices.positions();
	std::vector<float> points;
	points.resize(vertices.getCount() * 3);

	for (auto const& pos : positions)
	{
		size_t i = positions.distance<size_t>(pos) * 3;
		points[i + 0] = pos.x;
		points[i + 1] = pos.y;
		points[i + 2] = pos.z;
	}

	VertexIndexContainer vertexIndices;
	for (auto i = 0u; i < polygons.getCount(); ++i)
	{
		polygons.getVertexIndices(i, vertexIndices);
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
		vertexIndices.data(),
		(unsigned int)vertexIndices.size() / 3, params);

	return res;
}

}