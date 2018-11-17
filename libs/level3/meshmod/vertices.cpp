//
// Created by Computer on 31/07/2018.
//

#include "meshmod.h"
#include <algorithm>
#include <unordered_set>
#include <iterator>
#include <set>
#include "polygons.h"
#include "halfedges.h"
#include "vertices.h"
#include "mesh.h"

namespace MeshMod
{

Vertices::Vertices(Mesh& owner_) :	owner(owner_)
{
}

Vertices::~Vertices()
{
}

/**
Adds a new position.
Adds a new position (which has the effect of creating a new vertex), updates the point rep
@param x pos.x
@param y pos.y
@param z pos.z
@return index of new vertex
*/
VertexIndex Vertices::add( const float x, const float y, const float z )
{
	// default to linking to itself
	VertexIndex const link = (VertexIndex) getVerticesContainer().resizeForNewElement();
	// add position
	position(link) = VertexData::Position(x,y,z);

	// add point rep
	auto& pointReps = getOrAddAttributes<VertexData::PointReps>();
	pointReps[link] = VertexData::PointRep(link);

	return link;
}
namespace {
//! sorts things based on the float value, you can then
//! get the original version via the index
struct AxisSortCompare
{
	AxisSortCompare(MeshMod::VertexData::Positions const& positions_,
					MeshMod::VertexData::Axis axis_) :
			positions(positions_),
			axis(axis_) {}

