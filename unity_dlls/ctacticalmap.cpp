#if !defined(USING_STATIC_LIBS)
#define STB_IMAGE_IMPLEMENTATION
#endif

#include "core/core.h"
#include "cgeometryengine.h"
#include "tacticalmap/tacticalmap.h"
#include "tacticalmap/builder.h"
#include "meshmod/scenenode.h"
#include "meshmod/vertices.h"
#include "meshmod/polygons.h"
#include "meshops/basicmeshops.h"
#include "meshops/platonicsolids.h"
#include "meshops/gltf.h"
#include <enkiTS/src/TaskScheduler.h>
#include <limits>
#include <fstream>
#include "core/freelist.h"
#include "core/blob.h"

#if !defined(USING_STATIC_LIBS)
enki::TaskScheduler g_EnkiTS;
#endif

#include "tinygltf/stb_image.h"
#include "ctacticalmap.h"

static Core::FreeList<TacticalMap::Ptr, uint32_t> unityOwnedTacticalMap;
static Core::FreeList<ITacticalMapBuilder::Ptr, uint32_t> unityOwnedTacticalMapBuilders;
static Core::FreeList<ITacticalMapStitcher::Ptr, uint32_t> unityOwnedTacticalMapStitchers;

CAPI auto CTM_Load(char const* fileName) -> TacticalMapHandle
{
	std::vector<TacticalMap::Ptr> tactMaps;
	std::ifstream in(fileName, std::ifstream::in | std::ifstream::binary);
	TacticalMap::createFromStream(in, tactMaps);

	if(tactMaps.empty()) return TacticalMapInvalidHandle;

	// only handle the first tmap in a bundle currently
	TacticalMapHandle handle = unityOwnedTacticalMap.push(tactMaps[0]);

	return handle;
}
CAPI auto CTM_LoadFromBlob(Core::Blob* blob) -> TacticalMapHandle
{
	if(blob == nullptr) return TacticalMapInvalidHandle;
	if(blob->size == 0) return TacticalMapInvalidHandle;
	if(blob->nativeData == nullptr) return TacticalMapInvalidHandle;

	std::string str;
	str.resize(blob->size);
	std::memcpy(str.data(), blob->nativeData, blob->size);
	std::istringstream stream(str);

	std::vector<TacticalMap::Ptr> tactMaps;
	bool okay = TacticalMap::createFromStream(stream, tactMaps);

	if(!okay || tactMaps.empty()) return TacticalMapInvalidHandle;

	// only handle the first tmap in a bundle currently
	TacticalMapHandle handle = unityOwnedTacticalMap.push(tactMaps[0]);

	return handle;
}

CAPI auto CTM_Delete(TacticalMapHandle ctmHandle) -> void
{
	if (ctmHandle == ~0) return;
	unityOwnedTacticalMap.at((uint32_t)ctmHandle).reset();
}
CAPI auto CTM_Save(TacticalMapHandle ctmHandle, uint64_t userData, char const* fileName) -> bool
{
	if (ctmHandle == ~0) return false;
	auto tm = unityOwnedTacticalMap.at((uint32_t)ctmHandle);

	// write it out to a memory block
	std::vector<uint8_t> rawBundle;
	bool okay = tm->saveTo(userData, rawBundle);
	if(!okay) return false;

	std::ofstream out(fileName, std::ofstream::out | std::ofstream::binary);
	out.write((char const*)rawBundle.data(), rawBundle.size());
	if(out.bad()) return false;

	return true;
}

CAPI auto CTM_SaveToBlob(TacticalMapHandle ctmHandle, uint64_t userData, Core::Blob* out) -> bool
{
	if (ctmHandle == ~0) return false;
	auto tm = unityOwnedTacticalMap.at((uint32_t)ctmHandle);

	// write it out to a memory block
	std::vector<uint8_t> rawBundle;
	bool okay = tm->saveTo(userData, rawBundle);
	if(!okay) return false;
	okay = Core::Blob::Create(rawBundle.size(), out);
	if(!okay) return false;
	if( out->size != rawBundle.size()) return false;

	std::memcpy(out->nativeData, rawBundle.data(), out->size);

	return true;
}

CAPI auto CTM_LookupVolumeAtWorld(TacticalMapHandle ctmHandle, float const* point, float const range, uint32_t const levelMask, TacticalMapVolume* out) -> bool
{
	if (ctmHandle == ~0) return false;
	auto tm = unityOwnedTacticalMap.at((uint32_t)ctmHandle);
	return tm->lookupVolumeAtWorld(Math::Vec3FromArray(point), range, levelMask, out);
}

