//
// Created by Computer on 26/07/2018.
//
#include "core/core.h"

#if PLATFORM == WINDOWS
// If using a modern Microsoft Compiler, this define supress compilation
// warnings in stb_image_write
#define STBI_MSC_SECURE_CRT
#endif

#include "core/quick_hash.h"
#include "meshmod/meshmod.h"
#include "meshmod/vertices.h"
#include "meshmod/halfedges.h"
#include "meshmod/polygons.h"
#include "meshmod/mesh.h"
#include "meshmod/vertexdata/normalvertex.h"
#include "meshmod/vertexdata/uvvertex.h"
#include "tinygltf/tiny_gltf.h"
#include "gltf.h"
#include <stack>
#include <string_view>
#include <unordered_set>

namespace MeshOps {

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
	if(!okay)
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

	std::stack<int> nodeStack;
	for(int nodeIndex : gltfScene.nodes) nodeStack.push(nodeIndex);

	do
	{
		int nodeIndex = nodeStack.top();
		nodeStack.pop();

		Node const& node = model.nodes[nodeIndex];
		SceneNode::Ptr sceneNode = std::make_shared<SceneNode>();
		rootNode->addChild(sceneNode);
		convertTransform(node, sceneNode);

		for(int childIndex : node.children) nodeStack.push(childIndex);

		// TODO nodes other than meshes
		if(node.mesh != -1)
		{
			tinygltf::Mesh const& gltfMesh = model.meshes[node.mesh];
			auto mesh = std::make_shared<MeshMod::Mesh>(gltfMesh.name, false, false);
			sceneNode->addObject(mesh);

			convertPrimitives(model, gltfMesh, mesh);
		}
	} while(!nodeStack.empty());

	return rootNode;
}

void Gltf::convertTransform(tinygltf::Node const& node_, MeshMod::SceneNode::Ptr& sceneNode_)
{
	if(!node_.translation.empty())
	{
		sceneNode_->transform.position.x = (float) node_.translation[0];
		sceneNode_->transform.position.y = (float) node_.translation[1];
		sceneNode_->transform.position.z = (float) node_.translation[2];
	}
	if(!node_.scale.empty())
	{
		sceneNode_->transform.scale.x = (float) node_.scale[0];
		sceneNode_->transform.scale.y = (float) node_.scale[1];
		sceneNode_->transform.scale.z = (float) node_.scale[2];
	}
	if(!node_.rotation.empty())
	{
		sceneNode_->transform.orientation.x = (float) node_.rotation[0];
		sceneNode_->transform.orientation.y = (float) node_.rotation[1];
		sceneNode_->transform.orientation.z = (float) node_.rotation[2];
		sceneNode_->transform.orientation.w = (float) node_.rotation[3];
	}
}

void Gltf::convertPositionData(int posAttribIndex, tinygltf::Model const& model_,
							   std::shared_ptr<MeshMod::Mesh>& mesh_)
{
	using namespace tinygltf;
	using namespace MeshMod;

	tinygltf::Accessor const& accessor = model_.accessors[posAttribIndex];
	BufferView const& posBufferView = model_.bufferViews[accessor.bufferView];
	Buffer const& posBuffer = model_.buffers[posBufferView.buffer];

	auto const accessorByteStride = accessor.ByteStride(posBufferView);

	if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
	{
		LOG_F(WARNING, "Vertex buffer position component type %u not supported", accessor.componentType);
		return;
	}
	if (accessor.type != TINYGLTF_TYPE_VEC3)
	{
		LOG_F(WARNING, "Vertex buffer position type %u not supported", accessor.type);
		return;
	}

	if (accessorByteStride != sizeof(float) * 3)
	{
		LOG_F(WARNING, "Vertex buffer position stride %u not supported", accessorByteStride);
		return;
	}

	if (posBufferView.target != 0 && posBufferView.target != TINYGLTF_TARGET_ARRAY_BUFFER)
	{
		LOG_F(WARNING, "Vertex buffer position view target %u not supported", posBufferView.target);
		return;
	}

	auto& vertices = mesh_->getVertices();

	float* posData = (float*) (posBuffer.data.data() + posBufferView.byteOffset + accessor.byteOffset);
	for(uint64_t i = 0; i < accessor.count; ++i)
	{
		vertices.add(posData[0], posData[1], posData[2]);
		posData += 3;
	}
}

