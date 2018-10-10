/** \file meshsorter.cpp
   Geometry Optimiser Mesh sorting operations.
   (c) 2012 Deano Calver
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "meshops.h"
#include <cassert>
#include <algorithm>
#include <set>

#include <meshmod/vertices.h>
#include <meshmod/halfedges.h>
#include <meshmod/polygons.h>
#include <meshmod/mesh.h>
#include <meshmod/vertexdata/positionvertex.h>
#include <meshmod/vertexdata/normalvertex.h>
#include <meshmod/vertexdata/pointrepvertex.h>
#include <meshmod/halfedgedata/halfedgecontainers.h>
#include <geometry/aabb.h>
#include <core/exception.h>
#include <meshops/meshsorter.h>
#include "basicmeshops.h"

namespace MeshOps {

DECLARE_EXCEPTION( BasicMeshOp, "Cannot process mesh with this op" );

BasicMeshOps::BasicMeshOps( MeshMod::MeshPtr& _mesh ) :
		mesh( _mesh )
{
}

/**
Compute and stores triangle plane equations.
Add a face element with each faces plane equation, will work for polygons but non-planar 
polygons may produce incorrect plane equations.

If any lines or point faces are in the mesh the planeequation for that face will be the default
*/
void BasicMeshOps::computeFacePlaneEquations( bool replaceExisting, bool zeroBad, bool fixBad )
{
	using namespace MeshMod;
	using namespace Math;

	mesh->updateFromEdits();

	auto& vertices = mesh->getVertices();
	auto& polygons = mesh->getPolygons();

	auto const& vertCon = vertices.getVerticesContainer();
	auto& polyCon = polygons.getPolygonsContainer();

	// create plane equation face data if nessecary
	auto peEle = polyCon.getElements<PolygonData::PlaneEquations>();
	if(peEle && replaceExisting == false)
	{
		return; // we already have normal and don't want to overwrite existing so just return
	} else
	{
		peEle = polyCon.getOrAddElements<PolygonData::PlaneEquations>();
	}

	auto positions = mesh->getVertices().positions();

	PolygonData::PlaneEquations::iterator faceIt = peEle->elements.begin();
	while(faceIt != peEle->elements.end())
	{
		const PolygonIndex faceNum = (PolygonIndex) std::distance( peEle->elements.begin(), faceIt );

		VertexIndexContainer faceVert;
		polygons.getVertexIndices( faceNum, faceVert );

		// only makes sense for triangles or polygons (TODO should use newell method for polygons)
		if(faceVert.size() >= 3)
		{
			Vector3 a( positions[faceVert[0]].getVector3());
			Vector3 b( positions[faceVert[1]].getVector3());
			Vector3 c( positions[faceVert[2]].getVector3());

			Vector3 dba = b - a;
			Vector3 dbc = b - c;

			Vector3 cross = Cross( dba, dbc );
			Vector3 nc = Normalise( cross );

			if(IsFinite( nc ))
			{
				// d = distance along the plane normal to a vertex (all are on the plane if planar)
				float d = Dot( nc, b );
				(*peEle)[faceNum].planeEq = Math::Plane( nc, -d );
			} else
			{
				if(zeroBad)
				{
					(*peEle)[faceNum].planeEq = Math::Plane( 0, 0, 0, 0 );
				} else if(fixBad)
				{
					// polygon has degenerated to a line or point
					// therefore fake a normal (any will do) and project a vertex
					// it will give a plane going throught the line or point (best we can do)
					nc = Vector3( 1, 0, 0 ); // any normal would do, randome would be better tbh...
					float d = Dot( nc, b );
					(*peEle)[faceNum].planeEq = Math::Plane( nc, -d );
				} else
				{
					CoreThrowException( BasicMeshOp, "Bad plane equation" );
				}
			}
		} else
		{
			if(zeroBad)
			{
				(*peEle)[faceNum].planeEq = Math::Plane( 0, 0, 0, 0 );
			} else if(fixBad && faceVert.size() >= 1)
			{
				Vector3 a( positions[faceVert[0]].getVector3());
				// line or point cannot have a plane equation
				// therefore fake a normal (any will do) and project a vertex
				// it will give a plane going throught the line or point (best we can do)
				Vector3 nc( 1, 0, 0 ); // any normal would do, randome would be better tbh...
				float d = Dot( nc, a );
				(*peEle)[faceNum].planeEq = Math::Plane( nc, -d );
			}
		}

		++faceIt;
	}

	mesh->updateEditState( Mesh::TopologyAttributesEdits );
}


