#define LOGURU_IMPLEMENTATION 1
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION

#include "core/core.h"
#include "crc32c/crc32c.h"
#include "core/quick_hash.h"
#include "tinygltf/tiny_gltf.h"
#include "meshmod/meshmod.h"
#include "cgeometryengine.h"

#include "meshmod/vertices.h"
#include "meshmod/halfedges.h"
#include "meshmod/polygons.h"
#include "meshmod/mesh.h"
#include "meshmod/vertexdata/vertexcontainers.h"
#include "meshmod/vertexdata/uvvertex.h"
#include "meshmod/vertexdata/colourvertex.h"
#include "meshmod/vertexdata/normalvertex.h"
#include "meshops/gltf.h"
#include "meshops/basicmeshops.h"
#include "meshops/convexhullcomputer.h"


static std::vector<std::shared_ptr<MeshMod::Mesh>> unityOwnedMeshes;
static std::vector<std::shared_ptr<MeshOps::ConvexHullComputer::ReturnType>> unityOwnedConvexHullComputers;

/*
 * 1) The unity Mesh approach
 * 		Add all the position data without sharing extra data(vertex in Unity parlance)
 * 		Add all the extra vertex data
 * 		Add Polygons/Faces
 * 		MeshCreationComplete
 */
static int const MESH_TYPE_FULL_MAINTENANCE = 0;
static int const MESH_TYPE_MAINTAIN_POINT_REP = 0x1;
static int const MESH_TYPE_MAINTAIN_EDGE_CONNECTIONS = 0x2;

// creates and returns a mesh handle to unity, just pass it back
CAPI auto CGE_CreateMesh( int type, char *name ) -> MeshHandle
{

	size_t index = unityOwnedMeshes.size();

	bool maintainPointRep = type & MESH_TYPE_MAINTAIN_POINT_REP;
	bool maintainEdgeConnections = type & MESH_TYPE_MAINTAIN_EDGE_CONNECTIONS;
	auto ptr = std::make_shared<MeshMod::Mesh>( name, maintainPointRep, maintainEdgeConnections);

	unityOwnedMeshes.push_back(ptr);
	return (uint64_t) index;
}
// creates and returns a mesh handle to unity, just pass it back
CAPI auto CGE_CreateMeshFromSimpleMesh( int type, char *name, SimpleMesh *simpleMesh ) -> MeshHandle
{
	using namespace MeshMod;
	bool maintainPointRep = type & MESH_TYPE_MAINTAIN_POINT_REP;
	bool maintainEdgeConnections = type & MESH_TYPE_MAINTAIN_EDGE_CONNECTIONS;

	size_t index = unityOwnedMeshes.size();
	auto mesh = std::make_shared<MeshMod::Mesh>( name, maintainPointRep, maintainEdgeConnections);

	Vertices& vertices = mesh->getVertices();
	for(auto i = 0u; i < simpleMesh->positionCount; ++i)
	{
		vertices.add( simpleMesh->positions[(i * 3) + 0],
					  simpleMesh->positions[(i * 3) + 1],
					  simpleMesh->positions[(i * 3) + 2] );
	}

	Polygons& polygons = mesh->getPolygons();
	for(auto i = 0u; i < simpleMesh->triangleCount; ++i)
	{
		VertexIndexContainer triIndices = {
				VertexIndex(simpleMesh->triangleIndices[(i * 3) + 0]),
				VertexIndex(simpleMesh->triangleIndices[(i * 3) + 2]),
				VertexIndex(simpleMesh->triangleIndices[(i * 3) + 1])
		};
		mesh->getPolygons().addPolygon( triIndices );
	}

	mesh->updateFromEdits();

	unityOwnedMeshes.push_back(mesh);
	return (uint64_t) index;

}

// when you have finished with the mesh, this will clean up
CAPI auto CGE_DeleteMesh( MeshHandle meshHandle ) -> void
{
	assert( meshHandle < unityOwnedMeshes.size());
	unityOwnedMeshes[(size_t) meshHandle].reset();
}

// all vertex positions should be added before vertex data and before indices
CAPI auto CGE_AddPositions( MeshHandle meshHandle, uint32_t count, intptr_t iptr ) -> uint32_t 
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );

	auto mesh = unityOwnedMeshes[(uint32_t) meshHandle];
	auto startIndex = mesh->getVertices().getCount();

	float *positions = reinterpret_cast<float *>(iptr);
	for(auto i = 0u; i < count; ++i)
	{
		mesh->getVertices().add(	positions[i * 3 + 0], 
									positions[i * 3 + 1],
									positions[i * 3 + 2] );
	}
	return uint32_t(startIndex);
}

