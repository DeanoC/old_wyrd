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
	static auto computeFacePlaneEquations(std::shared_ptr<MeshMod::Mesh> const& mesh, bool replaceExisting = true, bool zeroBad = false, bool fixBad = true ) -> void;

	//! generates a basic vertex normal set. optionally replace any existing normals
	static auto computeVertexNormals(std::shared_ptr<MeshMod::Mesh> const& mesh, bool replaceExisting = true ) -> void;

	//! conpute vertex normals handling bad cases better
	static auto computeVertexNormalsEx(std::shared_ptr<MeshMod::Mesh> const& mesh, bool replaceExisting = true, bool zeroBad = false, bool fixBad = true ) -> void;

	//! turns simple convex n-gon to triangles, leaving points and lines along
	static auto triangulate(std::shared_ptr<MeshMod::Mesh> const& mesh) -> void;

	//! turns simple convex n-gon to triangles, leaving others (inlcuding quads) alone
	static auto quadOrTriangulate(std::shared_ptr<MeshMod::Mesh> const& mesh) -> void;

	//! computes and returns the axis aligned bounding box from the meshes positions
	static auto computeAABB(std::shared_ptr<MeshMod::Mesh const> const& mesh, Geometry::AABB& aabb ) -> void;

	//! transform the mesh by a matrix
	static auto transform(std::shared_ptr<MeshMod::Mesh> const& mesh, Math::mat4x4 const& transform ) -> void;

	// currently only position and polygons are combined
	static auto combine(std::shared_ptr<MeshMod::Mesh const> const& src_,
						  std::shared_ptr<MeshMod::Mesh> const& dst_) -> void;

	static auto isTriangleMesh(std::shared_ptr<MeshMod::Mesh const> const& mesh_) -> bool;

	// takes a triangle mesh and generate a mesh with 4 output triangles per input triangle
	static auto tesselate4(std::shared_ptr<MeshMod::Mesh const> const& mesh_) -> std::unique_ptr<MeshMod::Mesh>;

	static auto spherize(std::shared_ptr<MeshMod::Mesh> const& mesh_, float t_) -> void;

private:
	template<size_t n>
	static auto ngulate(std::shared_ptr<MeshMod::Mesh> const& mesh) -> void;

};
};


#endif