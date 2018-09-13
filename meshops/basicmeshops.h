/** \file basicmeshops.h
	
   (c) 2012 Dean Calver
 */

#if !defined( GOOPERATIONS_BASICMESHOPS_H_ )
#define GOOPERATIONS_BASICMESHOPS_H_

#include "meshmod/meshmod.h"
#include "meshmod/mesh.h"

namespace Core {
class AABB;
}
namespace MeshOps {
/**
	Basic Mesh Ops performs a bunch of common simple operations a mesh
*/
class BasicMeshOps
{
public:
	BasicMeshOps( MeshMod::MeshPtr& _mesh );

	//! compute a per-polygon plane equation
	void computeFacePlaneEquations( bool replaceExisting = true, bool zeroBad = false, bool fixBad = true );

	//! generates a basic vertex normal set. optionally replace any existing normals
	void computeVertexNormals( bool replaceExisting = true );

	//! conpute vertex normals handling bad cases better
	void computeVertexNormalsEx( bool replaceExisting = true, bool zeroBad = false, bool fixBad = true );

	//! turns simple convex n-gon to triangles, leaving points and lines along
	void triangulate();

	//! turns simple convex n-gon to triangles, leaving others (inlcuding quads) alone
	void quadOrTriangulate();

	//! computes and returns the axis aligned bounding box from the meshes positions
	static void computeAABB(MeshMod::Mesh const& mesh, Core::AABB& aabb );

	//! transform the mesh by a matrix
	void transform( Math::Matrix4x4 const& transform );

private:
	MeshMod::MeshPtr mesh;

	template<size_t n>
	void ngulate();

};
};


#endif