/**
Computes and returns the axis aligned box from the meshes position
*/
void BasicMeshOps::computeAABB(MeshMod::Mesh const& mesh, Geometry::AABB& aabb )
{
	using namespace MeshMod;

	auto const& positions = mesh.getVertices().positions();

	aabb = Geometry::AABB(); // reset aabb
	for(auto const& pos : positions)
	{
		aabb.expandBy( pos.getVector3());
	}
}

template<size_t n>
void BasicMeshOps::ngulate()
{
	using namespace MeshMod;

	mesh->updateFromEdits();

	auto& polygons = mesh->getPolygons();
	auto& halfEdges = mesh->getHalfEdges();

	PolygonIndex const origFaceCount = polygons.getCount();

	for(PolygonIndex polygonIndex = 0; polygonIndex < origFaceCount; polygonIndex++)
	{
		HalfEdgeIndexContainer faceHalfEdges;

		polygons.getHalfEdgeIndices( polygonIndex, faceHalfEdges );
		if(faceHalfEdges.size() > n)
		{

			// first 2 edges from face edges
			HalfEdgeIndex hei0 = faceHalfEdges[0];
			HalfEdgeIndex hei1 = faceHalfEdges[1];
			HalfEdgeIndex hei2 = MM_INVALID_INDEX;

			unsigned int origEdgeNum = 2;

			for(unsigned int i = 0; i < faceHalfEdges.size() - 2; i++)
			{
				PolygonIndex triIndex;

				if(i != 0)
				{
					triIndex = (PolygonIndex) polygons.getPolygonsContainer().cloneElement(polygonIndex ); // clone original
				} else
				{
					// reuse original poly if first triangle
					triIndex = (MeshMod::PolygonIndex) polygonIndex;
				}
				PolygonData::Polygon& triFace = polygons.polygon( triIndex );

				// start edge
				if(i > 0)
				{
					HalfEdgeData::HalfEdge& he0 = halfEdges.halfEdge( hei0 );
					// insert a new half edge (reverse the last tri last edge)
					hei0 = halfEdges.add( he0.endVertexIndex, he0.startVertexIndex, triIndex );

					// check
					HalfEdgeData::HalfEdge& he1 = halfEdges.halfEdge( hei1 );
					assert( halfEdges.halfEdge( hei0 ).endVertexIndex == he1.startVertexIndex );
				} else
				{
					// just check
					HalfEdgeData::HalfEdge& he0 = halfEdges.halfEdge( hei0 );
					HalfEdgeData::HalfEdge& he1 = halfEdges.halfEdge( hei1 );
					assert( he0.endVertexIndex == he1.startVertexIndex );
				}

				// end edge (do we need a new one or can we use the original polys edge)
				if(i != faceHalfEdges.size() - 3)
				{
					HalfEdgeData::HalfEdge& he0 = halfEdges.halfEdge( hei0 );
					HalfEdgeData::HalfEdge& he1 = halfEdges.halfEdge( hei1 );

					// insert a new edge
					hei2 = halfEdges.add( he1.endVertexIndex,
										  he0.startVertexIndex,
										  triIndex );

				} else
				{
					hei2 = faceHalfEdges[faceHalfEdges.size() - 1];

					HalfEdgeData::HalfEdge& he1 = halfEdges.halfEdge( hei1 );
					HalfEdgeData::HalfEdge& he2 = halfEdges.halfEdge( hei2 );
					assert( he1.endVertexIndex == he2.startVertexIndex );
				}

				triFace.anyHalfEdge = hei0; // repoint first edge
				// make half edge point to correct triangle
				halfEdges.halfEdge( hei0 ).polygonIndex = triIndex;
				halfEdges.halfEdge( hei1 ).polygonIndex = triIndex;
				halfEdges.halfEdge( hei2 ).polygonIndex = triIndex;

				// connect edges (complete the cycle list)
				halfEdges.halfEdge( hei0 ).next = hei1;
				halfEdges.halfEdge( hei1 ).next = hei2;
				halfEdges.halfEdge( hei2 ).next = hei0;

				// update edge indices (next edge)
				hei0 = hei2;
				hei1 = faceHalfEdges[origEdgeNum++];
			}
		}
	}

	// this is an incremental change so doesn't need an edit state change
	//mesh->updateEditState(Mesh::TopologyEdits);
}

