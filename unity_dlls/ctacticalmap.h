#pragma once
#ifndef NATIVESNAPSHOT_CTACTICALMAP_H
#define NATIVESNAPSHOT_CTACTICALMAP_H

#include "core/core.h"
#include "tacticalmap/tacticalmap.h"
#include "meshmod/scenenode.h"
#include "cutils.h"

namespace Core { struct Blob; }

// tactical maps can be a single parcel or stitched from multiple parcels at various postitions/orientations
// at the native level parcels are small tactical maps

using TacticalMapHandle = uint64_t;
using TacticalMapBuilderHandle = uint64_t;
using TacticalMapStitcherHandle = uint64_t;
constexpr uint64_t TacticalMapInvalidHandle = ~0;

// stitcher API
struct ParcelInstances
{
	uint32_t count;						// number of instances
	float const* positions;				// 3 floats per instance
	uint32_t const* rotationInDegrees;	// 1 uint32_t per instance (0, 90, 180, 270)
	uint32_t const* mapParcelIds;		// 1 uint32_t per instance
};

struct CTacticalMapInterface
{
	CAPI auto (*DestroyAll)() -> void;
	// tactical map / parcels are built by the builder and loaded when used
	CAPI auto (*CTM_Load)(char const* fileName)->TacticalMapHandle;
	CAPI auto (*CTM_LoadFromBlob)(Core::Blob* blob)->TacticalMapHandle;
	CAPI auto (*CTM_Save)(TacticalMapHandle ctmHandle, uint64_t userData, char const* fileName) -> bool;
	CAPI auto (*CTM_SaveToBlob)(TacticalMapHandle ctmHandle, uint64_t userData, Core::Blob* out) -> bool;
	CAPI auto (*CTM_Delete)(TacticalMapHandle ctmHandle) -> void;
	CAPI auto (*CTM_LookupVolumeAtWorld)(TacticalMapHandle ctmHandle, float const* point, float const range, uint32_t levelMask, TacticalMapVolume* out) -> bool;
	CAPI auto (*CTM_LookupLevelDataAtWorld)(TacticalMapHandle ctmHandle, float const* point, float const range, uint32_t levelMask, TacticalMapLevelDataHeader* out) -> bool;
	CAPI auto (*CTM_DamageStructure)(TacticalMapHandle ctmHandle, float const* center, float const* extent) -> void;

	CAPI auto (*CTMS_CreateStitcher)(char const* name_) -> TacticalMapStitcherHandle;
	CAPI auto (*CTMS_AddParcelInstances)(TacticalMapStitcherHandle ctmsHandle, TacticalMapHandle tmHandle, ParcelInstances* instances) -> void;
	CAPI auto (*CTMS_Stitch)(TacticalMapStitcherHandle ctmsHandle)->TacticalMapHandle;
	CAPI auto (*CTMS_Delete)(TacticalMapStitcherHandle ctmsHandle) -> void;

	// build API
	CAPI auto (*CTMB_CreateBuilder)(float* bounds2D, char const* name)->TacticalMapBuilderHandle;
	CAPI auto (*CTMB_SetMinimumHeight)(TacticalMapBuilderHandle handle_, float const minHeight_) -> void;
	CAPI auto (*CTMB_SetOpaqueLevelDataSize)(TacticalMapBuilderHandle handle_, uint32_t const size_) -> void;
	CAPI auto (*CTMB_AddMeshAt)(TacticalMapBuilderHandle handle, TacticalMapHandle meshHandle, TacticalMapLevelDataHeader const* opaqueData, float const* matrix) -> void;
	CAPI auto (*CTMB_AddBoxAt)(TacticalMapBuilderHandle handle, TacticalMapLevelDataHeader const* opaqueData, float const* center, float const* extent, float const* matrix) -> void;
	CAPI auto (*CTMB_DebugExportToGLTF)(TacticalMapBuilderHandle ctmbHandle, char const* fileName) -> void;
	CAPI auto (*CTMB_Build)(TacticalMapBuilderHandle ctmbHandle)->TacticalMapHandle;
	CAPI auto (*CTMB_Delete)(TacticalMapBuilderHandle ctmbHandle) -> void;
};

// cpp helpers
EXPORT_CPP auto CTMB_ExportToGLTF(TacticalMapBuilderHandle ctmbHandle, char const* fileName) -> MeshMod::SceneNode::Ptr;
EXPORT_CPP auto UnityOwnedTacticalMap(TacticalMapHandle handle) -> std::shared_ptr<TacticalMap>;
EXPORT_CPP auto UnityOwnedTacticalMapBuilder(TacticalMapBuilderHandle handle) -> std::shared_ptr<ITacticalMapBuilder>;
EXPORT_CPP auto UnityOwnedTacticalMapStitcher(TacticalMapStitcherHandle handle) ->std::shared_ptr<ITacticalMapStitcher>;

#if !defined(USING_STATIC_LIBS)
EXPORT void* GetInterface();
#else
CTacticalMapInterface* CTacticalMap();
#endif
#endif //NATIVESNAPSHOT_CTACTICALMAP_H