auto Gltf::convertVertexData(tinygltf::Primitive const& prim_, tinygltf::Model const& model_,
							 std::shared_ptr<MeshMod::Mesh>& mesh_) -> bool
{
	using namespace tinygltf;
	using namespace MeshMod;
	using namespace std::literals;

	auto& vertices = mesh_->getVertices();

	// do position element first for meshmod
	auto const posIt = prim_.attributes.find("POSITION"s);
	if(posIt == prim_.attributes.cend()) return false;

	convertPositionData(posIt->second, model_, mesh_);
	for(auto const[name, index] : prim_.attributes)
	{
		auto nameHash = Core::QuickHash(name);

		// TODO generalise this
		bool vec2Data = false;
		switch(nameHash)
		{
			case "POSITION"_hash: // skip as already done
				continue;
			default:
				LOG_S(WARNING) <<
							   "Unknown vertex attribute " << name <<
							   " on " << mesh_->getName();
				continue;

			case "NORMAL"_hash:
				break;
			case "TEXCOORD_0"_hash:
				[[fallthrough]];
			case "TEXCOORD_1"_hash:
				vec2Data = true;
				break;
		}

		tinygltf::Accessor const& accessor = model_.accessors[index];
		tinygltf::BufferView const& bufferView = model_.bufferViews[accessor.bufferView];
		Buffer const& buffer = model_.buffers[bufferView.buffer];
		if (bufferView.target != 0 && bufferView.target != TINYGLTF_TARGET_ARRAY_BUFFER)
		{
			LOG_F(WARNING, "Vertex buffer element %s view target %u not supported", name.c_str(), bufferView.target);
			break;
		}
		float* data = (float*) (buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);
		auto const accessorByteStride = accessor.ByteStride(bufferView);

		if(vec2Data)
		{
			// skip attrib if not vec2
			if(accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) continue;
			if(accessor.type != TINYGLTF_TYPE_VEC2) continue;
			if(accessorByteStride != sizeof(float) * 2) continue;

			switch(nameHash)
			{
				case "TEXCOORD_0"_hash:
					[[fallthrough]];
				case "TEXCOORD_1"_hash:
				{
					auto const elementSubName =
							(nameHash == "TEXCOORD_0"_hash) ? "0"s : "1"s;

					auto& uvs = vertices.getOrAddAttribute<VertexData::UVs>(elementSubName);
					for(auto j = 0u; j < accessor.count; ++j)
					{
						VertexIndex dstIndex = VertexIndex((accessor.byteOffset / accessorByteStride) + j);
						uvs[dstIndex].u = data[0];
						uvs[dstIndex].v = data[1];
						data += 2;
					}
					break;
				}
				default:
					LOG_S(WARNING) <<
								   "Unknown vec2 vertex attribute " << name <<
								   " on " << mesh_->getName();
					break;
			}
		} else
		{
			// skip attrib if not vec3
			if(accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) continue;
			if(accessor.type != TINYGLTF_TYPE_VEC3) continue;
			if(accessor.ByteStride(bufferView) != sizeof(float) * 3) continue;

			switch(nameHash)
			{
				case "NORMAL"_hash:
				{
					auto& normals = vertices.getOrAddAttribute<VertexData::Normals>();
					for(auto j = 0; j < accessor.count; ++j)
					{
						VertexIndex dstIndex = VertexIndex(((accessor.byteOffset) / accessorByteStride) + j);
						normals[dstIndex].x = data[0];
						normals[dstIndex].y = data[1];
						normals[dstIndex].z = data[2];
						data += 3;
					}
					break;
				}
				default:
					LOG_S(WARNING) <<
								   "Unknown vec3 vertex attribute " << name <<
								   " on " << mesh_->getName();
					break;
			}
		}
	}
	return true;
}

