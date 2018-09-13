//
// Created by Computer on 01/08/2018.
//

#include <meshops/VHACD_Lib/public/VHACD.h>
#include <functional>
#include "meshops.h"
#include "convexhullcomputer.h"

namespace MeshOps {
template<typename T>
using deleted_unique_ptr = std::unique_ptr<T, std::function<void( T * )>>;

MeshMod::MeshPtr ConvexHullComputer::generate( MeshMod::MeshPtr const& in )
{
	using namespace MeshMod;

	deleted_unique_ptr<VHACD::IVHACD> ivhacd(
			VHACD::CreateVHACD(),
			[]( VHACD::IVHACD *b )
			{
				b->Release();
			} );
	VHACD::IVHACD::Parameters params;
	params.Init();
	params.m_maxConvexHulls = 1;

	auto const& vertices = in->getVertices();
	auto const& polygons = in->getPolygons();

	auto const& positions = vertices.positions();
	std::vector<float> points;
	points.resize( vertices.getCount() * 3 );

	for(auto const& pos : positions)
	{
		size_t i = positions.distance<size_t>( pos ) * 3;
		points[i + 0] = pos.x;
		points[i + 1] = pos.y;
		points[i + 2] = pos.z;
	}


	VertexIndexContainer vertexIndices;
	for(auto i = 0u; i < polygons.getCount(); ++i)
	{
		polygons.getVertexIndices( i, vertexIndices );
	}

	bool res = ivhacd->Compute( points.data(),
								(unsigned int) points.size() / 3,
								vertexIndices.data(),
								(unsigned int) vertexIndices.size() / 3, params );
	if(!res) return MeshPtr();

	unsigned int nConvexHulls = ivhacd->GetNConvexHulls();
	if(nConvexHulls == 0 || nConvexHulls > 1)
		return MeshPtr();

	VHACD::IVHACD::ConvexHull ch;
	ivhacd->GetConvexHull( 0, ch );

	MeshPtr out = std::make_shared<Mesh>( in->getName() + "_ConvexHull" );
	for(auto i = 0u; i < ch.m_nPoints * 3; i += 3)
	{
		float x = (float) ch.m_points[i + 0];
		float y = (float) ch.m_points[i + 1];
		float z = (float) ch.m_points[i + 2];
		out->getVertices().add( x, y, z );
	}
	for(auto i = 0u; i < ch.m_nTriangles * 3; i += 3)
	{
		VertexIndexContainer triIndices = { 
				ch.m_triangles[i + 0],
				ch.m_triangles[i + 1],
				ch.m_triangles[i + 2],
		};
		out->getPolygons().add( triIndices );
	}

	out->updateFromEdits();
	return out;
}

void ConvexHullComputer::generateInline( MeshMod::MeshPtr& in )
{
	MeshMod::MeshPtr out = generate(in);
	if(out)
	{
		*(in.get()) = *(out.get());
	}
}

}