/**
Triangulates all faces into triangles. points and lines will be untouched
All faces must be simple convex polygons (no complex polygons yet), post this call all faces will have <= 3 vertices
*/
void BasicMeshOps::triangulate()
{
	ngulate<3>();
}

// post will have no n-gons, points, lines, triangles and quads will remain
// TODO restitch planar triangle pairs to quads
// TODO non-planar quads decompose to triangles
void BasicMeshOps::quadOrTriangulate()
{
	ngulate<4>();
}

/**
Computes and store per vertex normals.
If the object already has vertex normals they will be kept if replaceExising == false.
Simple average lighting normal.
*/
void BasicMeshOps::computeVertexNormals( bool replaceExisting )
{
	using namespace MeshMod;

	mesh->updateFromEdits();

	auto const& polygons = mesh->getPolygons();
	auto& vertices = mesh->getVertices();

	auto& vertCon = vertices.getVerticesContainer();
	auto& polyCon = polygons.getPolygonsContainer();

	auto normEle = vertCon.getElements<VertexData::Normals>();
	if(normEle && replaceExisting == false)
	{
		return; // we already have normal and don't want to overwrite existing so just return
	} else
	{
		normEle = vertCon.getOrAddElements<VertexData::Normals>();
	}

	// clear normals
	std::fill( normEle->begin(), normEle->end(), VertexData::Normal( 0, 0, 0 ));

	computeFacePlaneEquations( replaceExisting );

	auto planeEle = polyCon.getElements<PolygonData::PlaneEquations>();
	assert( planeEle );

	// add the poly equation to every vertex in every face
	for(auto planeEqIt = planeEle->cbegin(); planeEqIt != planeEle->cend(); ++planeEqIt)
	{
		PolygonIndex const polygonIndex = planeEle->distance<PolygonIndex>( planeEqIt );

		VertexIndexContainer vertList;
		polygons.getVertexIndices( polygonIndex, vertList );

		for(auto const& vert : vertList)
		{
			VertexIndexContainer simVertList;
			vertices.getSimilarVertexIndices( vert, simVertList );
			for(auto const& simVert : simVertList)
			{
				(*normEle)[simVert].x += (*planeEqIt).planeEq.a;
				(*normEle)[simVert].y += (*planeEqIt).planeEq.b;
				(*normEle)[simVert].z += (*planeEqIt).planeEq.c;
			}
		}
	}

	// now normalise all normals
	for(auto normIt = normEle->begin(); normIt != normEle->cend(); ++normIt)
	{
		VertexIndex const vertexIndex = normEle->distance<VertexIndex>( normIt );
		if(!vertices.isValid( vertexIndex ))
		{
			// invalid vertex so set normal to NAN
			(*normIt).x = s_floatMarker;
			(*normIt).y = s_floatMarker;
			(*normIt).z = s_floatMarker;
		} else
		{
			//valid vertex (not deleted)
			float norm = ((*normIt).x * (*normIt).x) +
						 ((*normIt).y * (*normIt).y) +
						 ((*normIt).z * (*normIt).z);
			norm = sqrtf( norm );

			(*normIt).x = -(*normIt).x / norm;
			(*normIt).y = -(*normIt).y / norm;
			(*normIt).z = -(*normIt).z / norm;
		}
	}

	mesh->updateEditState( Mesh::VertexAttributeEdits );
}