void Gltf::convertPrimitives(tinygltf::Model const& model_,
							 tinygltf::Mesh const& gltfMesh_,
							 std::shared_ptr<MeshMod::Mesh>& mesh_)
{
	using namespace MeshMod;
	using namespace tinygltf;

	auto& vertices = mesh_->getVertices();

	for(size_t i = 0; i < gltfMesh_.primitives.size(); i++)
	{
		Primitive const& primitive = gltfMesh_.primitives[i];

		VertexIndex baseIndex = VertexIndex(vertices.getCount());
		if(convertVertexData(primitive, model_, mesh_) == false) continue;

		auto const& accessor = model_.accessors[primitive.indices];
		BufferView const& bufferView = model_.bufferViews[accessor.bufferView];
		if (bufferView.target != 0 && bufferView.target != TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER)
		{
			LOG_F(WARNING, "Primitive buffer view target %u not supported", bufferView.target);
			continue;
		}
		Buffer const& buffer = model_.buffers[bufferView.buffer];

		int sizeOfIndex = sizeof(uint32_t);
		if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			sizeOfIndex = sizeof(uint16_t);

		VertexIndexContainer indices(accessor.count);

		assert(bufferView.byteLength == accessor.count * sizeOfIndex);
		if(sizeOfIndex == sizeof(uint16_t))
		{
			uint16_t* data = (uint16_t*) (buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);
			for(uint64_t j = 0; j < accessor.count; ++j)
			{
				if (!accessor.maxValues.empty())
				{
					assert(data[j] <= accessor.maxValues[0]);
				}
				indices[j] = (VertexIndex) data[j];
			}
		} else
		{
			uint32_t* data = (uint32_t*) (buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);
			for(uint64_t j = 0; j < accessor.count; ++j)
			{
				if (!accessor.maxValues.empty())
				{
					assert(data[j] <= accessor.maxValues[0]);
				}
				indices[j] = (VertexIndex) data[j];
			}
		}

		// TODO other primitive modes
		if(primitive.mode != TINYGLTF_MODE_TRIANGLES)
		{
			LOG_F(WARNING, "Primitive mode %u not supported", primitive.mode);
			continue;
		}

		mesh_->getPolygons().addTriangles(indices, baseIndex);
	}

	mesh_->updateEditState(MeshMod::Mesh::TopologyEdits);
	mesh_->updateFromEdits();
}

void Gltf::SaveAscii(MeshMod::SceneNode::Ptr mesh_, std::string const fileName_)
{
	std::string output_filename(fileName_);
	std::string embedded_filename =
			output_filename.substr(0, output_filename.size() - 5) + "-Embedded.gltf";

	// skip root node if empty
	MeshMod::SceneNode::Ptr rootNode = mesh_;
	if(rootNode->getObjectCount() == 0 && rootNode->getChildCount() == 1)
	{
		// now see if the transform is identical
		if(rootNode->transform.isIdentity())
		{
			rootNode = rootNode->getChild(0);
		}
	}

	tinygltf::Model model;
	SaveInternal(rootNode, model);
	tinygltf::TinyGLTF loader;

	if(model.buffers.size() == 1 && model.buffers[0].data.size() == 0)
	{
		printf("No data to save, this crashes tiny GLTF, so aborting\n");
		return;
	}

	loader.WriteGltfSceneToFile(&model, output_filename, true, true);
}

void Gltf::convertTransform(MeshMod::SceneNode::Ptr const& sceneNode_, tinygltf::Node& node_)
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
							   std::shared_ptr<MeshMod::Mesh> const& mesh,
							   tinygltf::Model& model)
{
	using namespace tinygltf;
	using namespace MeshMod;

	VerticesElementsContainer& vertCon = mesh->getVertices().getVerticesContainer();
	VertexData::Positions const& posEle = *vertCon.getElement<VertexData::Positions>();

	assert(attribMap.find("POSITION"_hash) != attribMap.end());
	int attribIndex = attribMap.find("POSITION"_hash)->second;
	tinygltf::Accessor& accessor = model.accessors[attribIndex];

	accessor.byteOffset = 0;
	accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accessor.type = TINYGLTF_TYPE_VEC3;
	accessor.count = posEle.size();

	auto const dataBufferSize = dataBuffer.size();
	dataBuffer.resize(dataBufferSize + accessor.count * sizeof(float) * 3);
	for(auto i = 0u; i < accessor.count; ++i)
	{
		Math::vec3 pos = posEle[VertexIndex(i)].getVec3();
		memcpy(dataBuffer.data() + dataBufferSize + i * sizeof(float) * 3, &pos.x, sizeof(float) * 3);
	}

	auto bufferViewIndex = model.bufferViews.size();
	model.bufferViews.resize(bufferViewIndex + 1);
	accessor.bufferView = (int) bufferViewIndex;
	BufferView& bufferView = model.bufferViews[accessor.bufferView];
	bufferView.buffer = 0;
	bufferView.byteLength = accessor.count * sizeof(float) * 3;
	bufferView.byteOffset = dataBufferSize;
	bufferView.byteStride = sizeof(float) * 3;
	bufferView.name = "POSITION";

	return true;
}

