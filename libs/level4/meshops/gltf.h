//
// Created by Computer on 26/07/2018.
//
#pragma once

#ifndef MESHOPS_GLTF_H
#define MESHOPS_GLTF_H

#include "meshmod/meshmod.h"
#include "meshmod/scenenode.h"
#include "meshmod/mesh.h"
#include <map>

namespace tinygltf
{
	class Node;
	class Model;
	struct Mesh;
}

namespace MeshOps {

class Gltf
{
public:
	static MeshMod::SceneNode::Ptr LoadAscii(std::string const& fileName_);
	static MeshMod::SceneNode::Ptr LoadBinary(std::string const& fileName_);

	static void SaveAscii(MeshMod::SceneNode::Ptr scene_, std::string const fileName_);
	static void SaveBinary(MeshMod::SceneNode::Ptr scene_, std::string const fileName_);

private:
	static MeshMod::SceneNode::Ptr LoadInternal(tinygltf::Model const& model);
	static void convertTransform(tinygltf::Node const &node_, MeshMod::SceneNode::Ptr &sceneNode_);
	static bool convertPositionData(std::map<uint32_t, uint32_t> const& attribMap, tinygltf::Model const& model, MeshMod::Mesh::Ptr& mesh);
	static void convertVertexData(std::map<uint32_t, uint32_t> const& attribMap, tinygltf::Model const& model, MeshMod::Mesh::Ptr& mesh);
	static void convertPrimitives(tinygltf::Model const& model, tinygltf::Mesh const& gltfMesh, MeshMod::Mesh::Ptr& mesh);

	static void SaveInternal(MeshMod::SceneNode::Ptr node_, tinygltf::Model& model_);

	static void convertTransform(MeshMod::SceneNode::Ptr const &sceneNode_, tinygltf::Node &node_);
	static bool convertPositionData(std::map<uint32_t, uint32_t> const& attribMap, std::vector<uint8_t>& dataBuffer, MeshMod::Mesh::Ptr const& mesh, tinygltf::Model & model);
	static void convertPrimitives(std::map<uint32_t, uint32_t> const& attribMap, std::vector<uint8_t>& dataBuffer, MeshMod::Mesh::Ptr const& mesh, tinygltf::Model & model, tinygltf::Mesh & gltfMesh);

};

}

#endif //CGEOMETRYENGINE_GLTF_H
