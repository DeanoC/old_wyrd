//
// Created by Computer on 31/07/2018.
//

#include <algorithm>
#include "vertices.h"
#include "halfedges.h"
#include "polygons.h"
#include "mesh.h"

namespace MeshMod {

Polygons::Polygons(Mesh& owner_) :
	owner(owner_)
{
}


Polygons::~Polygons()
{
}

/**
Adds a polygon to the mesh.
Adds a poly, creates edge/half edge data structures
@param indices - vertex indices the polygon is composed off
@param baseVertex - all vertex indices are based off this number
@return index of new polyMeshModn
*/
PolygonIndex Polygons::add(VertexIndexContainer const &indices, VertexIndex const baseVertex)
{
	// should always be convex simple polygons!!
	auto const& vertices = owner.getVertices();
	auto& halfEdges = owner.getHalfEdges();

	PolygonData::Polygon t0;
	// index of this polygon first edge
	HalfEdgeIndex firstEdge = halfEdges.getCount();
	// if the input face data has 0 vertices
	if(indices.size() == 0)
	{
		t0.anyHalfEdge = MM_INVALID_INDEX;
	} else
	{
		t0.anyHalfEdge = firstEdge;
	}
	PolygonIndex const faceIndex = (PolygonIndex const) polygonsContainer.resizeForNewElement();
	polygon(faceIndex) = t0;

	// insert polygon at the end of the polygons list
	for(VertexIndex i = 0; i < indices.size(); i++)
	{
		const VertexIndex svIndex = indices[i] + baseVertex;
		const VertexIndex evIndex = (VertexIndex) indices[(i + 1) % indices.size()] + baseVertex;

		assert(vertices.isValid(svIndex) && vertices.isValid(evIndex));

		HalfEdgeIndex halfEdgeIndex = halfEdges.add(svIndex, evIndex, faceIndex);
		HalfEdgeData::HalfEdge& halfEdge = halfEdges.halfEdge(halfEdgeIndex);

		if(i == 0) halfEdge.prev = (HalfEdgeIndex)(firstEdge + indices.size() - 1);
		else halfEdge.prev = firstEdge + i - 1;

		halfEdge.next = firstEdge + ((i + 1) % indices.size());
	}

	return faceIndex;
}
void Polygons::remove(PolygonIndex const index)
{
	if(!polygonsContainer.isValid(index)) return;

	PolygonData::Polygon& poly = polygon(index);
	auto& halfEdges = owner.getHalfEdges();

	HalfEdgeIndexContainer halfEdgeList;
	getHalfEdgeIndices(index, halfEdgeList);
	for (auto heIndex : halfEdgeList)
	{
		halfEdges.remove(heIndex);
	}

	poly.anyHalfEdge = MM_INVALID_INDEX;
	polygonsContainer.setValid(index, false);
}

/**
Gets all the faces surrounding (sharing edges) to specified face.
Uses the half edge pointers pair edges to get all faces surrounding
@param polygonIndex - face/polygon to get vertices from
@param faceList - return list of face
*/
void Polygons::getSurroundingPolygonIndices(const PolygonIndex polygonIndex, PolygonIndexContainer &polygonList) const
{
	if(polygonIndex == MM_INVALID_INDEX) return;

	auto& halfEdges = owner.getHalfEdges();
	HalfEdgeIndex const firstEdge = polygon(polygonIndex).anyHalfEdge;
	HalfEdgeIndex halfEdgeIndex = firstEdge;

	do {
		auto const& halfEdge = halfEdges.halfEdge(halfEdgeIndex);
		if (halfEdge.pair != MM_INVALID_INDEX)
		{
			auto const& halfEdgePair = halfEdges.halfEdge(halfEdge.pair);
			assert(polygonIndex != halfEdgePair.polygonIndex);
			polygonList.push_back(halfEdgePair.polygonIndex);
		}
		halfEdgeIndex = halfEdges.halfEdge(halfEdgeIndex).next;
	} while( halfEdgeIndex != firstEdge );
}

/**
Gets all the edges attached to the face.
Uses the half edge next pointers to get all edges
@param faceIndex - face/polygon to get edges from
@param halfEdgeList - return list of edges
*/
void Polygons::getHalfEdgeIndices(PolygonIndex const polygonIndex, HalfEdgeIndexContainer& halfEdgeList) const
{
	if(polygonIndex == MM_INVALID_INDEX) return;
	if (!isValid(polygonIndex)) return;

	auto const& polys = polygons();
	auto const& halfEdges = owner.getHalfEdges();
	auto const& hes = halfEdges.halfEdges();

	HalfEdgeIndex const firstHalfEdgeIndex = polys[polygonIndex].anyHalfEdge;
	HalfEdgeIndex halfEdgeIndex = firstHalfEdgeIndex;

	if(halfEdgeIndex == MM_INVALID_INDEX) return;

	// gather half edges for triangles
	do
	{
		if (!halfEdges.isValid(halfEdgeIndex)) break;

		halfEdgeList.push_back(halfEdgeIndex);
		halfEdgeIndex = hes[halfEdgeIndex].next;
	} while(halfEdgeIndex != firstHalfEdgeIndex);
}
void Polygons::visitHalfEdges(PolygonIndex const polygonIndex, std::function<void(HalfEdgeIndex const)> const& func) const
{
	if (polygonIndex == MM_INVALID_INDEX) return;
	if (!isValid(polygonIndex)) return;

	auto const& polys = polygons();
	auto& halfEdges = owner.getHalfEdges();

	halfEdges.visitLoop(polys[polygonIndex].anyHalfEdge, func);
}

/**
Gets all the vertices attached to the face.
Uses the half edge pointers to get all vertices that make up this face
@param faceIndex - face/polygon to get vertices from
@param vertexList - return list of vertex
*/
void Polygons::getVertexIndices(PolygonIndex const faceIndex, VertexIndexContainer& vertexList) const
{
	if(faceIndex == MM_INVALID_INDEX) return;

	auto const& vertices = owner.getVertices();
	auto const& halfEdges = owner.getHalfEdges();
	auto const& polys = polygons();
	auto const& hes = halfEdges.halfEdges();

	HalfEdgeIndex const firstHalfEdgeIndex = polys[faceIndex].anyHalfEdge;
	HalfEdgeIndex halfEdgeIndex = firstHalfEdgeIndex;
	if(halfEdgeIndex == MM_INVALID_INDEX) return;

	// gather vertices for polygons
	do
	{
		if (!halfEdges.isValid(halfEdgeIndex)) break;

		auto const& halfEdge = hes[halfEdgeIndex];
		if (vertices.isValid(halfEdge.startVertexIndex))
		{
			vertexList.push_back(halfEdge.startVertexIndex);
		}

		assert((halfEdgeIndex == firstHalfEdgeIndex) ||
			(halfEdgeIndex != halfEdge.next));

		halfEdgeIndex = halfEdge.next;
	} while(halfEdgeIndex != firstHalfEdgeIndex);
}

auto Polygons::getVertexCount(PolygonIndex const faceIndex_) const -> size_t
{
	if(faceIndex_ == MM_INVALID_INDEX) return 0;

	auto const& vertices = owner.getVertices();
	auto const& halfEdges = owner.getHalfEdges();
	auto const& polys = polygons();
	auto const& hes = halfEdges.halfEdges();

	HalfEdgeIndex const firstHalfEdgeIndex = polys[faceIndex_].anyHalfEdge;
	HalfEdgeIndex halfEdgeIndex = firstHalfEdgeIndex;
	if(halfEdgeIndex == MM_INVALID_INDEX) return 0;

	size_t vertexCount = 0;
	do
	{
		if(!halfEdges.isValid(halfEdgeIndex)) break;

		auto const& halfEdge = hes[halfEdgeIndex];
		if(vertices.isValid(halfEdge.startVertexIndex))
		{
			vertexCount++;
		}

		assert((halfEdgeIndex == firstHalfEdgeIndex) ||
			   (halfEdgeIndex != halfEdge.next));

		halfEdgeIndex = halfEdge.next;
	} while(halfEdgeIndex != firstHalfEdgeIndex);

	return vertexCount;
}


void Polygons::visitVertices(PolygonIndex const polygonIndex, std::function<void(VertexIndex const)> const& func) const
{
	VertexIndexContainer polyIndices;
	polyIndices.reserve(10);

	getVertexIndices(polygonIndex, polyIndices);
	for (auto vi : polyIndices)
	{
		func(vi);
	}
}
void Polygons::visitValidVertices(std::function<void(PolygonIndex const, VertexIndex const)> const& func) const
{
	VertexIndexContainer polyIndices;
	polyIndices.reserve(10);

	visitValid([this, &polyIndices, &func](PolygonIndex const polygonIndex) {
		polyIndices.clear();
		getVertexIndices(polygonIndex, polyIndices);
		for (auto vi : polyIndices)
		{
			func(polygonIndex, vi);
		}
	});
}


void Polygons::repack()
{
	// remove all invalid polygons using a polygon remap table
	// not very efficient as does a edge pair rebuild rather than remap that

	auto const& vertices = owner.getVertices();
	auto& halfEdges = owner.getHalfEdges();

	// [index] = existing polygon index of valid indices
	std::vector<PolygonIndex> newToOld;
	std::vector<PolygonIndex> oldToNew; // MM_INVALID_INDEX if no new index
	newToOld.resize(polygonsContainer.size());
	oldToNew.resize(polygonsContainer.size());

	size_t validCount = 0;
	size_t invalidCount = 0;
	for (auto i = 0u; i < polygonsContainer.size(); ++i)
	{
		if (isValid(i))
		{
			auto& poly = polygon((PolygonIndex)i);
			assert(poly.anyHalfEdge != MM_INVALID_INDEX);

			newToOld[validCount] = i;
			oldToNew[i] = (PolygonIndex) validCount;
			validCount++;
		}
		else
		{
			oldToNew[i] = MM_INVALID_INDEX;
			invalidCount++;
		}
	}

	// nothing to repack
	if (invalidCount == 0) return;

	newToOld.resize(validCount);

	PolygonElementsContainer newPolygonCon;
	polygonsContainer.cloneTo(newPolygonCon);
	newPolygonCon.resize(validCount);
	newPolygonCon.resetValidFlags();

	for(size_t i = 0; i < newPolygonCon.getSizeOfElementContainer(); ++i)
	{
		PolygonElementsContainer::Ptr oldCon = polygonsContainer.getElementContainer(i);
		PolygonElementsContainer::Ptr newCon = newPolygonCon.getElementContainer(i);

		for(size_t polyIndex = 0; polyIndex < validCount; ++polyIndex)
		{
			oldCon->unsafeCopyElementTo(*newCon, newToOld[polyIndex], polyIndex);
		}
	}
	// polygons now remapped
	newPolygonCon.cloneTo(polygonsContainer);

	// update each halfedge of every polygon
	for(size_t polyIndex = 0; polyIndex < validCount; ++polyIndex)
	{
		auto& poly = polygon((PolygonIndex) polyIndex);

		assert(poly.anyHalfEdge != MM_INVALID_INDEX);

		HalfEdgeIndex const firstHalfEdgeIndex = poly.anyHalfEdge;
		HalfEdgeIndex halfEdgeIndex = firstHalfEdgeIndex;

		do
		{
			if (!halfEdges.isValid(halfEdgeIndex)) break;

			HalfEdgeData::HalfEdge& halfEdge = halfEdges.halfEdge(halfEdgeIndex);
			if(halfEdge.polygonIndex != MM_INVALID_INDEX)
			{
				halfEdge.polygonIndex = (PolygonIndex) polyIndex;
			}
			halfEdgeIndex = halfEdge.next;
		} while(halfEdgeIndex != firstHalfEdgeIndex);
	}

	// this could (and should be done my remapping)
	halfEdges.breakPairs();
	halfEdges.connectPairs();
}

void Polygons::visitAll(std::function<void(PolygonIndex const)> const& func) const
{
	for (auto i = 0u; i < polygonsContainer.size(); ++i)
	{
		func(i);
	}
}

void Polygons::visitValid(std::function<void(PolygonIndex const)> const& func) const
{
	for (auto i = 0u; i < polygonsContainer.size(); ++i)
	{
		if (isValid(i))
		{
			func(i);
		}
	}
}

}