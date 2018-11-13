/** \file meshsorter.h
	
   (c) 2012 Dean Calver
 */

#include <meshmod/vertexdata/vertexcontainers.h>

#if !defined( MESHOPS_MESHSORTER_H_ )
#define MESHOPS_MESHSORTER_H_


namespace MeshOps {
	/**
		MeshSorter performs a variety of non destructive sorts on a mesh
	*/
	class MeshSorter {
	public:

		MeshSorter( const MeshMod::Mesh::Ptr& _mesh );

		std::shared_ptr<MeshMod::PolygonData::SortIndices> sortPolygonsByMaterialIndex();
		std::shared_ptr<MeshMod::VertexData::SortIndices> sortVerticesByAxis( MeshMod::VertexData::Position::AXIS axis );

	private:
		const MeshMod::Mesh::Ptr	mesh;
	};
};

#endif