// add any optional vertex data after AddPositions
CAPI auto CGE_AddVertexData( MeshHandle meshHandle, char *typeName, uint32_t startIndex, uint32_t count, intptr_t iptr ) -> void
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );

	using namespace MeshMod;
	auto mesh = unityOwnedMeshes[(size_t) meshHandle];
	assert( mesh->getVertices().getCount() >= (startIndex + count));

	VerticesElementsContainer& vertCon = mesh->getVertices().getVerticesContainer();

	switch(Core::QuickHash( std::string( typeName )))
	{
		case "uvs"_hash:
		{
			auto uvEle = vertCon.getOrAddElement<VertexData::UVs>();
			float *data = reinterpret_cast<float *>(iptr);
			for(auto i = startIndex; i < startIndex + count; ++i)
			{
				(*uvEle)[VertexIndex(i)] = VertexData::UV( data[i * 2 + 0], data[i * 2 + 1] );
			}
			break;
		}
		case "uvs_1"_hash:
		{
			auto uvEle = vertCon.getOrAddElement<VertexData::UVs>( "1" );
			float *data = reinterpret_cast<float *>(iptr);
			for(auto i = startIndex; i < startIndex + count; ++i)
			{
				(*uvEle)[VertexIndex(i)] = VertexData::UV( data[i * 2 + 0], data[i * 2 + 1] );
			}
			break;
		}
		case "normals"_hash:
		{
			auto normEle = vertCon.getOrAddElement<VertexData::Normals>();
			float *data = reinterpret_cast<float *>(iptr);
			for(auto i = startIndex; i < startIndex + count; ++i)
			{
				(*normEle)[VertexIndex(i)] = VertexData::Normal( data[i * 3 + 0], data[i * 3 + 1], data[i * 3 + 2] );
			}
			break;
		}
		case "tangents"_hash:
		{ // this is actually the binormal usually but whatever
			auto normEle = vertCon.getOrAddElement<VertexData::Normals>( "binormal" );
			float *data = reinterpret_cast<float *>(iptr);
			for(auto i = startIndex; i < startIndex + count; ++i)
			{
				(*normEle)[VertexIndex(i)] = VertexData::Normal( data[i * 3 + 0], data[i * 3 + 1], data[i * 3 + 2] );
			}
			break;
		}
		case "colours"_hash:
		{ // 4 x float
			auto colEle = vertCon.getOrAddElement<VertexData::FloatRGBAColourVertexElements>();
			float *data = reinterpret_cast<float *>(iptr);
			for(auto i = startIndex; i < startIndex + count; ++i)
			{
				(*colEle)[VertexIndex(i)] = VertexData::FloatRGBAColour(
						data[i * 4 + 0], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3] );
			}
			break;
		}
		default:
			assert( false && "invalid typeName" );
	}
}


CAPI auto CGE_AddTriangle( MeshHandle meshHandle, uint32_t i0, uint32_t i1, uint32_t i2 ) -> uint32_t
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );

	using namespace MeshMod;
	auto mesh = unityOwnedMeshes[(size_t) meshHandle];

	VertexIndexContainer tri = {
		VertexIndex(i0),
		VertexIndex(i1),
		VertexIndex(i2)
	};
	return (uint32_t) mesh->getPolygons().addPolygon( tri );
}

CAPI auto CGE_AddQuad( MeshHandle meshHandle, uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3 ) -> uint32_t
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );

	using namespace MeshMod;
	auto mesh = unityOwnedMeshes[(size_t) meshHandle];

	VertexIndexContainer quad = {
		VertexIndex(i0),
		VertexIndex(i1),
		VertexIndex(i2),
		VertexIndex(i3)
	};

	return (uint32_t) mesh->getPolygons().addPolygon(quad);
}

CAPI auto CGE_AddTriangles( MeshHandle meshHandle, uint32_t count, intptr_t indicesPtr ) -> uint32_t
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );

	using namespace MeshMod;
	auto mesh = unityOwnedMeshes[(size_t) meshHandle];
	auto startFace = mesh->getPolygons().getCount();

	uint32_t *indices = reinterpret_cast<uint32_t *>(indicesPtr);

	for(uint32_t i = 0; i < count * 3; i += 3)
	{
		VertexIndexContainer tri = {
			VertexIndex(indices[i + 0]), 
			VertexIndex(indices[i + 2]), 
			VertexIndex(indices[i + 1])
		};
		mesh->getPolygons().addPolygon( tri );
	}

	return uint32_t(startFace);
}