void Gltf::convertPrimitives(std::map<uint32_t, uint32_t> const& attribMap,
							 std::vector<uint8_t>& dataBuffer,
							 std::shared_ptr<MeshMod::Mesh> const& mesh,
							 tinygltf::Model& model,
							 tinygltf::Mesh& gltfMesh)
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

	for(auto i = 0u; i < faceEle.size(); ++i)
	{
		swapperfacesVertexIndices.clear();
		polygons.getVertexIndices(PolygonIndex(i), swapperfacesVertexIndices);
		if(swapperfacesVertexIndices.size() == 3)
		{
			facesVertexIndices.push_back(swapperfacesVertexIndices[0]);
			facesVertexIndices.push_back(swapperfacesVertexIndices[2]);
			facesVertexIndices.push_back(swapperfacesVertexIndices[1]);
		} else
		{
			static bool warnedAboutPrims = false;
			if(warnedAboutPrims == false)
			{
				printf("GLTF saving supports triangles only currently, ignoring other primitives\n");
				warnedAboutPrims = true;
			}
		}
	}

	size_t maxIndex{0};
	for(auto const& fi : facesVertexIndices) { maxIndex = std::max(maxIndex, size_t(fi)); }

	auto const dataBufferSize = dataBuffer.size();

	accessor.count = facesVertexIndices.size();
	accessor.byteOffset = 0;
	accessor.type = TINYGLTF_TYPE_SCALAR;

	size_t indexBufferSize;
	if(maxIndex >= (1 << 16))
	{
		indexBufferSize = accessor.count * sizeof(uint32_t);
		dataBuffer.resize(dataBufferSize + indexBufferSize);
		memcpy(dataBuffer.data() + dataBufferSize, facesVertexIndices.data(), indexBufferSize);
		accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
	} else
	{
		indexBufferSize = accessor.count * sizeof(uint16_t);
		dataBuffer.resize(dataBufferSize + indexBufferSize);
		for(auto i = 0u; i < accessor.count; ++i)
		{
			uint16_t index = (uint16_t) facesVertexIndices[i];
			memcpy(dataBuffer.data() + dataBufferSize + i * sizeof(uint16_t), &index, sizeof(uint16_t));
		}
		accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
	}

	auto bufferViewIndex = model.bufferViews.size();
	model.bufferViews.resize(bufferViewIndex + 1);
	accessor.bufferView = (int) bufferViewIndex;

	BufferView& bufferView = model.bufferViews[accessor.bufferView];
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
	while(!nodeStack.empty())
	{
		MeshMod::SceneNode::Ptr node = nodeStack.top();
		nodeStack.pop();
		int nodeIndex = (int) model_.nodes.size();
		model_.nodes.resize(nodeIndex + 1);
		gltfScene.nodes.push_back(nodeIndex);
		tinygltf::Node& gltfNode = model_.nodes[nodeIndex];
		convertTransform(node, gltfNode);

		for(auto i = 0u; i < node->getObjectCount(); ++i)
		{
			auto const& obj = node->getObject(i);
			if(Core::QuickHash(obj->getType()) == meshTypeHash)
			{
				auto mesh = std::dynamic_pointer_cast<MeshMod::Mesh>(obj);
				int meshIndex = (int) model_.meshes.size();
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

		for(auto i = 0u; i < node->getChildCount(); ++i)
		{
			nodeStack.push(node->getChild(i));
		}
	}

	model_.defaultScene = 0;
	model_.scenes.emplace_back(gltfScene);
	model_.buffers.emplace_back(tinygltf::Buffer{
			"",
			dataBuffer
	});
}

}
