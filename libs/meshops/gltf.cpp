//
// Created by Computer on 26/07/2018.
//
#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION




#include "core/core.h"

#if PLATFORM == WINDOWS
// If using a modern Microsoft Compiler, this define supress compilation
// warnings in stb_image_write
#define STBI_MSC_SECURE_CRT
#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
#endif

#include "core/quick_hash.h"
#include "meshmod/meshmod.h"
#include "meshops/meshops.h"
#include "meshmod/vertices.h"
#include "meshmod/halfedges.h"
#include "meshmod/polygons.h"
#include "meshmod/mesh.h"
#include "meshmod/vertexdata/normalvertex.h"
#include "tinygltf/tiny_gltf.h"
#include "gltf.h"
#include <stack>
#include <string_view>

namespace MeshOps
{

MeshMod::SceneNode::Ptr Gltf::LoadAscii(std::string const& fileName_)
{
	using namespace tinygltf;
	using namespace MeshMod;

	Model model;
	TinyGLTF tinyGLTF;

	std::string err;
	bool okay = tinyGLTF.LoadASCIIFromFile(&model, &err, fileName_, REQUIRE_ALL);
	if(!okay)
	{
		printf("GLTF Loading Error : %s", err.c_str());
		return nullptr;
	}

	return LoadInternal(model);
}
MeshMod::SceneNode::Ptr Gltf::LoadBinary(std::string const& fileName_)
{
	using namespace tinygltf;
	using namespace MeshMod;

	Model model;
	TinyGLTF tinyGLTF;

	std::string err;
	bool okay = tinyGLTF.LoadBinaryFromFile(&model, &err, fileName_);
	if (!okay)
	{
		printf("GLTF Loading Error : %s", err.c_str());
		return nullptr;
	}

	return LoadInternal(model);
}

MeshMod::SceneNode::Ptr Gltf::LoadInternal(tinygltf::Model const& model)
{
	using namespace tinygltf;
	using namespace MeshMod;

	int sceneIndex = model.defaultScene;
	tinygltf::Scene const& gltfScene = model.scenes[sceneIndex];

	SceneNode::Ptr rootNode = std::make_shared<SceneNode>();

	for (int nodeIndex : gltfScene.nodes)
	{
		Node const& node = model.nodes[nodeIndex];
		SceneNode::Ptr sceneNode = std::make_shared<SceneNode>();
		rootNode->addChild(sceneNode);
		convertTransform(node, sceneNode);

		// TODO nodes other than meshes
		if (node.mesh != -1)
		{
			tinygltf::Mesh const& gltfMesh = model.meshes[node.mesh];
			MeshMod::Mesh::Ptr mesh = std::make_shared<MeshMod::Mesh>(gltfMesh.name);
			sceneNode->addObject(mesh);

			std::map<uint32_t, uint32_t> accessorAttributeMap;
			for (size_t i = 0; i < gltfMesh.primitives.size(); i++)
			{
				auto const& prim = gltfMesh.primitives[i];

				for (auto const& it : prim.attributes)
				{
					auto accessorNameHash = Core::QuickHash(it.first);

					auto const aamIt = accessorAttributeMap.find(accessorNameHash);
					if (aamIt != accessorAttributeMap.end())
					{
						//check they are the same
						if (aamIt->second != it.second)
						{
							printf("GLTF parser error: primitives attribute parse fail\n");
						}
					}
					else
					{
						accessorAttributeMap[accessorNameHash] = it.second;
					}
				}
			}

			bool posOkay = convertPositionData(accessorAttributeMap, model, mesh);
			if (posOkay)
			{
				convertVertexData(accessorAttributeMap, model, mesh);
				convertPrimitives(model, gltfMesh, mesh);
			}
		}
	}

	return rootNode;
}

void Gltf::convertTransform(tinygltf::Node const &node_, MeshMod::SceneNode::Ptr &sceneNode_)
{
	if (!node_.translation.empty())
	{
		sceneNode_->transform.position.x = (float)node_.translation[0];
		sceneNode_->transform.position.y = (float)node_.translation[1];
		sceneNode_->transform.position.z = (float)node_.translation[2];
	}
	if (!node_.scale.empty())
	{
		sceneNode_->transform.scale.x = (float)node_.scale[0];
		sceneNode_->transform.scale.y = (float)node_.scale[1];
		sceneNode_->transform.scale.z = (float)node_.scale[2];
	}
	if (!node_.rotation.empty())
	{
		sceneNode_->transform.orientation.x = (float)node_.rotation[0];
		sceneNode_->transform.orientation.y = (float)node_.rotation[1];
		sceneNode_->transform.orientation.z = (float)node_.rotation[2];
		sceneNode_->transform.orientation.w = (float)node_.rotation[3];
	}
}

bool Gltf::convertPositionData(std::map<uint32_t, uint32_t> const& attribMap, tinygltf::Model const& model, MeshMod::Mesh::Ptr& mesh)
{
	using namespace tinygltf;
	using namespace MeshMod;

	// we require position attribute and need to do it first
	assert(attribMap.find("POSITION"_hash) != attribMap.end());
	int posAttribIndex = attribMap.find("POSITION"_hash)->second;

	tinygltf::Accessor const& accessor = model.accessors[posAttribIndex];
	if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) return false;
	if (accessor.type != TINYGLTF_TYPE_VEC3) return false;
	if (accessor.ByteStride(model.bufferViews[accessor.bufferView]) != sizeof(float) * 3)
		return false;

