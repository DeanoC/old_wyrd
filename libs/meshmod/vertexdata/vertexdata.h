#pragma once
/** \file MeshModVertex.h
   A vertex system.
   Holds  a number of named vertex elements, 
   contains a list of similar vertices (same position, different elements (uv's, normals)
   contains a list of polyMeshModns each vertex is attached to
   (c) 2002 Dean Calver
 */

#if !defined( MESH_MOD_VERTEXDATA_H_ )
#define MESH_MOD_VERTEXDATA_H_

#include "meshmod/types.h"
#include "meshmod/genericcontainters.h"

namespace MeshMod {

/**
	A namespace just to keep vertex data seperate from the reset of MeshMod.
	Custom vertex data obviously doesn't require it, but its not a bad
	idea...
*/
namespace VertexData {

struct BoneWeight
{
	struct Weight
	{
		unsigned int index;
		float weight;

		Weight() {};

		Weight(const unsigned int in_, const float wt_) :
				index(in_), weight(wt_) {};

		// useful for sorting by weight
		bool operator<(const Weight& rhs) const
		{
			return weight < rhs.weight;
		}
	};

	std::vector<Weight> boneData;

	BoneWeight interpolate(BoneWeight const& b, float const t) const
	{
		const float one_minus_t = 1.0f - t;
		// bone weights are hard to interpolate, if they have different indexes
		// or number of weights per vertex. if both are the same we interpolate
		// the actually weight else we just copy our version
		// TODO when copying the index/weight using the magnitude of t to select
		// TODO the source would be better I think
		if(boneData.size() == b.boneData.size())
		{
			BoneWeight nbw;
			nbw.boneData.resize(boneData.size());
			for(unsigned int i = 0; i < boneData.size(); ++i)
			{
				if(boneData[i].index == b.boneData[i].index)
				{
					nbw.boneData[i].index = boneData[i].index;
					nbw.boneData[i].weight = boneData[i].weight * t +
											 b.boneData[i].weight * one_minus_t;
				} else
				{
					nbw.boneData[i].index = boneData[i].index;
					nbw.boneData[i].weight = boneData[i].weight;
				}
			}

			return nbw;
		} else
		{
			return *this;
		}
	}

	BoneWeight interpolate(BoneWeight const& b, BoneWeight const& c, const float u, const float v) const
	{
		// bone weights are hard to interpolate, if they have different indexes
		// or number of weights per vertex. if both are the same we interpolate
		// the actually weight else we just copy our version
		// TODO when copying the index/weight using the magnitude of uvw to select
		// TODO the source would be better I think
		if(boneData.size() == b.boneData.size() &&
		   boneData.size() == c.boneData.size())
		{
			BoneWeight nbw;
			nbw.boneData.resize(boneData.size());
			const float w = 1.0f - v - u;
			for(unsigned int i = 0; i < boneData.size(); ++i)
			{
				if(boneData[i].index == b.boneData[i].index &&
				   boneData[i].index == c.boneData[i].index)
				{
					nbw.boneData[i].index = boneData[i].index;
					nbw.boneData[i].weight = boneData[i].weight * u +
											 b.boneData[i].weight * v +
											 b.boneData[i].weight * w;
				} else
				{
					nbw.boneData[i].index = boneData[i].index;
					nbw.boneData[i].weight = boneData[i].weight;
				}
			}

			return nbw;
		} else
		{
			return *this;
		}
	}


	static const std::string getName() { return "BoneWeight"; };
};

struct PositionDelta
{
	float dx;
	float dy;
	float dz;

	PositionDelta() :
			dx(s_floatMarker), dy(s_floatMarker), dz(s_floatMarker) {};

	PositionDelta(const float x_, const float y_, const float z_) :
			dx(x_), dy(y_), dz(z_) {};

	PositionDelta interpolate(const PositionDelta& b, const float t) const
	{
		const float one_minus_t = 1.0f - t;
		return PositionDelta((dx * t) + (b.dx * one_minus_t),
							 (dy * t) + (b.dy * one_minus_t),
							 (dz * t) + (b.dz * one_minus_t));
	}

	PositionDelta interpolate(const PositionDelta& b, const PositionDelta& c, const float u, const float v) const
	{
		const float w = 1.0f - v - u;
		return PositionDelta((dx * u) + (b.dx * v) + (c.dx * w),
							 (dy * u) + (b.dy * v) + (c.dy * w),
							 (dz * u) + (b.dz * v) + (c.dz * w));
	}

	static const std::string getName() { return "PositionDelta"; };
};

struct SortIndex
{
	size_t index;

	SortIndex() {};

	SortIndex(const unsigned int in_) :
			index(in_) {}

	static const std::string getName() { return "SortIndex"; };
};

struct HalfEdge
{
	HalfEdgeIndexContainer halfEdgeIndexContainer;

	static const std::string getName() { return "HalfEdge"; };
};

}
} // end namespace
//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Externals
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// End Header file
//---------------------------------------------------------------------------
#endif
