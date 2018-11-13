#pragma once
#ifndef NATIVESNAPSHOT_CGEOMETRYENGINE_H
#define NATIVESNAPSHOT_CGEOMETRYENGINE_H

#include "core/core.h"
#include "meshmod/mesh.h"
#include "cutils.h"

using MeshHandle = uint64_t;
using ConvexHullGeneratorHandle = uint64_t;

namespace MeshOps
{
class ConvexHullParameters;
}

struct CGeometryEngineInterface
{
	CAPI auto (*DestroyAll)() -> void;
	CAPI auto (*CGE_CreateMesh)(int type, char *name) -> MeshHandle;
	CAPI auto (*CGE_CreateMeshFromSimpleMesh)(int type, char *name, SimpleMesh *simpleMesh) -> MeshHandle;
	CAPI auto (*CGE_DeleteMesh)(MeshHandle meshHandle) -> void;

	CAPI auto (*CGE_AddPositions)(MeshHandle meshHandle, uint32_t count, intptr_t iptr) -> uint32_t;
	CAPI auto (*CGE_AddVertexData)(MeshHandle meshHandle, char *typeName, uint32_t startIndex, uint32_t count, intptr_t iptr) -> void;
	CAPI auto (*CGE_AddTriangle)(MeshHandle meshHandle, uint32_t i0, uint32_t i1, uint32_t i2) -> uint32_t;
	CAPI auto (*CGE_AddQuad)(MeshHandle meshHandle, uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3) -> uint32_t;
	CAPI auto (*CGE_AddTriangles)(MeshHandle meshHandle, uint32_t count, intptr_t indicesPtr) -> uint32_t;
	CAPI auto (*CGE_AddQuads)(MeshHandle meshHandle, uint32_t count, intptr_t indicesPtr) -> uint32_t;
	CAPI auto (*CGE_MeshCreationComplete)(MeshHandle meshHandle) -> void;
	
	CAPI auto (*CGE_ExportMeshToGLTF)(MeshHandle meshHandle, char *filename) -> void;
	CAPI auto (*CGE_MeshToSimpleMesh)(MeshHandle meshHandle, SimpleMesh* out) -> bool;

	CAPI auto (*CGE_GenerateConvexHulls)(MeshHandle meshHandle, MeshOps::ConvexHullParameters* params_, MeshHandle* out)->uint32_t;
	CAPI auto (*CGE_GenerateConvexHullInline)(MeshHandle meshHandle) -> void;
	CAPI auto (*CGE_CreateConvexHullsAsync)(MeshHandle meshHandle, MeshOps::ConvexHullParameters* params_)->ConvexHullGeneratorHandle;
	CAPI auto (*CGE_DestroyConvexHullsAsync)(ConvexHullGeneratorHandle cvHandle)->void;
	CAPI auto (*CGE_ConvexHullAsyncIsReady)(ConvexHullGeneratorHandle cvHandle)->bool;
	CAPI auto (*CGE_ConvexHullAsyncGetResults)(ConvexHullGeneratorHandle cvHandle, MeshHandle* out)->uint32_t;

};

EXPORT_CPP MeshMod::Mesh::Ptr UnityOwnedMesh(MeshHandle meshHandle);
EXPORT_CPP MeshHandle TakeOwnershipOfMesh(MeshMod::Mesh::Ptr mesh);


#if !defined(USING_STATIC_LIBS)
EXPORT void* GetInterface();
#else
CGeometryEngineInterface* CGeometryEngine();
#endif

#endif //NATIVESNAPSHOT_CGEOMETRYENGINE_H