CAPI auto CGE_AddQuads( MeshHandle meshHandle, uint32_t count, intptr_t indicesPtr ) -> uint32_t
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );

	using namespace MeshMod;
	auto mesh = unityOwnedMeshes[(size_t) meshHandle];
	auto startFace = mesh->getPolygons().getCount();

	uint32_t *indices = reinterpret_cast<uint32_t *>(indicesPtr);
	for(uint32_t i = 0; i < count * 4; i += 4)
	{
		VertexIndexContainer quad = {
			VertexIndex(indices[i + 0]), 
			VertexIndex(indices[i + 2]), 
			VertexIndex(indices[i + 1]), 
			VertexIndex(indices[i + 3])
		};
		mesh->getPolygons().addPolygon( quad );
	}

	return uint32_t(startFace);
}

CAPI auto CGE_MeshCreationComplete( MeshHandle meshHandle ) -> void
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );

	using namespace MeshMod;
	auto mesh = unityOwnedMeshes[(size_t) meshHandle];

	mesh->updateEditState( MeshMod::Mesh::TopologyEdits );
	mesh->updateFromEdits();
}

CAPI auto CGE_GenerateConvexHulls(MeshHandle meshHandle, MeshOps::ConvexHullParameters* params_, MeshHandle* out) -> uint32_t
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );

	using namespace MeshMod;
	auto in = unityOwnedMeshes[(size_t) meshHandle];

	MeshOps::ConvexHullParameters params;
	if (params_ != nullptr)
	{
		std::memcpy(&params, params_, sizeof(MeshOps::ConvexHullParameters));
	}
	auto convexHulls = MeshOps::ConvexHullComputer::generate(in, params);

	for(auto i = 0u; i < convexHulls.size(); ++i)
	{
		out[i] = TakeOwnershipOfMesh(convexHulls[i]);
	}

	return (uint32_t)convexHulls.size();

}

CAPI auto CGE_GenerateConvexHullInline(MeshHandle meshHandle) -> void
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );

	using namespace MeshMod;
	auto mesh = unityOwnedMeshes[(size_t) meshHandle];
	MeshOps::ConvexHullComputer::generateInline(mesh);
}

CAPI auto CGE_ExportMeshToGLTF( MeshHandle meshHandle, char *filename ) -> void
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );

	using namespace MeshMod;
	auto mesh = unityOwnedMeshes[(size_t) meshHandle];

	SceneNode::Ptr rootNode = std::make_shared<SceneNode>();
	rootNode->addObject( mesh );
	MeshOps::Gltf::SaveAscii( rootNode, filename );
}

CAPI auto CGE_MeshToSimpleMesh(MeshHandle meshHandle, SimpleMesh* out) -> bool
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );

	using namespace MeshMod;
	auto in = unityOwnedMeshes[(size_t) meshHandle];

	auto const& vertices = in->getVertices();
	auto const& polygons = in->getPolygons();

	// todo add generic mesh tags to avoid performance sapping re-triangulation
	MeshOps::BasicMeshOps::triangulate(in);

	auto const& positions = vertices.positions();
	if( out->positionCount == 0 ||
		out->triangleCount == 0)
	{
		out->positionCount = uint32_t(vertices.getCount());
		out->triangleCount = uint32_t(polygons.getCount());
		return false;
	}
	assert(out->positions != nullptr);
	assert(out->triangleIndices != nullptr);
	assert(out->positionCount == uint32_t(vertices.getCount()));
	assert(out->triangleCount == uint32_t(polygons.getCount()));

	float* points = out->positions;
	for(auto const& pos : positions)
	{
		size_t i = size_t(positions.distance(pos)) * 3;
		points[i + 0] = pos.x;
		points[i + 1] = pos.y;
		points[i + 2] = pos.z;
	}

	uint32_t* indices = out->triangleIndices;
	VertexIndexContainer vertexIndices;
	for(auto i = 0u; i < polygons.getCount(); ++i)
	{
		vertexIndices.clear();
		polygons.getVertexIndices(PolygonIndex(i), vertexIndices );
		assert(vertexIndices.size()==3);

		indices[i*3+0] = (uint32_t) vertexIndices[0];
		indices[i*3+1] = (uint32_t) vertexIndices[2];
		indices[i*3+2] = (uint32_t) vertexIndices[1];
	}
	return true;
}

CAPI auto CGE_CreateConvexHullsAsync(MeshHandle meshHandle, MeshOps::ConvexHullParameters* params_)->ConvexHullGeneratorHandle
{
	auto meshPtr = UnityOwnedMesh(meshHandle);

	MeshOps::ConvexHullParameters params;
	if (params_ != nullptr)
	{
		std::memcpy(&params, params_, sizeof(MeshOps::ConvexHullParameters));
	}

	auto ptr = MeshOps::ConvexHullComputer::createAsync(meshPtr, *params_);

	size_t index = unityOwnedConvexHullComputers.size();
	unityOwnedConvexHullComputers.push_back(ptr);
	return (uint64_t)index;
}