	BufferView const& posBufferView = model.bufferViews[accessor.bufferView];
	Buffer const& posBuffer = model.buffers[posBufferView.buffer];

	auto& vertices =  mesh->getVertices();

	for (uint64_t i = 0; i < accessor.count; ++i)
	{
		uint64_t offset = posBufferView.byteOffset + (i * sizeof(float) * 3);
		float* posData = (float*)(posBuffer.data.data() + offset);
		vertices.add(posData[0], posData[1], posData[2]);
	}

	return true;
}



void Gltf::convertVertexData(std::map<uint32_t, uint32_t> const& attribMap, tinygltf::Model const& model, MeshMod::Mesh::Ptr& mesh)
{
	using namespace tinygltf;
	using namespace MeshMod;

	for(auto const& it : attribMap)
	{
		auto const &accessor = model.accessors[it.second];
		// TODO generalise this
		bool vec2Data = false;
		switch(it.first)
		{
			case "POSITION"_hash: // skip already done
				return;
			case "NORMAL"_hash:
				break;
			case "TEXCOORD_0"_hash:
				vec2Data = true;
				break;
			case "TEXCOORD_1"_hash:
				vec2Data = true;
				break;
			default:
				printf("Unknown vertex attribute %s", accessor.name.c_str());
		}

		if(vec2Data)
		{
			tinygltf::BufferView const& bufferView = model.bufferViews[accessor.bufferView];
			VerticesElementsContainer& vertCon = mesh->getVertices().getVerticesContainer();

			// skip attrib if not vec2
			if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) break;
			if (accessor.type != TINYGLTF_TYPE_VEC2) break;
			if(accessor.ByteStride(bufferView) != sizeof(float) * 2) break;

			std::string elementSubName = "";
			Buffer const &buffer = model.buffers[bufferView.buffer];
			switch(it.first)
			{
				case "TEXCOORD_0"_hash:
				case "TEXCOORD_1"_hash:
				{
					if(it.first == "TEXCOORD_0"_hash) elementSubName = "0";
					else elementSubName = "1";
					VertexData::UVs& uvEle = *vertCon.getOrAddElements<VertexData::UVs>(elementSubName);
					for(size_t i = 0; i < accessor.count; ++i)
					{
						uint64_t offset = bufferView.byteOffset + (i * sizeof(float) * 2);
						float *data = (float *) (buffer.data.data() + offset);
						uvEle[i].u = data[0];
						uvEle[i].v = data[1];
					}
					break;
				}
			}

		} else
		{
			tinygltf::BufferView const& bufferView = model.bufferViews[accessor.bufferView];
			VerticesElementsContainer& vertCon = mesh->getVertices().getVerticesContainer();

			// skip attrib if not vec3
			if(accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) break;
			if(accessor.type != TINYGLTF_TYPE_VEC3) break;
			if(accessor.ByteStride(bufferView) != sizeof(float) * 3) break;

			Buffer const &buffer = model.buffers[bufferView.buffer];
			switch(it.first)
			{
				case "NORMAL"_hash:
				{
					VertexData::Normals& nvEle = *vertCon.getOrAddElements<VertexData::Normals>();
					for(size_t i = 0; i < accessor.count; ++i)
					{
						uint64_t offset = bufferView.byteOffset + (i * sizeof(float) * 3);
						float *data = (float *) (buffer.data.data() + offset);
						nvEle[i].x = data[0];
						nvEle[i].y = data[1];
						nvEle[i].z = data[2];
					}
					break;
				}
			}
		}
	}
}