	/// function to sort by position[axis]
	bool operator()(const MeshMod::VertexData::SortIndex& a, const MeshMod::VertexData::SortIndex& b) const
	{
		return (positions[a.index].get(axis) < positions[b.index].get(axis));
	}

private:
	MeshMod::VertexData::Positions const& positions;
	MeshMod::VertexData::Axis axis;
};
}
/**
Create a point rep, based on the idea that any thing close (fEpsilon) to the
same position, is the 'similar'
Uses a 1D sort to speed this up, assuming that most things will be reasonable
distributed along the X. Obviously worse case (A line constant in X) could be
very slow. but I'm betting that in practice this will be fast enough for most
data sets.
@param fEpsilon how similar is similar?
*/
void Vertices::createPointReps(VertexData::Axis axis, float fEpsilon)
{
	static const char* axisNames[] = { "X axis", "Y axis", "Z axis" };

	if (getCount() == 0) return;
	if (verticesContainer.getSizeOfElementContainer() == 2) return;

	auto& sortEle = *getVerticesContainer().getOrAddElements<VertexData::SortIndices>(axisNames[(uint8_t) axis]);

	// fill sort mapper with identity mapping
	VertexData::SortIndices::iterator idenIt = sortEle.elements.begin();
	while( idenIt != sortEle.elements.end() ) {
		(*idenIt).index = (unsigned int) std::distance(sortEle.elements.begin(), idenIt);
		++idenIt;
	}

	AxisSortCompare sorter(positions(), axis);
	std::sort( sortEle.elements.begin(), sortEle.elements.end(), sorter );

	// clear existing point representation
	auto& pointReps = getOrAddAttributes<VertexData::PointReps>();
	auto prIt = pointReps.begin();
	while(prIt != pointReps.end())
	{
		auto const prIndex = (VertexIndex) std::distance(pointReps.begin(), prIt);
		// a point rep set to MM_INVALID_INDEX is 'deleted' if its deleted leave it deleted
		if((*prIt).next != MM_INVALID_INDEX) (*prIt).next = prIndex;

		++prIt;
	}

	for(auto siIt = sortEle.cbegin() + 1; siIt != sortEle.cend();++siIt)
	{
		VertexIndex const siIndex = (VertexIndex) siIt->index;

		float const cmp = position(siIndex).get(axis);

		// need to back up until we are further than epsilon (with a nice fudge factor)
		// away, test everything in this range and add it to our point rep if in range
		// we never scan forward as the next johnny will search backwards
		auto siBackIt = siIt - 1;
		while((fabsf((position((VertexIndex)siBackIt->index).get(axis)) - cmp) < fEpsilon * 5))
		{
			VertexIndex const siBackIndex  = (VertexIndex)siBackIt->index;
			// ignore deleted vertices
			if(!isValid(siBackIndex))
			{
				--siBackIt;
				continue;
			}

			if(position(siBackIndex).equal(position(siIndex), fEpsilon))
			{
				// we stop at the first equal as the previous iterations
				// will have found others.
				const VertexIndex link = pointReps[siBackIndex].next;
				pointReps[siBackIndex].next = siIndex;
				pointReps[siIndex].next = link;
				break;
			}
			// break out if we butt up against the first element (but after testing it)
			if (siBackIt == sortEle.cbegin())
				break;

			--siBackIt;
		}
	}
}
/**
Clones the input vertex.
An exact copy of the input vertex is created and its index returned, point reps are kept upto date
@param vertexToCopy the index of the vertex to clone
@return index of the new vertex
*/
VertexIndex Vertices::clone(const VertexIndex vertexToCopy)
{
	if(!isValid(vertexToCopy)) return MM_INVALID_INDEX;

	VertexIndex const newIndex = (VertexIndex) getVerticesContainer().cloneElement(vertexToCopy);

	// add to the point rep list (increased via vertex container)
	auto& pointReps = getOrAddAttributes<VertexData::PointReps>();
	VertexIndex const temp = pointReps[vertexToCopy].next;
	pointReps[vertexToCopy] = VertexData::PointRep(newIndex);
	pointReps[newIndex] = VertexData::PointRep(temp);

	return newIndex;
}


/**
Removes a vertex position.
Also note vertices aren't physical removed they will have a point rep of MM_INVALID_INDEX
this is to keep the potentially expensive reindex operation to a minimum
@param index the index to remove
*/
void Vertices::remove(const VertexIndex vertexIndex)
{
	// ignore deleted vertices
	if (!isValid(vertexIndex)) return;

	auto& halfEdges = owner.getHalfEdges();
	auto& vertexHalfEdges = getAttributes<VertexData::HalfEdges>();
	auto& pointReps = getOrAddAttributes<VertexData::PointReps>();

	VertexIndexContainer vertList;
	getSimilarVertexIndices(vertexIndex, vertList);

	// first remove all those who are similar to the original
	std::vector<VertexIndex>::iterator vlIt = vertList.begin();
	VertexIndex i = pointReps[vertexIndex].next;
	while (i != vertexIndex)
	{
		VertexIndex del = i;
		i = pointReps[i].next;
		moveToSimilarIfPossible(del);
	}

	// mark all the halfedges attached to this vertex with an invalid vertex id
	HalfEdgeIndexContainer halfEdgeList;
	getVertexHalfEdges(vertexIndex, halfEdgeList);

	for (auto halfEdgeIndex : halfEdgeList)
	{
		auto& halfEdge = halfEdges.halfEdge(halfEdgeIndex);
		if(!isValid(halfEdge.startVertexIndex)) halfEdge.startVertexIndex = MM_INVALID_INDEX;
		if(!isValid(halfEdge.endVertexIndex)) halfEdge.endVertexIndex = MM_INVALID_INDEX;
	}

	verticesContainer.setValid(vertexIndex, false);
}
bool Vertices::moveToSimilarIfPossible(VertexIndex index)
{
	// ignore deleted vertices
	if (!isValid(index)) return true;

	auto& halfEdges = owner.getHalfEdges();
	auto& vertexHalfEdges = getAttributes<VertexData::HalfEdges>();
	auto& pointReps = getOrAddAttributes<VertexData::PointReps>();

	// decide wether we have a similar vertex to move to
	VertexIndex const moveTo = pointReps[index].next;
	if (moveTo == index) return true;

	HalfEdgeIndexContainer vertexHalfEdgeList;
	getVertexHalfEdges(index, vertexHalfEdgeList);

	HalfEdgeIndexContainer::iterator edIt = vertexHalfEdgeList.begin();
	while (edIt != vertexHalfEdgeList.end()) {
		auto& edge = halfEdges.halfEdge(*edIt);
		if (edge.pair != MM_INVALID_INDEX)
		{
			auto& pedge = halfEdges.halfEdge(edge.pair);
			if (pedge.startVertexIndex == index) {
				if (pedge.endVertexIndex != moveTo) {
					pedge.startVertexIndex = moveTo;
				}
			}
			if (pedge.endVertexIndex == index) {
				if (edge.startVertexIndex != moveTo) {
					pedge.endVertexIndex = moveTo;
				}
			}
		}
		if (edge.startVertexIndex == index) {
			if (edge.endVertexIndex != moveTo) {
				edge.startVertexIndex = moveTo;
			}
		}
		if (edge.endVertexIndex == index) {
			if (edge.startVertexIndex != moveTo) {
				edge.endVertexIndex = moveTo;
			}
		}
		++edIt;
	}

	// loop to find the previous link in the point rep chain
	VertexIndex i = pointReps[index].next;
	do {
		i = pointReps[i].next;
	} while (pointReps[i].next != index);

	pointReps[i].next = pointReps[index].next;
	pointReps[index] = MM_INVALID_INDEX;
	verticesContainer.setValid(index, false);

	return false;
}

/**
Collapse all similar position to a single one, ignores all other data similarity is just for position
Generally do before you add any other data as it will potionally remove that other data
@param fEpsilon how similar is similar?
*/
void Vertices::removeAllSimilarPositions(VertexData::Axis axis, float fEpsilon)
{
	// NOTE : I'm not sure I should do these 3 operations...
	// but then this is a extremely destructive operations so
	// meh if you don't know what your doing you've got bigger problems

	// first stop is to use a point rep to accelerate the op
	// this should be fairly fast due to a 1D sort
	createPointReps(axis, fEpsilon);

	auto& halfEdges = owner.getHalfEdges();

	// we rely on half edge connection (TODO only do this if not currently build)
	halfEdges.breakPairs();
	halfEdges.connectPairs();

	// walk the position array
	auto& pointReps = getAttributes<VertexData::PointReps>();
	std::vector<VertexIndex> vertList;
	for(auto const& pos : positions())
	{
		VertexIndex vertexIndex = positions().distance<VertexIndex>(pos);

		// skip deleted one
		if(!isValid(vertexIndex)) continue;

		// using the points list we can now quickly find our similar brethren
		vertList.clear();
		getSimilarVertexIndices(vertexIndex, vertList);

		// now remove all those who are similar to the original
		std::vector<VertexIndex>::iterator vlIt = vertList.begin();
		VertexIndex i = pointReps[vertexIndex].next;
		while(i != vertexIndex)
		{
			VertexIndex del = i;
			i = pointReps[i].next;
			moveToSimilarIfPossible(del);
		}
	}
}

/**
Do the 2 vertices have the same position?.
Uses the pointrep to check if the 2 input vertices have the same position
@param i0 first vertex index
@param i1 second vertex index
@return boolean result
*/
bool Vertices::similarPosition(const VertexIndex i0, const VertexIndex i1) const
{
	// the obvious really quick check :-)
	if(i0 == i1)
	{
		return true;
	}

	// deleted vertex never match the same
	if(!isValid(i0) || isValid(i1)) return false;

	auto& pointReps = getAttributes<VertexData::PointReps>();

	// old school delete marker
	if(pointReps[i0].next == MM_INVALID_INDEX ||
	   pointReps[i1].next == MM_INVALID_INDEX)
	{
		return false;
	}

	// walk the point rep, similar vertices match the samePosition test
	unsigned int i = i0;
	do
	{
		if(pointReps[i].next == i1)
			return true;

		i = pointReps[i].next;
	} while(pointReps[i].next != i0);

	// nope, we rely on point rep so we are not the same
	// NOTE : in future this might not be good enough...
	// and provide a more extensive version for when you really really
	// want an exhaustive samePosition (i.e blendshapes may have
	// same position but they might not be similar...)
	return false;
}

/**
Get a list of all vertex that share the input vertex position.
Use the point rep to produce a list of vertex that share the same(ish) position
@param x pos.x
@param y pos.y
@param z pos.z
@return index of new vertex
*/
void Vertices::getSimilarVertexIndices(VertexIndex const vertexIndex, VertexIndexContainer& vertList) const
{
	VertexIndex i = vertexIndex;
	// a deleted vertex has no 'similar' vertices
	if(!isValid(i)) return;

	auto& pointReps = getAttributes<VertexData::PointReps>();
	if(pointReps[i].next == MM_INVALID_INDEX) return;

	i = pointReps[i].next;
	while (i != vertexIndex)
	{
		vertList.push_back(i);
		i = pointReps[i].next;
	};
}

/**
Is this vertex similar to a vertex in the list of vertices provided
@return index of first similar vertex
*/
VertexIndex Vertices::hasPosition(VertexIndex const i0, VertexIndexContainer const& vertexList) const
{
	VertexIndexContainer::const_iterator vIt = vertexList.begin();
	while(vIt != vertexList.end())
	{
		if(similarPosition(i0, (*vIt)) == true)
		{
			return (*vIt);
		}
		++vIt;
	}
	return MM_INVALID_INDEX;
}

/**
get all the edge attached to this vertex.
Uses the edge set to quickly get all edge this vertex is attached to
@param vertexIndex vertex to get edge from
@param halfEdgeList return edge list
*/
void Vertices::getVertexHalfEdges(VertexIndex const vertexIndex, HalfEdgeIndexContainer &halfEdgeList) const
{
	// a deleted vertex has no edges
	if(!isValid(vertexIndex))return;

	std::set<HalfEdgeIndex> halfEdgeSet;

	auto const& vertexHalfEdges = getAttributes<VertexData::HalfEdges>();
	auto& pointReps = getAttributes<VertexData::PointReps>();

	VertexIndex curVert = vertexIndex;
	do
	{
		assert(isValid(curVert));
		VertexData::HalfEdge const &halfEdge = vertexHalfEdges [curVert];
		halfEdgeSet.insert(halfEdge.halfEdgeIndexContainer.begin(), halfEdge.halfEdgeIndexContainer.end());
		curVert = pointReps[curVert].next;
	} while(vertexIndex != curVert);

	std::copy(halfEdgeSet.cbegin(), halfEdgeSet.cend(),
			  std::inserter(halfEdgeList, halfEdgeList.end()));
}

void Vertices::visitAll(std::function<void(VertexIndex const)> const& func)
{
	for (auto i = 0u; i < verticesContainer.size(); ++i)
	{
		func(i);
	}
}

void Vertices::visit(VertexIndexContainer const& indexList, std::function<void(VertexIndex const)> const& func)
{
	for (auto const index : indexList)
	{
		func(index);
	}
}

void Vertices::visitValid(std::function<void(VertexIndex const)> const& func)
{
	for (auto i = 0u; i < verticesContainer.size(); ++i)
	{
		if (isValid(i))
		{
			func(i);
		}
	}
}

void Vertices::visitSimilarVertexIndices(VertexIndex const vertexIndex, std::function<void(VertexIndex const)> const& func)
{
	VertexIndex i = vertexIndex;
	// a deleted vertex has no 'similar' vertices
	if (!isValid(i)) return;

	auto& pointReps = getAttributes<VertexData::PointReps>();
	if (pointReps[i].next == MM_INVALID_INDEX) return;
	
	i = pointReps[i].next;
	while(i != vertexIndex)
	{
		func(i);
		i = pointReps[i].next;
	};
}

void Vertices::visitHalfEdges(VertexIndex const vertexIndex, std::function<void(HalfEdgeIndex const)> const& func)
{
	auto const& vertexHalfEdges = getAttributes<VertexData::HalfEdges>();
	auto& pointReps = getAttributes<VertexData::PointReps>();

	VertexIndex curVert = vertexIndex;
	do
	{
		assert(isValid(curVert));
		VertexData::HalfEdge const &halfEdge = vertexHalfEdges[curVert];
		for (auto const heIndex : halfEdge.halfEdgeIndexContainer)
		{
			func(heIndex);
		}
		curVert = pointReps[curVert].next;
	} while (vertexIndex != curVert);
}



unsigned int Vertices::getValency(VertexIndex const vertexIndex) const
{
	auto const& vertexHalfEdges = getAttributes<VertexData::HalfEdges>();
	return (unsigned int) vertexHalfEdges[vertexIndex].halfEdgeIndexContainer.size();
}

void Vertices::repack(std::vector<VertexIndex>& oldToNew)
{
	// remove all invalid vertices using a vertex remap table
	VerticesElementsContainer& vertCon = verticesContainer;

	// [index] = existing vertex index of valid indices
	std::vector<VertexIndex> newToOld;
	newToOld.resize(vertCon.size());
	oldToNew.clear();
	oldToNew.resize(vertCon.size());

	size_t validCount = 0;
	size_t invalidCount = 0;
	for (auto i = 0u; i < vertCon.size(); ++i)
	{
		if (isValid(i))
		{
			newToOld[validCount] = i;
			oldToNew[i] = (VertexIndex) validCount;
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

	// remap point rep before moving. Point reps can't be used until
	// the physical remap happens
	{
		std::vector<bool> pointMapRemapped(verticesContainer.size());

		VertexIndexContainer similarList;
		auto& pointReps = getAttributes<VertexData::PointReps>();
		visitAll([this, &similarList,
							&pointReps,
							&oldToNew,
							&pointMapRemapped,
							validCount](VertexIndex vIndex) {
			if (isValid(vIndex))
			{
				if (pointMapRemapped[vIndex] == false)
				{
					similarList.clear();
					getSimilarVertexIndices(vIndex, similarList);
					for (auto index : similarList)
					{
						pointReps[index].next = oldToNew[pointReps[index].next];
						assert(pointReps[index].next < validCount);
						pointMapRemapped[index] = true;
					}
					pointReps[vIndex].next = oldToNew[pointReps[vIndex].next];
					pointMapRemapped[vIndex] = true;
				}
			}
		});
	}

	// remap all vertices to there new positions
	VerticesElementsContainer newVerticesCon;
	verticesContainer.cloneTo(newVerticesCon);
	newVerticesCon.resize(validCount);
	newVerticesCon.resetValidFlags();

	for (size_t i = 0; i < newVerticesCon.getSizeOfElementContainer(); ++i)
	{
		VerticesElementsContainer::Ptr oldCon = verticesContainer.getElementContainer(i);
		VerticesElementsContainer::Ptr newCon = newVerticesCon.getElementContainer(i);

		for (size_t vertIndex = 0; vertIndex < validCount; ++vertIndex)
		{
			oldCon->unsafeCopyElementTo(*newCon, newToOld[vertIndex], vertIndex);
		}
	}
	// vertices now remapped
	newVerticesCon.cloneTo(verticesContainer);

	// remap all indices in non vertex data from the old vertices array to updated indices

	// update each halfedges
	HalfEdges& halfEdges = owner.halfEdges;
	for (auto& he : halfEdges.halfEdges())
	{
		HalfEdgeIndex hei = halfEdges.halfEdges().distance<HalfEdgeIndex>(he);
		if (halfEdges.isValid(hei))
		{
			if (he.startVertexIndex != MM_INVALID_INDEX)
			{
				VertexIndex newSVI = oldToNew[he.startVertexIndex];
				if (newSVI < verticesContainer.size())
					he.startVertexIndex = newSVI;
				else
				{
					he.startVertexIndex = MM_INVALID_INDEX;
					halfEdges.remove(hei);
				}
			}
			if (he.endVertexIndex != MM_INVALID_INDEX)
			{
				VertexIndex newEVI = oldToNew[he.endVertexIndex];
				if (newEVI < verticesContainer.size())
					he.endVertexIndex = newEVI;
				else
				{
					he.endVertexIndex = MM_INVALID_INDEX;
					halfEdges.remove(hei);
				}
			}
		}
		else
		{
			he.startVertexIndex = MM_INVALID_INDEX;
			he.endVertexIndex = MM_INVALID_INDEX;
		}
	}
}


}