CAPI auto CGE_DestroyConvexHullsAsync(ConvexHullGeneratorHandle cvHandle)->void
{
	assert(cvHandle < unityOwnedConvexHullComputers.size());
	assert(unityOwnedConvexHullComputers[(size_t)cvHandle]);
	unityOwnedConvexHullComputers[(size_t)cvHandle].reset();

}

CAPI auto CGE_ConvexHullAsyncIsReady(ConvexHullGeneratorHandle cvHandle)->bool
{
	assert(cvHandle < unityOwnedConvexHullComputers.size());
	assert(unityOwnedConvexHullComputers[(size_t)cvHandle]);
	return MeshOps::ConvexHullComputer::isReady(unityOwnedConvexHullComputers[(size_t)cvHandle]);
}

CAPI auto CGE_ConvexHullAsyncGetResults(ConvexHullGeneratorHandle cvHandle, MeshHandle* out)->uint32_t
{
	assert(cvHandle < unityOwnedConvexHullComputers.size());
	assert(unityOwnedConvexHullComputers[(size_t)cvHandle]);
	auto ptr = unityOwnedConvexHullComputers[(size_t)cvHandle];

	auto convexHulls = MeshOps::ConvexHullComputer::getResults(ptr);
	for (auto i = 0u; i < convexHulls.size(); ++i)
	{
		out[i] = TakeOwnershipOfMesh(convexHulls[i]);
	}

	return (uint32_t)convexHulls.size();
}

CAPI static auto DestroyAll() -> void
{
	unityOwnedMeshes = {};
	unityOwnedConvexHullComputers = {};
}

// for other native libraries to consume CGeometryEngine handles
EXPORT_CPP auto UnityOwnedMesh(MeshHandle meshHandle) -> std::shared_ptr<MeshMod::Mesh>
{
	assert( meshHandle < unityOwnedMeshes.size());
	assert( unityOwnedMeshes[(size_t) meshHandle] );
	return unityOwnedMeshes[(size_t) meshHandle];
}

EXPORT_CPP auto TakeOwnershipOfMesh(std::shared_ptr<MeshMod::Mesh> mesh) -> MeshHandle
{
	size_t index = unityOwnedMeshes.size();
	MeshMod::Mesh::Ptr ptr = unityOwnedMeshes.emplace_back(mesh);
	return (uint64_t)index;
}

static CGeometryEngineInterface Interface;

#if !defined(USING_STATIC_LIBS)
EXPORT void* GetInterface()
#else
CGeometryEngineInterface* CGeometryEngine()
#endif
{
	if (Interface.CGE_CreateMesh == nullptr)
	{
		Interface.DestroyAll = &DestroyAll;
		Interface.CGE_CreateMesh = &CGE_CreateMesh;
		Interface.CGE_CreateMeshFromSimpleMesh = &CGE_CreateMeshFromSimpleMesh;
		Interface.CGE_DeleteMesh = &CGE_DeleteMesh;
		Interface.CGE_AddPositions = &CGE_AddPositions;
		Interface.CGE_AddVertexData = &CGE_AddVertexData;
		Interface.CGE_AddTriangle = &CGE_AddTriangle;
		Interface.CGE_AddQuad = &CGE_AddQuad;
		Interface.CGE_AddTriangles = &CGE_AddTriangles;
		Interface.CGE_AddQuads = &CGE_AddQuads;
		Interface.CGE_MeshCreationComplete = &CGE_MeshCreationComplete;
		Interface.CGE_ExportMeshToGLTF = &CGE_ExportMeshToGLTF;
		Interface.CGE_MeshToSimpleMesh = &CGE_MeshToSimpleMesh;

		Interface.CGE_GenerateConvexHulls = &CGE_GenerateConvexHulls;
		Interface.CGE_GenerateConvexHullInline = &CGE_GenerateConvexHullInline;
		Interface.CGE_CreateConvexHullsAsync = &CGE_CreateConvexHullsAsync;
		Interface.CGE_DestroyConvexHullsAsync = &CGE_DestroyConvexHullsAsync;
		Interface.CGE_ConvexHullAsyncIsReady = &CGE_ConvexHullAsyncIsReady;
		Interface.CGE_ConvexHullAsyncGetResults = &CGE_ConvexHullAsyncGetResults;
	}
	return &Interface;
}