CAPI auto CTM_LookupLevelDataAtWorld(TacticalMapHandle ctmHandle, float const* point, float const range, uint32_t const levelMask, TacticalMapLevelDataHeader* out) -> bool
{
	if (ctmHandle == ~0) return false;
	auto tm = unityOwnedTacticalMap.at((uint32_t)ctmHandle);
	return tm->lookupLevelDataAtWorld(Math::Vec3FromArray(point), range, levelMask, out);
}

CAPI auto CTM_DamageStructure(TacticalMapHandle ctmHandle, float const* center, float const* extent) -> void
{
	if (ctmHandle == ~0) return;
	auto tm = unityOwnedTacticalMap.at((uint32_t)ctmHandle);

	Math::vec3 vCenter = Math::Vec3FromArray(center);
	Math::vec3 vHalfLength = Math::Vec3FromArray(extent);
	vHalfLength *= 0.5f;
	Geometry::AABB box = Geometry::AABB::fromCenterAndHalfLength(vCenter, vHalfLength);
	tm->damageStructure(box);
}

//------------------------------------------------------//

CAPI auto CTMB_CreateBuilder(float* bounds2D) -> TacticalMapBuilderHandle
{
	int const width = (int)std::floor(bounds2D[2] - bounds2D[0]);
	int const height = (int)std::floor(bounds2D[3] - bounds2D[1]);

	const uint32_t maxSupported = std::numeric_limits<TacticalMap::TileCoord_t>::max();
	if (width > maxSupported || height > maxSupported) return ~0;
	if (width <= 0 || height <= 0) return ~0;

#if !defined(USING_STATIC_LIBS)
	g_EnkiTS.Initialize();
#endif

	Math::vec2 bounds(bounds2D[0], bounds2D[1]);

	std::shared_ptr<ITacticalMapBuilder> builder = TacticalMap::allocateBuilder(bounds, width, height);

	return unityOwnedTacticalMapBuilders.push(builder);
}

CAPI auto CTMB_Delete(TacticalMapBuilderHandle tmHandle) -> void
{
	if (tmHandle == ~0) return;
	unityOwnedTacticalMapBuilders.at((uint32_t)tmHandle).reset();
}
CAPI auto CTMB_SetMinimumHeight(TacticalMapBuilderHandle handle_, float const minHeight_) -> void
{
	if (handle_ == ~0) return;
	assert(unityOwnedTacticalMapBuilders.at((uint32_t)handle_));
	auto const ibuilder = unityOwnedTacticalMapBuilders[(uint32_t)handle_];
	ibuilder->setMinimumHeight(minHeight_);
}

CAPI auto CTMB_SetOpaqueLevelDataSize(TacticalMapBuilderHandle handle_, uint32_t const size_) -> void
{
	if (handle_ == ~0) return;
	assert(unityOwnedTacticalMapBuilders.at((uint32_t)handle_));
	auto const ibuilder = unityOwnedTacticalMapBuilders[(uint32_t)handle_];
	ibuilder->setLevelDataSize(size_);
}
CAPI auto CTMB_DebugExportToGLTF(TacticalMapBuilderHandle tmHandle, char const* fileName) -> void
{
	if (tmHandle == ~0) return;
	CTMB_ExportToGLTF(tmHandle, fileName);
}