/**
Computes and store per vertex normals.
Based on mesh libs version with fixing and handling of slivers
*/
void BasicMeshOps::computeVertexNormalsEx( bool replaceExisting, bool zeroBad, bool fixBad )
{
	if(zeroBad && fixBad)
	{
		CoreThrowException( BasicMeshOp, "Cannot both zero and fix bad normals" );
	}

	using namespace MeshMod;
	using namespace Math;

	mesh->updateFromEdits();

	auto& vertices = mesh->getVertices();
	auto const& halfEdges = mesh->getHalfEdges();
	auto const& polygons = mesh->getPolygons();

	VerticesElementsContainer& vertCon = vertices.getVerticesContainer();

	auto normEle = vertCon.getElements<VertexData::Normals>();
	if(normEle && replaceExisting == false)
	{
		return; // we already have normal and don't want to overwrite existing so just return
	} else
	{
		normEle = vertCon.getOrAddElements<VertexData::Normals>();
	}

	// clear normals
	std::fill( normEle->elements.begin(), normEle->elements.end(), VertexData::Normal( 0, 0, 0 ));

	computeFacePlaneEquations( replaceExisting, zeroBad, fixBad );

	auto const& hes = halfEdges.halfEdges();
	auto const& planeEqs = polygons.getAttributes<PolygonData::PlaneEquations>();
	auto& normals = vertices.getOrAddAttributes<VertexData::Normals>();

	HalfEdgeIndexContainer vertexHalfEdges;
	vertexHalfEdges.reserve( 10 );

	// each normal try to generate a fair normal (TODO smoothing group polygons)
	// fix or zero bad normals, fix try using simplier plane equation generator
	// TODO if this fix fails try mesh libs average of existing normals?

	for(auto& normal : normals)
	{
		// get the vertex and edges connected to this vertex
		auto const vertexIndex = normals.distance<VertexIndex>( normal );

		vertexHalfEdges.clear();
		vertices.getVertexHalfEdges( vertexIndex, vertexHalfEdges );

		Math::Vector3 vertexNormal( 0, 0, 0 );

		// TODO smoothing groups
		for(auto heIt = vertexHalfEdges.cbegin(); heIt != vertexHalfEdges.cend(); ++heIt)
		{
			const HalfEdgeData::HalfEdge& he = hes.get( *heIt );
			const PolygonData::PlaneEquation& pe = planeEqs.get( he.polygonIndex );
			Math::Vector3 localNormal = pe.planeEq.normal();

			// get opposing indices
			const HalfEdgeIndex i1 = ((*heIt) + 1) % (HalfEdgeIndex) vertexHalfEdges.size();
			const HalfEdgeIndex i2 = ((*heIt) + 2) % (HalfEdgeIndex) vertexHalfEdges.size();
			const HalfEdgeData::HalfEdge& he1 = hes.get( i1 );
			const HalfEdgeData::HalfEdge& he2 = hes.get( i2 );
			const PolygonData::PlaneEquation& pe1 = planeEqs.get( he1.polygonIndex );
			const PolygonData::PlaneEquation& pe2 = planeEqs.get( he2.polygonIndex );

			Math::Vector3 e1 = Normalise(pe1.planeEq.normal());
			Math::Vector3 e2 = Normalise(pe2.planeEq.normal());

			// compute the angle and only accumulate at non-sliver angles
			const float angle = std::acos( -Dot( e1, e2 ) / (Math::Length( e1 ) * Math::Length( e2 )));
			if(std::isfinite( angle ))
			{
				// accumulate the normal
				vertexNormal += localNormal * angle;
			}
		}

		// normalise it
		vertexNormal = Math::Normalise( vertexNormal );

		// check it's finite
		if(!IsFinite( vertexNormal ))
		{
			// either fix it using simplier plane equation average, zero or ignore
			if(zeroBad)
			{
				vertexNormal = Math::Vector3( 0, 0, 0 );
			} else if(fixBad)
			{
				HalfEdgeIndexContainer::const_iterator edgeIt = vertexHalfEdges.begin();
				vertexNormal = Math::Vector3( 0, 0, 0 );
				while(edgeIt != vertexHalfEdges.end())
				{
					const HalfEdgeData::HalfEdge& he = hes.get( *edgeIt );
					const PolygonData::PlaneEquation& pe = planeEqs.get( he.polygonIndex );
					vertexNormal += pe.planeEq.normal();
					++edgeIt;
				}
				// normalise it
				vertexNormal = Math::Normalise( vertexNormal );
			}
		}

		// remove denormals here at source, to ensure none enter the pipe at source
		if(!std::isnormal( normal.x )) normal.x = float( 0 );
		if(!std::isnormal( normal.y )) normal.y = float( 0 );
		if(!std::isnormal( normal.z )) normal.z = float( 0 );

		normal.x += vertexNormal.x;
		normal.y += vertexNormal.y;
		normal.z += vertexNormal.z;
	}
	mesh->updateEditState( Mesh::VertexAttributeEdits );

}

void BasicMeshOps::transform( Math::Matrix4x4 const& transform )
{
	auto& vertices = mesh->getVertices();
	for(auto& vertex : vertices.positions())
	{
		Math::Vector3 pos = Math::TransformAndProject( vertex.getVector3(), transform );
		vertex.x = pos.x;
		vertex.y = pos.y;
		vertex.z = pos.z;
	}
	mesh->updateEditState( MeshMod::Mesh::PositionEdits );
}

} // end namespace