void Gltf::convertPrimitives(tinygltf::Model const& model, tinygltf::Mesh const& gltfMesh, MeshMod::Mesh::Ptr& mesh)
{
	using namespace tinygltf;
	for (size_t i = 0; i < gltfMesh.primitives.size(); i++)
	{
		Primitive const& primitive = gltfMesh.primitives[i];
		auto const &accessor = model.accessors[primitive.indices];
		BufferView const& bufferView = model.bufferViews[accessor.bufferView];
		Buffer const &buffer = model.buffers[bufferView.buffer];

		int sizeOfIndex = sizeof(uint32_t);
		if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			sizeOfIndex = sizeof(uint16_t);

		// TODO other primitive modes
		if (primitive.mode != TINYGLTF_MODE_TRIANGLES) break;

		for (uint64_t i = 0; i < accessor.count/3; ++i)
		{
			MeshMod::VertexIndexContainer indices;

			uint64_t offset = bufferView.byteOffset + (i * sizeOfIndex * 3);
			if (sizeOfIndex == sizeof(uint16_t))
			{
				uint16_t *data = (uint16_t*)(buffer.data.data() + offset);
				indices.push_back((MeshMod::VertexIndex)data[0]);
				indices.push_back((MeshMod::VertexIndex)data[2]);
				indices.push_back((MeshMod::VertexIndex)data[1]);
			}
			else
			{
				uint32_t *data = (uint32_t*)(buffer.data.data() + offset);
				indices.push_back((MeshMod::VertexIndex)data[0]);
				indices.push_back((MeshMod::VertexIndex)data[2]);
				indices.push_back((MeshMod::VertexIndex)data[1]);
			}
			mesh->getPolygons().add(indices);
		}
	}

	mesh->updateEditState(MeshMod::Mesh::TopologyEdits);
	mesh->updateFromEdits();
}

void Gltf::SaveAscii(MeshMod::SceneNode::Ptr mesh_, std::string const fileName_){
	std::string output_filename(fileName_);
	std::string embedded_filename =
			output_filename.substr(0, output_filename.size() - 5) + "-Embedded.gltf";

	// skip root node if empty
	MeshMod::SceneNode::Ptr rootNode = mesh_;
	if (rootNode->getObjectCount() == 0 && rootNode->getChildCount() == 1)
	{
		// now see if the transform is identical
		if (rootNode->transform.isIdentity())
		{
			rootNode = rootNode->getChild(0);
		}
	}

	tinygltf::Model model;
	SaveInternal(rootNode, model);
	tinygltf::TinyGLTF loader;

	if (model.buffers.size() == 1 && model.buffers[0].data.size() == 0)
	{
		printf("No data to save, this crashes tiny GLTF, so aborting\n");
		return;
	}

	loader.WriteGltfSceneToFile(&model, output_filename);
//	loader.WriteGltfSceneToFile(&model, embedded_filename, true, true);
}