CAPI auto CTMB_ExportToGLTF(TacticalMapBuilderHandle tmHandle, char const* fileName) -> MeshMod::SceneNode::Ptr
{
	using namespace MeshMod;

	assert(unityOwnedTacticalMapBuilders.at((uint32_t)tmHandle) );
	auto const ibuilder = unityOwnedTacticalMapBuilders[(uint32_t)tmHandle];
	auto const builder = std::dynamic_pointer_cast<TacticalMapBuilder>(ibuilder);
	assert(builder);

	typedef std::unordered_set<PolygonIndex> MeshPolygons;
	std::unordered_map<MeshMod::Mesh*, MeshPolygons> meshPolygons;

	std::vector<MeshMod::MeshPtr> boxes;
	std::unordered_map<uint64_t, std::vector<Geometry::AABB>> mortonBoxesAABBs;

	Math::vec3 const mortonMax(	(float)builder->getWidth(),
									(float)builder->getHeight(),
									(float)builder->getHeight());

	for (auto y = 0; y < builder->getHeight(); ++y)
	{
		for (auto x = 0; x < builder->getWidth(); ++x)
		{
			auto const& tileBuilder = builder->tileBuilders[y * builder->getWidth() + x];
			for (auto const& [solidIndex, polygonIndex]: tileBuilder.polygons)
			{
				auto& [mesh, box, levelData] = builder->solids[solidIndex];

				meshPolygons[mesh.get()].insert(polygonIndex.cbegin(), polygonIndex.cend());
			}
			// expensive compares!
			for (auto const& [solidIndex, box] : tileBuilder.boxes)
			{
				uint64_t mortonCode = Math::MortonCurve(box.getMinExtent(), mortonMax);
				auto& boxesAABBs = mortonBoxesAABBs[mortonCode];
				bool found = false;
				for (auto const& cbox : boxesAABBs)
				{
					if (Math::ApproxEqual(cbox.getMinExtent(), box.getMinExtent()) &&
						Math::ApproxEqual(cbox.getMaxExtent(), box.getMaxExtent()))
					{
						found = true;
						break;
					}
				}
				if (found == false)
				{
					boxesAABBs.push_back(box);
				}
			}
		}
	}
	for (auto const it : mortonBoxesAABBs)
	{
		for (auto const& box : it.second)
		{
			boxes.push_back(MeshOps::PlatonicSolids::createBoxFrom(box));
			auto boxMesh = boxes.back();
			auto& meshPolys = meshPolygons[boxMesh.get()];
			boxMesh->getPolygons().visitValid([&meshPolys](MeshMod::PolygonIndex polygonIndex)
			{
				meshPolys.insert(polygonIndex);
			});
		}
	}

	MeshPtr out = std::make_shared<MeshMod::Mesh>("TacticalMap");
	for (auto const& meshpair : meshPolygons)
	{
		auto& mesh = meshpair.first;
		auto const& vertices = mesh->getVertices();
		auto const& polygons = mesh->getPolygons();

		VertexIndexContainer polyIndices;
		polyIndices.reserve(10);
		for (auto const polygonIndex : meshpair.second)
		{
			polyIndices.clear();
			polygons.visitVertices(polygonIndex,
				[&vertices, &out, &polyIndices](VertexIndex vIndex)
				{
					auto const pos = vertices.position(vIndex);
					VertexIndex outVIndex = out->getVertices().add(pos.x, pos.y, pos.z);
					polyIndices.push_back(outVIndex);
				});
			out->getPolygons().add(polyIndices, 0);
		}
	}
	out->updateFromEdits();

	MeshOps::BasicMeshOps::triangulate(out);

	auto rootScene = std::make_shared<SceneNode>();
	rootScene->addObject(out);

	MeshOps::Gltf::SaveAscii(rootScene, fileName);
	return rootScene;
}


CAPI auto CTMB_Build(TacticalMapBuilderHandle tmbHandle) -> TacticalMapHandle
{
	if (tmbHandle == ~0) return ~0;
	assert(unityOwnedTacticalMapBuilders.at((uint32_t)tmbHandle));

	auto const tmb = unityOwnedTacticalMapBuilders[(uint32_t)tmbHandle];
	auto tm = tmb->build();
	if(!tm) return TacticalMapInvalidHandle;

	return unityOwnedTacticalMap.push(tm);
}

CAPI auto CTMB_AddMeshAt(TacticalMapBuilderHandle handle, TacticalMapHandle meshHandle, TacticalMapLevelDataHeader const* levelData, float const* matrix) -> void
{
	if (handle == ~0) return;

	assert(unityOwnedTacticalMapBuilders.at((uint32_t)handle));
	assert(levelData);
	auto const tmb = unityOwnedTacticalMapBuilders[(uint32_t)handle];
	MeshMod::MeshPtr mesh(UnityOwnedMesh(meshHandle));

	Math::mat4x4 transform = Math::Mat4x4FromArray(matrix);
	tmb->addMeshAt(mesh, levelData, transform);
}

CAPI auto CTMB_AddBoxAt(TacticalMapBuilderHandle handle, TacticalMapLevelDataHeader const* levelData, float const* center, float const* extent, float const* matrix) -> void
{
	if (handle == ~0) return;

	assert(unityOwnedTacticalMapBuilders.at((uint32_t)handle) );
	assert(levelData);

	auto const tmb = unityOwnedTacticalMapBuilders[(uint32_t)handle];

	Math::vec3 vCenter = Math::Vec3FromArray(center);
	Math::vec3 vHalfLength= Math::Vec3FromArray(extent);
	vHalfLength *= 0.5f;
	Geometry::AABB box = Geometry::AABB::fromCenterAndHalfLength(vCenter, vHalfLength);
	Math::mat4x4 transform = Math::Mat4x4FromArray(matrix);
	tmb->addBoxAt(box, levelData, transform);
}

CAPI auto CTMS_CreateStitcher() -> TacticalMapStitcherHandle
{
#if !defined(USING_STATIC_LIBS)
	g_EnkiTS.Initialize();
#endif

	std::shared_ptr<ITacticalMapStitcher> builder = TacticalMap::allocateStitcher();

	return unityOwnedTacticalMapStitchers.push(builder);
}

