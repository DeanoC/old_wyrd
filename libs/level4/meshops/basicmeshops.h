/** \file basicmeshops.h
	
   (c) 2012 Dean Calver
 */

#if !defined( GOOPERATIONS_BASICMESHOPS_H_ )
#define GOOPERATIONS_BASICMESHOPS_H_

#include "meshmod/meshmod.h"
#include "meshmod/mesh.h"

namespace Geometry {
class AABB;
}
namespace MeshOps {
/**
	Basic Mesh Ops performs a bunch of common simple operations a mesh
*/
class BasicMeshOps
{
public:

	//! compute a per-polygon plane equation
	static auto computeFacePlaneEquations(MeshMod::Mesh::Ptr const& mesh, bool replaceExisting = true, bool zeroBad = false, bool fixBad = true ) -> void;

	//! generates a basic vertex normal set. optionally replace any existing normals
	static auto computeVertexNormals(MeshMod::Mesh::Ptr const& mesh, bool replaceExisting = true ) -> void;

	//! conpute vertex normals handling bad cases better
	static auto computeVertexNormalsEx(MeshMod::Mesh::Ptr const& mesh, bool replaceExisting = true, bool zeroBad = false, bool fixBad = true ) -> void;

	//! turns simple convex n-gon to triangles, leaving points and lines along
	static auto triangulate(MeshMod::Mesh::Ptr const& mesh) -> void;

	//! turns simple convex n-gon to triangles, leaving others (inlcuding quads) alone
	static auto quadOrTriangulate(MeshMod::Mesh::Ptr const& mesh) -> void;

	//! computes and returns the axis aligned bounding box from the meshes positions
	static auto computeAABB(MeshMod::Mesh::ConstPtr const& mesh, Geometry::AABB& aabb ) -> void;

	//! transform the mesh by a matrix
	static auto transform(MeshMod::Mesh::Ptr const& mesh, Math::mat4x4 const& transform ) -> void;

private:
	template<size_t n>
	static auto ngulate(MeshMod::Mesh::Ptr const& mesh) -> void;

};
};


#endif