void Gltf::convertTransform(MeshMod::SceneNode::Ptr const& sceneNode_, tinygltf::Node &node_ )
{
	node_.translation.resize(3);
	node_.translation[0] = sceneNode_->transform.position.x;
	node_.translation[1] = sceneNode_->transform.position.y;
	node_.translation[2] = sceneNode_->transform.position.z;

	node_.scale.resize(3);
	node_.scale[0] = sceneNode_->transform.scale.x;
	node_.scale[1] = sceneNode_->transform.scale.y;
	node_.scale[2] = sceneNode_->transform.scale.z;

	node_.rotation.resize(4);

	node_.rotation[0] = sceneNode_->transform.orientation.x;
	node_.rotation[1] = sceneNode_->transform.orientation.y;
	node_.rotation[2] = sceneNode_->transform.orientation.z;
	node_.rotation[3] = sceneNode_->transform.orientation.w;
}
bool Gltf::convertPositionData(std::map<uint32_t, uint32_t> const& attribMap, 
	std::vector<uint8_t>& dataBuffer, 
	MeshMod::Mesh::Ptr const& mesh,
	tinygltf::Model& model)
{
	using namespace tinygltf;
	using namespace MeshMod;

	VerticesElementsContainer& vertCon = mesh->getVertices().getVerticesContainer();
	VertexData::Positions const& posEle = *vertCon.getElements<VertexData::Positions>();

	assert(attribMap.find("POSITION"_hash) != attribMap.end());
	int attribIndex = attribMap.find("POSITION"_hash)->second;
	tinygltf::Accessor& accessor = model.accessors[attribIndex];

	accessor.byteOffset = 0;
	accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accessor.type = TINYGLTF_TYPE_VEC3;
	accessor.count = posEle.size();

	auto const dataBufferSize = dataBuffer.size();
	dataBuffer.resize(dataBufferSize + accessor.count * sizeof(float) * 3);
	for (auto i = 0u; i < accessor.count; ++i)
	{
		Math::Vector3 pos = posEle[i].getVector3();
		memcpy(dataBuffer.data() + dataBufferSize + i * sizeof(float) * 3, &pos.x, sizeof(float) * 3);
	}	

	auto bufferViewIndex = model.bufferViews.size();
	model.bufferViews.resize(bufferViewIndex + 1);
	accessor.bufferView = (int)bufferViewIndex;
	BufferView & bufferView = model.bufferViews[accessor.bufferView];
	bufferView.buffer = 0;
	bufferView.byteLength = accessor.count * sizeof(float) * 3;
	bufferView.byteOffset = dataBufferSize;
	bufferView.byteStride = sizeof(float) * 3;
	bufferView.name = "POSITION";

	return true;
}
void Gltf::convertPrimitives(std::map<uint32_t, uint32_t> const& attribMap, 
	std::vector<uint8_t>& dataBuffer, 
	MeshMod::Mesh::Ptr const& mesh,
	tinygltf::Model & model, 
	tinygltf::Mesh & gltfMesh)
{
	using namespace tinygltf;
	using namespace MeshMod;
	auto& polygons = mesh->getPolygons();
	PolygonData::Polygons const& faceEle = polygons.polygons();

	assert(attribMap.find("INDICIES"_hash) != attribMap.end());
	int attribIndex = attribMap.find("INDICIES"_hash)->second;
	tinygltf::Accessor& accessor = model.accessors[attribIndex];
	
	VertexIndexContainer facesVertexIndices;
	facesVertexIndices.reserve(faceEle.size());

	VertexIndexContainer swapperfacesVertexIndices;
	swapperfacesVertexIndices.reserve(10);

	for( auto faceIndex = 0u; faceIndex < faceEle.size(); ++faceIndex)
	{
		swapperfacesVertexIndices.clear();
		polygons.getVertexIndices(faceIndex, swapperfacesVertexIndices);
		if (swapperfacesVertexIndices.size() == 3)
		{
			facesVertexIndices.push_back(swapperfacesVertexIndices[0]);
			facesVertexIndices.push_back(swapperfacesVertexIndices[2]);
			facesVertexIndices.push_back(swapperfacesVertexIndices[1]);
		}
		else
		{
			static bool warnedAboutPrims = false;
			if (warnedAboutPrims == false)
			{
				printf("GLTF saving supports triangles only currently, ignoring other primitives\n");
				warnedAboutPrims = true;
			}
		}
	}

	PolygonIndex maxIndex = 0;
	for (auto const& fi : facesVertexIndices) { maxIndex = std::max(maxIndex, fi); }

	auto const dataBufferSize = dataBuffer.size();

	accessor.count = facesVertexIndices.size();
	accessor.byteOffset = dataBufferSize;
	accessor.type = TINYGLTF_TYPE_SCALAR;

	size_t indexBufferSize;
	if (maxIndex >= (1 << 16))
	{
		indexBufferSize = accessor.count * sizeof(uint32_t);
		dataBuffer.resize(dataBufferSize + indexBufferSize);
		memcpy(dataBuffer.data() + dataBufferSize, facesVertexIndices.data(), indexBufferSize);
		accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
	}
	else
	{
		indexBufferSize = accessor.count * sizeof(uint16_t);
		dataBuffer.resize(dataBufferSize + indexBufferSize);
		for (auto i = 0u; i < accessor.count; ++i)
		{
			uint16_t index = (uint16_t) facesVertexIndices[i];
			memcpy(dataBuffer.data() + dataBufferSize + i * sizeof(uint16_t), &index, sizeof(uint16_t));
		}
		accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
	}

	auto bufferViewIndex = model.bufferViews.size();
	model.bufferViews.resize(bufferViewIndex + 1);
	accessor.bufferView = (int)bufferViewIndex;

	BufferView & bufferView = model.bufferViews[accessor.bufferView];
	bufferView.buffer = 0;
	bufferView.byteLength = indexBufferSize;
	bufferView.byteOffset = dataBufferSize;
	bufferView.byteStride = maxIndex >= (1 << 16) ? sizeof(uint32_t) : sizeof(uint16_t);
	bufferView.name = "INDICES";

	gltfMesh.primitives.resize(1);
	Primitive& primitive = gltfMesh.primitives[0];
	primitive.indices = attribIndex;
	primitive.mode = TINYGLTF_MODE_TRIANGLES;
	primitive.attributes["POSITION"] = attribMap.find("POSITION"_hash)->second;

}

