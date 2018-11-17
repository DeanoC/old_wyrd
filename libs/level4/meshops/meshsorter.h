#pragma once
#ifndef WYRD_MESHOPS_MESHSORTER_H_
#define WYRD_MESHOPS_MESHSORTER_H_

#include "core/core.h"
#include "meshmod/polygonsdata/polygoncontainers.h"
#include "meshmod/vertexdata/vertexcontainers.h"
#include "meshmod/vertexdata/positionvertex.h"

namespace MeshMod {
class Mesh;
}

namespace MeshOps {
	/**
		MeshSorter performs a variety of non destructive sorts on a mesh
	*/
	class MeshSorter {
	public:

		MeshSorter(std::shared_ptr<MeshMod::Mesh> const& _mesh);

		std::shared_ptr<MeshMod::PolygonData::SortIndices> sortPolygonsByMaterialIndex();
		std::shared_ptr<MeshMod::VertexData::SortIndices> sortVerticesByAxis(MeshMod::VertexData::Axis axis);

	private:
		std::shared_ptr<MeshMod::Mesh> mesh;
	};
};

#endif