CAPI auto CTMS_AddParcelInstances(TacticalMapStitcherHandle ctmsHandle, TacticalMapHandle tmHandle, ParcelInstances* instances) -> void
{
	if (ctmsHandle == ~0) return;
	assert(unityOwnedTacticalMapStitchers[(uint32_t)ctmsHandle] );
	auto const tms = unityOwnedTacticalMapStitchers.at((uint32_t)ctmsHandle);
	auto const map = unityOwnedTacticalMap.at((uint32_t)tmHandle);

	for(auto index = 0u; index < instances->count; ++index)
	{
		tms->addTacticalMapInstance(map, 
			Math::Vec3FromArray(instances->positions + index*3),
			instances->rotationInDegrees[index], 
			instances->mapParcelIds[index]);
	}
}

CAPI auto CTMS_Stitch(TacticalMapStitcherHandle ctmsHandle) -> TacticalMapHandle
{
	if (ctmsHandle == ~0) return ~0;
	assert( ctmsHandle < unityOwnedTacticalMapStitchers.size());
	assert(unityOwnedTacticalMapStitchers[(uint32_t) ctmsHandle] );

	auto const tms = unityOwnedTacticalMapStitchers[(uint32_t)ctmsHandle];
	TacticalMap::Ptr tm = tms->build();
	if(!tm) return TacticalMapInvalidHandle;

	return unityOwnedTacticalMap.push(tm);
}

CAPI auto CTMS_Delete(TacticalMapStitcherHandle ctmsHandle) -> void
{
	if (ctmsHandle == ~0) return;
	assert(ctmsHandle < unityOwnedTacticalMapStitchers.size());
	unityOwnedTacticalMapStitchers[(uint32_t)ctmsHandle].reset();
}

CAPI static auto DestroyAll() -> void
{
	unityOwnedTacticalMap = {};
	unityOwnedTacticalMapBuilders = {};
	unityOwnedTacticalMapStitchers = {};
}

EXPORT_CPP auto UnityOwnedTacticalMap(TacticalMapHandle tmHandle) -> TacticalMap::Ptr
{
	assert(unityOwnedTacticalMap[(uint32_t)tmHandle]);
	return unityOwnedTacticalMap.at((uint32_t)tmHandle);
}

EXPORT_CPP auto UnityOwnedTacticalMapBuilder(TacticalMapBuilderHandle handle) -> std::shared_ptr<ITacticalMapBuilder> 
{
	assert(unityOwnedTacticalMapBuilders[(uint32_t)handle]);
	return unityOwnedTacticalMapBuilders.at((uint32_t)handle);
}

EXPORT_CPP auto UnityOwnedTacticalMapStitcher(TacticalMapStitcherHandle handle) -> std::shared_ptr<ITacticalMapStitcher>
{
	assert(unityOwnedTacticalMapStitchers[(uint32_t)handle]);
	return unityOwnedTacticalMapStitchers.at((uint32_t)handle);
}

static CTacticalMapInterface Interface;

#if !defined(USING_STATIC_LIBS)
EXPORT void* GetInterface()
#else
CTacticalMapInterface* CTacticalMap()
#endif
{
	if (Interface.CTM_Load == nullptr)
	{
		Interface.DestroyAll = &DestroyAll;
		Interface.CTM_Load = &CTM_Load;
		Interface.CTM_LoadFromBlob = &CTM_LoadFromBlob;
		Interface.CTM_Save = &CTM_Save;
		Interface.CTM_SaveToBlob = &CTM_SaveToBlob;
		Interface.CTM_Delete = &CTM_Delete;
		Interface.CTM_LookupVolumeAtWorld = &CTM_LookupVolumeAtWorld;
		Interface.CTM_LookupLevelDataAtWorld = &CTM_LookupLevelDataAtWorld;
		Interface.CTM_DamageStructure = &CTM_DamageStructure;

		Interface.CTMS_CreateStitcher = &CTMS_CreateStitcher;
		Interface.CTMS_AddParcelInstances = &CTMS_AddParcelInstances;
		Interface.CTMS_Stitch = &CTMS_Stitch;
		Interface.CTMS_Delete = &CTMS_Delete;

		Interface.CTMB_CreateBuilder = &CTMB_CreateBuilder;
		Interface.CTMB_SetMinimumHeight = &CTMB_SetMinimumHeight;
		Interface.CTMB_SetOpaqueLevelDataSize = &CTMB_SetOpaqueLevelDataSize;
		Interface.CTMB_AddMeshAt = &CTMB_AddMeshAt;
		Interface.CTMB_AddBoxAt = &CTMB_AddBoxAt;
		Interface.CTMB_DebugExportToGLTF = &CTMB_DebugExportToGLTF;
		Interface.CTMB_Build = &CTMB_Build;
		Interface.CTMB_Delete = &CTMB_Delete;
	}
	return &Interface;
}