void Gltf::SaveInternal(MeshMod::SceneNode::Ptr node_, tinygltf::Model& model_)
{
	using namespace tinygltf;
	using namespace MeshMod;
	using namespace std::string_view_literals;

	tinygltf::Scene gltfScene;
	static const uint32_t meshTypeHash = Core::QuickHash("Mesh"sv);

	std::vector<uint8_t> dataBuffer;
	std::stack<MeshMod::SceneNode::Ptr> nodeStack;
	nodeStack.push(node_);

	std::map<uint32_t, uint32_t> accessorAttributeMap;
	model_.accessors.resize(2); // TODO more than just positions and indices
	accessorAttributeMap["INDICIES"_hash] = 0;
	accessorAttributeMap["POSITION"_hash] = 1;
	while (!nodeStack.empty())
	{
		MeshMod::SceneNode::Ptr node = nodeStack.top(); nodeStack.pop();
		int nodeIndex = (int)model_.nodes.size();
		model_.nodes.resize(nodeIndex + 1);
		gltfScene.nodes.push_back(nodeIndex);
		tinygltf::Node& gltfNode = model_.nodes[nodeIndex];
		convertTransform(node, gltfNode);

		for (auto i = 0u; i < node->getObjectCount(); ++i)
		{
			auto const& obj = node->getObject(i);
			if (Core::QuickHash(obj->getType()) == meshTypeHash)
			{
				auto mesh = std::dynamic_pointer_cast<MeshMod::Mesh>(obj);
				int meshIndex = (int)model_.meshes.size();
				model_.meshes.resize(meshIndex + 1);
				// TODO a node in MeshMod can have multiple objects attached gltf can't
				gltfNode.mesh = meshIndex;
				tinygltf::Mesh& gltfMesh = model_.meshes[meshIndex];
				gltfMesh.name = mesh->getName();
				convertPositionData(accessorAttributeMap, dataBuffer, mesh, model_);
				convertPrimitives(accessorAttributeMap, dataBuffer, mesh, model_, gltfMesh);
				break;
			}
		}

		for (auto i = 0u; i < node->getChildCount(); ++i)
		{
			nodeStack.push(node->getChild(i));
		}
	}

	model_.defaultScene = 0;
	model_.scenes.emplace_back(gltfScene);
	model_.buffers.emplace_back(tinygltf::Buffer {
		"",
		dataBuffer
		});
}

}
