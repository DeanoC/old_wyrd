#include "meshmod.h"
#include "halfedgedata/halfedgedata.h"
#include "polygons.h"
#include "halfedges.h"
#include "vertices.h"
#include "mesh.h"
#include <algorithm>
#include <string>

namespace MeshMod {

HalfEdges::HalfEdges(Mesh& owner_) :
	owner(owner_)
{
}

HalfEdges::~HalfEdges()
{
}

/**
Adds a half edge to the mesh.
Handles all the bookkeeping the half edge's correct, doesn't have a next edge index at this stage
@param svIndex start vertex index
@param evIndex end vertex index
@param poIndex polygon index the edge is attached to
@return index of half edge
*/
HalfEdgeIndex HalfEdges::add(VertexIndex const svIndex, VertexIndex const evIndex, PolygonIndex const faceIndex)
{
	// add half edge
	HalfEdgeData::HalfEdge e0;
	e0.startVertexIndex = svIndex;
	e0.endVertexIndex = evIndex;
	e0.polygonIndex = faceIndex;
	e0.next = MM_INVALID_INDEX;
	e0.prev = MM_INVALID_INDEX;

	auto& vertices = owner.getVertices();

	HalfEdgeIndex halfEdgeIndex = (HalfEdgeIndex) halfEdgesContainer.resizeForNewElement();
	halfEdge(halfEdgeIndex) = e0;

	auto& verticesHalfEdge = vertices.getOrAddAttributes<VertexData::HalfEdges>();

	// check for duplicates
	if(std::find(verticesHalfEdge[svIndex].halfEdgeIndexContainer.begin(),
				 verticesHalfEdge[svIndex].halfEdgeIndexContainer.end(),
				 halfEdgeIndex) == verticesHalfEdge[svIndex].halfEdgeIndexContainer.end())
	{
		verticesHalfEdge[svIndex].halfEdgeIndexContainer.push_back(halfEdgeIndex);
	}

	if(std::find(verticesHalfEdge [evIndex].halfEdgeIndexContainer.begin(),
				 verticesHalfEdge [evIndex].halfEdgeIndexContainer.end(),
				 halfEdgeIndex) == verticesHalfEdge [evIndex].halfEdgeIndexContainer.end())
	{
		verticesHalfEdge[evIndex].halfEdgeIndexContainer.push_back(halfEdgeIndex);
	}

	return halfEdgeIndex;
}

void HalfEdges::remove(HalfEdgeIndex const index)
{
	if(!isValid(index)) return;

	Vertices& vertices = owner.getVertices();
	Polygons& polygons = owner.getPolygons();

	HalfEdgeData::HalfEdge& hedge = halfEdge(index);
	VertexData::HalfEdges& vertexHalfEdges = vertices.getAttributes<VertexData::HalfEdges>();

	// change the polygons any half edge to next if possible
	if (hedge.polygonIndex != MM_INVALID_INDEX)
	{
		auto& polygon = polygons.polygon(hedge.polygonIndex);
		if (hedge.next == MM_INVALID_INDEX || hedge.next == index)
		{
			polygon.anyHalfEdge = MM_INVALID_INDEX;
			polygons.remove(hedge.polygonIndex);
		}
		else
		{
			polygon.anyHalfEdge = hedge.next;
		}
	}

	// link previous halfedge to next half edge (and vice versa)
	if (hedge.prev != MM_INVALID_INDEX)
	{
		HalfEdgeData::HalfEdge& prevHE = halfEdge(hedge.prev);
		prevHE.endVertexIndex = hedge.startVertexIndex;
		prevHE.next = hedge.next;
	}
	if(hedge.next != MM_INVALID_INDEX)
	{
		HalfEdgeData::HalfEdge& nextHE = halfEdge(hedge.next);
		nextHE.startVertexIndex = hedge.endVertexIndex;
		nextHE.prev = hedge.prev;
	}

	// remove from pair's pair 
	if (hedge.pair != MM_INVALID_INDEX)
	{
		auto& pedge = halfEdge(hedge.pair);
		pedge.pair = MM_INVALID_INDEX;
	}

	// from this half edge from start and end vertex half edge lists
	if (vertices.isValid(hedge.startVertexIndex))
	{
		VertexData::HalfEdge& svhe = vertexHalfEdges[hedge.startVertexIndex];
		auto sit = std::find(svhe.halfEdgeIndexContainer.begin(),
			svhe.halfEdgeIndexContainer.end(), index);
		if (sit != svhe.halfEdgeIndexContainer.end())
		{
			svhe.halfEdgeIndexContainer.erase(sit);
		}
	}

	if (vertices.isValid(hedge.endVertexIndex))
	{
		auto& evhe = vertexHalfEdges[hedge.endVertexIndex];
		auto eit = std::find(evhe.halfEdgeIndexContainer.begin(),
			evhe.halfEdgeIndexContainer.end(), index);
		if (eit != evhe.halfEdgeIndexContainer.end()) evhe.halfEdgeIndexContainer.erase(eit);
	}

	hedge.prev = MM_INVALID_INDEX;
	hedge.next = MM_INVALID_INDEX;
	hedge.pair = MM_INVALID_INDEX;
	hedge.polygonIndex = MM_INVALID_INDEX;
	halfEdgesContainer.setValid(index, false);

}
void HalfEdges::removeEdge(HalfEdgeIndex const index)
{
	HalfEdgeData::HalfEdge& hedge = halfEdge(index);
	// remove pair half edge 
	if (hedge.pair != MM_INVALID_INDEX)
	{
		remove(hedge.pair);
	}
	remove(index);
}
void HalfEdges::repack()
{
	// repack all valid half edges
	auto& vertices = owner.getVertices();
	auto& halfEdges = owner.getHalfEdges();
	auto& polygons = owner.getPolygons();

	// [index] = existing half edge index of valid indices
	std::vector<VertexIndex> newToOld;
	std::vector<VertexIndex> oldToNew; // MM_INVALID_INDEX if no new index
	newToOld.resize(halfEdgesContainer.size());
	oldToNew.resize(halfEdgesContainer.size());

	size_t validCount = 0;
	size_t invalidCount = 0;
	for (auto heIndex = 0u; heIndex < halfEdgesContainer.size(); ++heIndex)
	{
		if (isValid(heIndex))
		{
			auto& halfEdge = halfEdges.halfEdge(heIndex);
			assert(isValid(halfEdge.prev));
			assert(isValid(halfEdge.next));
			newToOld[validCount] = heIndex;
			oldToNew[heIndex] = (VertexIndex) validCount;
			validCount++;
		}
		else
		{
			oldToNew[heIndex] = MM_INVALID_INDEX;
			invalidCount++;
		}
	}

	// nothing to repack
	if (invalidCount == 0) return;
	
	polygons.visitValid([this, &polygons, &oldToNew](PolygonIndex polyIndex) {
		auto& poly = polygons.polygon(polyIndex);

		if (poly.anyHalfEdge == MM_INVALID_INDEX)
		{
			// mark this polygon as invalid
			polygons.remove(polyIndex);
			return;
		}
		if (oldToNew[poly.anyHalfEdge] == MM_INVALID_INDEX)
		{
			// mark this polygon as invalid
			polygons.remove(polyIndex);
			return;
		}
	});

	newToOld.resize(validCount);

	HalfEdgeElementsContainer newHalfEdgeCon;
	halfEdgesContainer.cloneTo(newHalfEdgeCon);
	newHalfEdgeCon.resize(validCount);
	newHalfEdgeCon.resetValidFlags();

	for (size_t i = 0; i < newHalfEdgeCon.getSizeOfElementContainer(); ++i)
	{
		HalfEdgeElementsContainer::Ptr oldCon = halfEdgesContainer.getElementContainer(i);
		HalfEdgeElementsContainer::Ptr newCon = newHalfEdgeCon.getElementContainer(i);

		for (size_t heIndex = 0; heIndex < validCount; ++heIndex)
		{
			oldCon->unsafeCopyElementTo(*newCon, newToOld[heIndex], heIndex);
		}
	}

	// half edge now remapped
	newHalfEdgeCon.cloneTo(halfEdgesContainer);

	auto& vertexHalfEdges = vertices.getAttributes<VertexData::HalfEdges>();
	for (auto newIndex = 0u; newIndex < getCount(); ++newIndex)
	{
		auto& halfEdge = halfEdges.halfEdge(newIndex);

		halfEdge.prev = oldToNew[halfEdge.prev];
		halfEdge.next = oldToNew[halfEdge.next];
		auto oldIndex = newToOld[newIndex];

		if (halfEdge.pair != MM_INVALID_INDEX)
		{
			if(oldToNew[halfEdge.pair] >= validCount)
				halfEdge.pair = MM_INVALID_INDEX;
			else
				halfEdge.pair = oldToNew[halfEdge.pair];
		}
	}

	vertices.visitAll([this, &halfEdges, &vertexHalfEdges, &oldToNew, validCount](VertexIndex vertexIndex)
	{
		auto& vheList = vertexHalfEdges[vertexIndex].halfEdgeIndexContainer;
		for (auto& heIndex : vheList)
		{

			if (oldToNew[heIndex] < validCount)
			{
				heIndex = oldToNew[heIndex];
			}
			else
			{
				heIndex = MM_INVALID_INDEX;
			}
		}
		vheList.erase(std::remove_if(vheList.begin(), vheList.end(), [](HalfEdgeIndex heIndex) {
			return heIndex == MM_INVALID_INDEX;
		}), vheList.end());
	});

	polygons.visitValid([this, &polygons, &oldToNew](PolygonIndex polyIndex) {
		auto& poly = polygons.polygon(polyIndex);
		assert(oldToNew[poly.anyHalfEdge] != MM_INVALID_INDEX);
		poly.anyHalfEdge = oldToNew[poly.anyHalfEdge];
	});

}
/**
Breaks all edge pairs for the entire mesh.
Break each half edge from its other half
*/
void HalfEdges::breakPairs()
{
	std::vector<HalfEdgeData::HalfEdge>::iterator edgeIt = halfEdges().begin();
	while(edgeIt != halfEdges().end())
	{
		(*edgeIt).pair = MM_INVALID_INDEX;
		++edgeIt;
	}
}

/**
Connects Half edge to the other half edges for the entire mesh.
Builds the half edge to half edge data structures
*/
void HalfEdges::connectPairs()
{
	auto const& vertices = owner.getVertices();
	auto const& polygons = owner.getPolygons();

	for(auto& poly : polygons.polygons())
	{
		if(!polygons.isValid(poly)) continue;

		const HalfEdgeIndex firstEdge = poly.anyHalfEdge;
		HalfEdgeIndex halfEdgeIndex = firstEdge;

		do
		{
			if (!isValid(halfEdgeIndex)) break;

			// scan around edges attached to the start vertex of this edge
			// pair up any edge with the same start and end
			HalfEdgeData::HalfEdge& he = halfEdge(halfEdgeIndex);
			VertexIndex startVert = he.startVertexIndex;
			VertexIndex endVert = he.endVertexIndex;
			if (startVert == MM_INVALID_INDEX || 
				endVert == MM_INVALID_INDEX)
				break;

			// swap indices for consistent ordering
			if(startVert > endVert)
			{
				std::swap(startVert, endVert);
			}
			auto const& vertexHalfEdges = vertices.getAttributes<VertexData::HalfEdges>();

			//iterate through the edges connect to the start vertex
			for(auto vheIndex : vertexHalfEdges.get(startVert).halfEdgeIndexContainer)
			{
				// check that we are not working on our own edge
				if(vheIndex != halfEdgeIndex)
				{
					// does this edge have the same start and end vertex indices, if so its a pair
					HalfEdgeData::HalfEdge& e0 = halfEdge(vheIndex);
					HalfEdgeData::HalfEdge& e1 = halfEdge(halfEdgeIndex);
					if (e0.pair == MM_INVALID_INDEX && e1.pair == MM_INVALID_INDEX)
					{
						if (((e0.startVertexIndex == startVert) &&
							(e0.endVertexIndex == endVert)) ||
							((e0.startVertexIndex == endVert) &&
							(e0.endVertexIndex == startVert)))
						{
							e0.pair = halfEdgeIndex;
							e1.pair = vheIndex;
						}
					}
				}
			}
			halfEdgeIndex = he.next;
		} while(halfEdgeIndex != firstEdge);
	}
}

void HalfEdges::visitAll(std::function<void(HalfEdgeIndex const)> const& func)
{
	for (auto i = 0u; i < halfEdgesContainer.size(); ++i)
	{
		func(i);
	}
}

void HalfEdges::visitValid(std::function<void(HalfEdgeIndex const)> const& func)
{
	for (auto i = 0u; i < halfEdgesContainer.size(); ++i)
	{
		if (isValid(i))
		{
			func(i);
		}
	}
}

void HalfEdges::visitLoop(HalfEdgeIndex const firstHalfEdgeIndex, std::function<void(HalfEdgeIndex const)> const& func)
{
	HalfEdgeIndex halfEdgeIndex = firstHalfEdgeIndex;

	if (halfEdgeIndex == MM_INVALID_INDEX) return;

	// vists half edges for polygons
	do
	{
		if (!isValid(halfEdgeIndex)) break;
		func(halfEdgeIndex);

		halfEdgeIndex = halfEdge(halfEdgeIndex).next;
	} while (halfEdgeIndex != firstHalfEdgeIndex);
}

}
