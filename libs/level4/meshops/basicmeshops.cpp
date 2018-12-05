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
#include "core/core.h"
#include "meshops/basicmeshops.h"

#include "geometry/aabb.h"
#include "core/exception.h"
#include "meshmod/scene.h"
#include "meshmod/sceneobject.h"
#include "meshmod/vertices.h"
#include "meshmod/halfedges.h"
#include "meshmod/polygons.h"
#include "meshmod/mesh.h"
#include "meshmod/vertexdata/vertexdata.h"
#include "meshmod/vertexdata/uvvertex.h"
#include "meshmod/vertexdata/positionvertex.h"
#include "meshmod/vertexdata/normalvertex.h"
#include "meshmod/vertexdata/pointrepvertex.h"
#include "meshmod/halfedgedata/halfedgecontainers.h"
#include "meshmod/polygonsdata/polygoncontainers.h"
#include "meshops/meshsorter.h"

#include "fmt/format.h"

#include <cassert>
#include <algorithm>
#include <set>

namespace MeshOps {

DECLARE_EXCEPTION(BasicMeshOp, "Cannot process mesh with this op");

/**
Compute and stores triangle plane equations.
Add a face element with each faces plane equation, will work for polygons but non-planar 
polygons may produce incorrect plane equations.

If any lines or point faces are in the mesh the planeequation for that face will be the default
*/
auto BasicMeshOps::computeFacePlaneEquations(std::shared_ptr<MeshMod::Mesh> const& mesh, bool replaceExisting, bool zeroBad,
											 bool fixBad) -> void
{
	using namespace MeshMod;
	using namespace Math;

	mesh->updateFromEdits();

	auto& vertices = mesh->getVertices();
	auto& polygons = mesh->getPolygons();

	auto const& vertCon = vertices.getVerticesContainer();
	auto& polyCon = polygons.getPolygonsContainer();

	// create plane equation face data if nessecary
	if(polyCon.getElement<PolygonData::PlaneEquations>() &&
	   replaceExisting == false)
	{
		return; // we already have normal and don't want to overwrite existing so just return
	}

	auto& planeEquations = polygons.getOrAddAttribute<PolygonData::PlaneEquations>();
	auto const& positions = mesh->getVertices().positions();

	auto peIt = planeEquations.begin();
	while(peIt != planeEquations.end())
	{
		auto const polyonIndex = planeEquations.distance(peIt);

		VertexIndexContainer faceVert;
		polygons.getVertexIndices(polyonIndex, faceVert);

		// only makes sense for triangles or polygons (TODO should use newell method for polygons)
		if(faceVert.size() >= 3)
		{
			vec3 const a(positions[faceVert[0]].getVec3());
			vec3 const b(positions[faceVert[1]].getVec3());
			vec3 const c(positions[faceVert[2]].getVec3());

			vec3 const dba = b - a;
			vec3 const dbc = b - c;

			vec3 cross = Math::cross(dba, dbc);
			vec3 nc = Normalise(cross);

			if(IsFinite(nc))
			{
				// d = distance along the plane normal to a vertex (all are on the plane if planar)
				float d = dot(nc, b);
				peIt->planeEq = Math::Plane(nc, -d);
			} else
			{
				if(zeroBad)
				{
					peIt->planeEq = Math::Plane(0, 0, 0, 0);
				} else if(fixBad)
				{
					// polygon has degenerated to a line or point
					// therefore fake a normal (any will do) and project a vertex
					// it will give a plane going throught the line or point (best we can do)
					nc = vec3(1, 0, 0); // any normal would do, randome would be better tbh...
					float d = dot(nc, b);
					peIt->planeEq = Math::Plane(nc, -d);
				} else
				{
					CoreThrowException(BasicMeshOp, "Bad plane equation");
				}
			}
		} else
		{
			if(zeroBad)
			{
				peIt->planeEq = Math::Plane(0, 0, 0, 0);
			} else if(fixBad && faceVert.size() >= 1)
			{
				vec3 a(positions[faceVert[0]].getVec3());
				// line or point cannot have a plane equation
				// therefore fake a normal (any will do) and project a vertex
				// it will give a plane going throught the line or point (best we can do)
				vec3 nc(1, 0, 0); // any normal would do, randome would be better tbh...
				float d = dot(nc, a);
				peIt->planeEq = Math::Plane(nc, -d);
			}
		}

		++peIt;
	}

	mesh->updateEditState(Mesh::TopologyAttributesEdits);
}


/**
Computes and returns the axis aligned box from the meshes position
*/
auto BasicMeshOps::computeAABB(std::shared_ptr<MeshMod::Mesh const> const& mesh, Geometry::AABB& aabb) -> void
{
	using namespace MeshMod;

	auto const& positions = mesh->getVertices().positions();

	aabb = Geometry::AABB(); // reset aabb
	for(auto const& pos : positions)
	{
		aabb.expandBy(pos.getVec3());
	}
}

template<size_t n>
auto BasicMeshOps::ngulate(std::shared_ptr<MeshMod::Mesh> const& mesh) -> void
{
	using namespace MeshMod;

	mesh->updateFromEdits();

	auto& polygons = mesh->getPolygons();
	auto& halfEdges = mesh->getHalfEdges();

	auto const origFaceCount = polygons.getCount();

	for(auto polygonIndex = 0u; polygonIndex < origFaceCount; polygonIndex++)
	{
		HalfEdgeIndexContainer faceHalfEdges;

		polygons.getHalfEdgeIndices(PolygonIndex(polygonIndex), faceHalfEdges);
		if(faceHalfEdges.size() > n)
		{

			// first 2 edges from face edges
			HalfEdgeIndex hei0 = faceHalfEdges[0];
			HalfEdgeIndex hei1 = faceHalfEdges[1];
			HalfEdgeIndex hei2 = InvalidHalfEdgeIndex;

			unsigned int origEdgeNum = 2;

			for(unsigned int i = 0; i < faceHalfEdges.size() - 2; i++)
			{
				PolygonIndex triIndex;

				if(i != 0)
				{
					// clone original
					triIndex = polygons.getPolygonsContainer().cloneElement(PolygonIndex(polygonIndex));
				} else
				{
					// reuse original poly if first triangle
					triIndex = PolygonIndex(polygonIndex);
				}
				PolygonData::Polygon& triFace = polygons.polygon(triIndex);

				// start edge
				if(i > 0)
				{
					HalfEdgeData::HalfEdge& he0 = halfEdges.halfEdge(hei0);
					// insert a new half edge (reverse the last tri last edge)
					hei0 = halfEdges.add(he0.endVertexIndex, he0.startVertexIndex, triIndex);

					// check
					HalfEdgeData::HalfEdge& he1 = halfEdges.halfEdge(hei1);
					assert(halfEdges.halfEdge(hei0).endVertexIndex == he1.startVertexIndex);
				} else
				{
					// just check
					HalfEdgeData::HalfEdge& he0 = halfEdges.halfEdge(hei0);
					HalfEdgeData::HalfEdge& he1 = halfEdges.halfEdge(hei1);
					assert(he0.endVertexIndex == he1.startVertexIndex);
				}

				// end edge (do we need a new one or can we use the original polys edge)
				if(i != faceHalfEdges.size() - 3)
				{
					HalfEdgeData::HalfEdge& he0 = halfEdges.halfEdge(hei0);
					HalfEdgeData::HalfEdge& he1 = halfEdges.halfEdge(hei1);

					// insert a new edge
					hei2 = halfEdges.add(he1.endVertexIndex,
										 he0.startVertexIndex,
										 triIndex);

				} else
				{
					hei2 = faceHalfEdges[faceHalfEdges.size() - 1];

					HalfEdgeData::HalfEdge& he1 = halfEdges.halfEdge(hei1);
					HalfEdgeData::HalfEdge& he2 = halfEdges.halfEdge(hei2);
					assert(he1.endVertexIndex == he2.startVertexIndex);
				}

				triFace.anyHalfEdge = hei0; // repoint first edge
				// make half edge point to correct triangle
				halfEdges.halfEdge(hei0).polygonIndex = triIndex;
				halfEdges.halfEdge(hei1).polygonIndex = triIndex;
				halfEdges.halfEdge(hei2).polygonIndex = triIndex;

				// connect edges (complete the cycle list)
				halfEdges.halfEdge(hei0).next = hei1;
				halfEdges.halfEdge(hei1).next = hei2;
				halfEdges.halfEdge(hei2).next = hei0;

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
auto BasicMeshOps::triangulate(std::shared_ptr<MeshMod::Mesh> const& mesh) -> void
{
	ngulate<3>(mesh);
}

// post will have no n-gons, points, lines, triangles and quads will remain
// TODO restitch planar triangle pairs to quads
// TODO non-planar quads decompose to triangles
auto BasicMeshOps::quadOrTriangulate(std::shared_ptr<MeshMod::Mesh> const& mesh) -> void
{
	ngulate<4>(mesh);
}

/**
Computes and store per vertex normals.
If the object already has vertex normals they will be kept if replaceExising == false.
Simple average lighting normal.
*/
auto BasicMeshOps::computeVertexNormals(std::shared_ptr<MeshMod::Mesh> const& mesh, bool replaceExisting) -> void
{
	using namespace MeshMod;

	mesh->updateFromEdits();

	auto const& polygons = mesh->getPolygons();
	auto& vertices = mesh->getVertices();

	if(vertices.hasAttribute<VertexData::Normals>() &&
	   replaceExisting == false)
	{
		return; // we already have normal and don't want to overwrite existing so just return
	}

	bool backupMaintainPointReps = mesh->isMaintainPointReps();
	mesh->maintainPointReps(true);

	// clear normals
	auto normals = vertices.getAttribute <VertexData::Normals>();
	vertices.visitAll(
			[&normals](VertexIndex const vertexIndex_)
			{
				normals[vertexIndex_] = {0,0,0};
			});

	computeFacePlaneEquations(mesh, replaceExisting);

	auto const& planeEqs = polygons.getAttribute<PolygonData::PlaneEquations>();
	// add the poly equation to every vertex in every face
	polygons.visitValid(
			[&polygons, &vertices, &normals, &planeEqs](PolygonIndex const polygonIndex_)
			{
				VertexIndexContainer vertList;
				polygons.getVertexIndices(polygonIndex_, vertList);

				for(auto const& vert : vertList)
				{
					VertexIndexContainer simVertList;
					vertices.getSimilarVertexIndices(vert, simVertList);
					for(auto const& simVert : simVertList)
					{

						normals[simVert].x += planeEqs[polygonIndex_].planeEq.a;
						normals[simVert].y += planeEqs[polygonIndex_].planeEq.b;
						normals[simVert].z += planeEqs[polygonIndex_].planeEq.c;
					}
				}
			});

	// now normalise all normals
	vertices.visitAll(
			[&vertices, &normals](VertexIndex const vertexIndex_)
			{
				if(!vertices.isValid(vertexIndex_))
				{
					// invalid vertex so set normal to NAN
					normals[vertexIndex_] = {s_floatMarker, s_floatMarker, s_floatMarker};
				} else
				{
					//valid vertex (not deleted)
					float norm = Math::Length(normals[vertexIndex_].getVec3());
					norm = sqrtf(norm);

					normals[vertexIndex_].x = normals[vertexIndex_].x / norm;
					normals[vertexIndex_].y = normals[vertexIndex_].y / norm;
					normals[vertexIndex_].z = normals[vertexIndex_].z / norm;
				}
			});

	mesh->maintainPointReps(backupMaintainPointReps);
	mesh->updateEditState(Mesh::VertexAttributeEdits);
}

/**
Computes and store per vertex normals.
Based on mesh libs version with fixing and handling of slivers
*/
auto BasicMeshOps::computeVertexNormalsEx(std::shared_ptr<MeshMod::Mesh> const& mesh, bool replaceExisting, bool zeroBad,
										  bool fixBad) -> void
{
	if(zeroBad && fixBad)
	{
		CoreThrowException(BasicMeshOp, "Cannot both zero and fix bad normals");
	}

	using namespace MeshMod;
	using namespace Math;


	auto& vertices = mesh->getVertices();
	auto const& halfEdges = mesh->getHalfEdges();
	auto const& polygons = mesh->getPolygons();

	if(vertices.hasAttribute<VertexData::Normals>() && replaceExisting == false)
	{
		return; // we already have normal and don't want to overwrite existing so just return
	}

	bool backupMaintainPointReps = mesh->isMaintainPointReps();
	mesh->maintainPointReps(true);
	mesh->updateFromEdits();

	// clear normals
	auto& normals = vertices.getOrAddAttribute <VertexData::Normals>();
	vertices.visitAll(
			[&normals](VertexIndex const vertexIndex_)
			{
				normals[vertexIndex_] = {0,0,0};
			});

	computeFacePlaneEquations(mesh, replaceExisting, zeroBad, fixBad);
	auto const& planeEqs = polygons.getAttribute<PolygonData::PlaneEquations>();

	auto const& hes = halfEdges.halfEdges();

	HalfEdgeIndexContainer vertexHalfEdges;
	vertexHalfEdges.reserve(10);

	// each normal try to generate a fair normal (TODO smoothing group polygons)
	// fix or zero bad normals, fix try using simplier plane equation generator
	// TODO if this fix fails try mesh libs average of existing normals?

	for(auto& normal : normals)
	{
		// get the vertex and edges connected to this vertex
		auto const vertexIndex = normals.distance(normal);
		if (vertices.isValid(vertexIndex) == false) continue;

		vertexHalfEdges.clear();
		vertices.getVertexHalfEdges(vertexIndex, vertexHalfEdges);

		Math::vec3 vertexNormal(0, 0, 0);

		// TODO smoothing groups
		for(auto heIt = vertexHalfEdges.cbegin(); heIt != vertexHalfEdges.cend(); ++heIt)
		{
			const HalfEdgeData::HalfEdge& he = hes.at(*heIt);
			const PolygonData::PlaneEquation& pe = planeEqs.at(he.polygonIndex);
			Math::vec3 localNormal = pe.planeEq.normal();

			// get opposing indices
			const HalfEdgeIndex i1 = (HalfEdgeIndex) ((size_t(*heIt) + 1) % vertexHalfEdges.size());
			const HalfEdgeIndex i2 = (HalfEdgeIndex) ((size_t(*heIt) + 2) % vertexHalfEdges.size());
			const HalfEdgeData::HalfEdge& he1 = hes.at(i1);
			const HalfEdgeData::HalfEdge& he2 = hes.at(i2);
			const PolygonData::PlaneEquation& pe1 = planeEqs.at(he1.polygonIndex);
			const PolygonData::PlaneEquation& pe2 = planeEqs.at(he2.polygonIndex);

			Math::vec3 e1 = Normalise(pe1.planeEq.normal());
			Math::vec3 e2 = Normalise(pe2.planeEq.normal());

			// compute the angle and only accumulate at non-sliver angles
			const float angle = std::acos(-dot(e1, e2) / (Math::Length(e1) * Math::Length(e2)));
			if(std::isfinite(angle))
			{
				// accumulate the normal
				vertexNormal += localNormal * angle;
			}
		}

		// normalise it
		vertexNormal = Math::Normalise(vertexNormal);

		// check it's finite
		if(!IsFinite(vertexNormal))
		{
			// either fix it using simplier plane equation average, zero or ignore
			if(zeroBad)
			{
				vertexNormal = Math::vec3(0, 0, 0);
			} else if(fixBad)
			{
				HalfEdgeIndexContainer::const_iterator edgeIt = vertexHalfEdges.begin();
				vertexNormal = Math::vec3(0, 0, 0);
				while(edgeIt != vertexHalfEdges.end())
				{
					const HalfEdgeData::HalfEdge& he = hes.at(*edgeIt);
					const PolygonData::PlaneEquation& pe = planeEqs.at(he.polygonIndex);
					vertexNormal += pe.planeEq.normal();
					++edgeIt;
				}
				// normalise it
				vertexNormal = Math::Normalise(vertexNormal);
			}
		}

		// remove denormals here at source, to ensure none enter the pipe at source
		if(!std::isnormal(normal.x)) normal.x = float(0);
		if(!std::isnormal(normal.y)) normal.y = float(0);
		if(!std::isnormal(normal.z)) normal.z = float(0);

		normal.x += vertexNormal.x;
		normal.y += vertexNormal.y;
		normal.z += vertexNormal.z;
	}

	mesh->maintainPointReps(backupMaintainPointReps);
	mesh->updateEditState(Mesh::VertexAttributeEdits);

}

auto BasicMeshOps::transform(std::shared_ptr<MeshMod::Mesh> const& mesh, Math::mat4x4 const& transform) -> void
{
	auto& vertices = mesh->getVertices();
	for(auto& vertex : vertices.positions())
	{
		Math::vec3 pos = Math::TransformAndProject(transform, vertex.getVec3());
		vertex.x = pos.x;
		vertex.y = pos.y;
		vertex.z = pos.z;
	}
	mesh->updateEditState(MeshMod::Mesh::PositionEdits);
}

auto BasicMeshOps::isTriangleMesh(std::shared_ptr<MeshMod::Mesh const> const& mesh_) -> bool
{
	using namespace MeshMod;
	using namespace Math;

	Polygons const& polygons = mesh_->getPolygons();

	bool isTriangle = true;

	polygons.visitAll(
			[&polygons, &isTriangle](PolygonIndex const polygonIndex_)
			{
				if(polygons.getVertexCount(polygonIndex_) != 3)
					isTriangle = false;
			});

	return isTriangle;

}

auto BasicMeshOps::tesselate4(std::shared_ptr<MeshMod::Mesh const> const& mesh_) -> std::unique_ptr<MeshMod::Mesh>
{
	using namespace MeshMod;
	using namespace Math;

	std::string const newName = fmt::format("{}_t", mesh_->getName());
	auto tessMesh = std::make_unique<Mesh>(newName);

	VertexIndexContainer vertexIndexContainer;
	vertexIndexContainer.reserve(3);

	Polygons const& polygons = mesh_->getPolygons();
	Vertices const& vertices = mesh_->getVertices();
	polygons.visitAll(
			[&polygons, &vertices, &vertexIndexContainer, &tessMesh]
					(PolygonIndex const polygonIndex_)
			{
				assert(polygons.getVertexCount(polygonIndex_) == 3);
				vertexIndexContainer.clear();
				polygons.getVertexIndices(polygonIndex_, vertexIndexContainer);

				Math::vec3 verts[6];
				verts[0] = vertices.position(vertexIndexContainer[0]).getVec3();
				verts[5] = vertices.position(vertexIndexContainer[1]).getVec3();
				verts[2] = vertices.position(vertexIndexContainer[2]).getVec3();

				verts[1] = (verts[0] + verts[2]) * 0.5f;
				verts[3] = (verts[0] + verts[5]) * 0.5f;
				verts[4] = (verts[2] + verts[5]) * 0.5f;

				VertexIndex vi[6];

				for(auto j = 0u; j < 6; ++j)
				{
					vi[j] = tessMesh->getVertices().add(verts[j].x, verts[j].y, verts[j].z);
				}

				tessMesh->getPolygons().addPolygon({vi[0], vi[3], vi[1]});
				tessMesh->getPolygons().addPolygon({vi[3], vi[4], vi[1]});
				tessMesh->getPolygons().addPolygon({vi[4], vi[2], vi[1]});
				tessMesh->getPolygons().addPolygon({vi[3], vi[5], vi[4]});
			});

	tessMesh->getVertices().removeAllSimilarPositions();
	tessMesh->updateFromEdits();

	return tessMesh;
}

auto BasicMeshOps::spherize(std::shared_ptr<MeshMod::Mesh> const& mesh_, float t_) -> void
{
	using namespace MeshMod;
	using namespace Math;

	Vertices& vertices = mesh_->getVertices();

	Math::vec3 maxLen = {0, 0, 0};
	vertices.visitAll(
			[&vertices, &maxLen](VertexIndex vertexIndex_)
			{
				Math::vec3 p = vertices.position(vertexIndex_).getVec3();
				float const len = Math::length(p);
				maxLen = Math::max(maxLen, len);
			});

	vertices.visitAll(
			[&vertices, &t_, maxLen](VertexIndex vertexIndex_)
			{
				Math::vec3 p0 = vertices.position(vertexIndex_).getVec3();
				float const len = Math::length(p0);
				Math::vec3 p1 = Math::normalize(p0) * maxLen;
				Math::vec3 p = p0 + t_ * (p1 - p0);
				vertices.position(vertexIndex_).x = p.x;
				vertices.position(vertexIndex_).y = p.y;
				vertices.position(vertexIndex_).z = p.z;
			});

}


} // end namespace