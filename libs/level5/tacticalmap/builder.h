#pragma once
#ifndef NATIVESNAPSHOT_TACTICALMAP_BUILDER_H
#define NATIVESNAPSHOT_TACTICALMAP_BUILDER_H

#include "core/core.h"
#include "math/scalar_math.h"
#include "math/vector_math.h"
#include "meshmod/mesh.h"
#include "meshops/layeredtexture.h"
#include "geometry/aabb.h"
#include "tacticalmap/tacticalmap.h"

#include <memory>
#include <vector>
#include <list>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <array>

static int const fragmentSubSamples = 16; // ^2 samples per tile

struct TMapTBHeightFragment
{
	Math::vec3 n;
	float t;
	size_t solidIndex;
};

struct Bond
{
	Bond() : solidIndex(~0), otherIndex(~0), direction(Cardinal::Above) {}
	Bond(size_t solidIndex_, size_t otherIndex_, Cardinal direction_) :
		solidIndex(solidIndex_), otherIndex(otherIndex_), direction(direction_) {}
	size_t solidIndex;
	size_t otherIndex;
	Cardinal direction;
};

struct TMapTBStructuralBox
{
	std::vector<Bond> bonds;
	StructuralType structuralType;
	int32_t structuralIntegrity;
};

struct TMapTBLayer
{
	// note variance is just square(std dev), so we don't bother storing it...
	float floorMean = 0;
	float floorStdDev = 0;

	float ceilMean = 0;
	float ceilStdDev = 0;

	uint32_t floorDominantSolidIndex = 0;
	uint32_t ceilDominantSolidIndex = 0;

	std::vector<TMapTBHeightFragment const *> validFloorFragments;
	std::vector<TMapTBHeightFragment const *> validCeilFragments;

	Math::Plane floorPlane;
	Math::Plane ceilPlane;

	float minHeight;
	float maxHeight;

	Geometry::AABB aabb;
	std::vector<size_t> destructionSolidIndices;

};

struct TacticalMapTileBuilder
{
	~TacticalMapTileBuilder();
	using MeshPolygons = std::unordered_set<MeshMod::PolygonIndex>;
	using TMapTBHeightFragmentList = std::vector<TMapTBHeightFragment>;
	using TMapTBLayerList = std::vector<TMapTBLayer>;
	int maxLayers;

	std::unordered_map<size_t, MeshPolygons> polygons;
	std::unordered_map<size_t, Geometry::AABB> boxes;
	std::unordered_map<size_t, Geometry::AABB> destructables;

	using StructuralBoxList = std::vector<TMapTBStructuralBox>;
	using BondList = std::vector<Bond>;
	StructuralBoxList structuralBoxs;
	BondList bonds;

	// TODO using a vector for std::sort but this isn't ideal
	std::array<TMapTBHeightFragmentList, fragmentSubSamples * fragmentSubSamples> heightMaps;
	TMapTBLayerList layers;

};



class TacticalMapBuilder : public ITacticalMapBuilder
{
public:
	~TacticalMapBuilder();
	using TileCoord_t = int32_t;

	struct WorldSolid
	{
		WorldSolid(MeshMod::MeshPtr mesh_, Geometry::AABB aabb_, size_t extraLevelDataOffset_) :
			mesh(mesh_), aabb(aabb_), extraLevelDataOffset(extraLevelDataOffset_) {}
		MeshMod::MeshPtr mesh;
		Geometry::AABB aabb;
		size_t extraLevelDataOffset;
	};


	// these are public only for debug/vis purposes
	static inline Math::vec2 const extentIncrement = Math::vec2(1, 1);

	TacticalMapBuilder(Math::vec2 const bottomLeft_, TileCoord_t width_, TileCoord_t height_);

	using FragmentTextureLayer = MeshOps::TextureLayer<TMapTBHeightFragment const *>;
	using FragmentTexture = MeshOps::LayeredTexture;

	int getWidth() const { return (int)width; }
	int getHeight() const { return (int)height; }
	Math::vec2 const getBottomLeft() const { return bottomLeft; }

	std::vector<WorldSolid> solids;

	std::vector<TacticalMapTileBuilder> tileBuilders;
	std::unique_ptr<FragmentTexture> globalFragmentTexture;
	std::unique_ptr<MeshOps::LayeredTexture > smoothLayerTexture;

	void setMinimumHeight(float height_) final { rayMinHeight = height_; };
	void setMaximumFloorInclination(float angleInRadians_) final { maxFloorInclination = angleInRadians_; };
	void setLevelDataSize(uint32_t size_) final { tacticalLevelDataSize = size_; }
	void addMeshAt(MeshMod::MeshPtr const& mesh, TacticalMapLevelDataHeader const* levelData, Math::mat4x4 const& transform) final;
	void addBoxAt( Geometry::AABB const& box, TacticalMapLevelDataHeader const* levelData, Math::mat4x4 const& transform) final;
	TacticalMap::Ptr build() override;

private:


	Math::vec2 worldToLocal( Math::vec3 const& world ) const;
	void worldToLocal(Math::vec3 const& world, TileCoord_t& outX, TileCoord_t &outY) const;

	TileCoord_t width;
	TileCoord_t height;
	Math::vec2 bottomLeft;
	float minHeight, maxHeight;
	float rayMinHeight = -10000.0f;
	float maxFloorInclination = Math::degreesToRadians(30.0f);
	uint32_t tacticalLevelDataSize = sizeof(TacticalMapLevelDataHeader);
	std::vector<uint8_t> tacticalLevelDataHeap;

	// building functions
	void generateLayers();
	void generateHeightFragmentsForTileAt(TileCoord_t x, TileCoord_t z);
	void generatePlanesForTileAt(TileCoord_t x, TileCoord_t z);
	void generateStructuralBoxesForTileAt(TileCoord_t x, TileCoord_t z);

	void getValidFragmentsForTile(	TacticalMapTileBuilder& tileBuilder);
	void processHeightsForTile( TacticalMapTileBuilder& tileBuilder );

	void calculateHeightMapMeanAndStandardDeviation( TacticalMapTileBuilder& tileBuilder );

	void generateGlobalFragmentTexture();
	void smoothGlobalFragmentTexture();
	
	auto DetermineStructuralType(WorldSolid const& solid_)->StructuralType;

	void insertBox( Geometry::AABB const& box, std::function<void(TacticalMapTileBuilder&)> func);
};

inline Math::vec2 TacticalMapBuilder::worldToLocal( Math::vec3 const& world ) const
{
	return Math::vec2((world.x - bottomLeft.x), (world.z - bottomLeft.y));
}

inline void TacticalMapBuilder::worldToLocal(Math::vec3 const& world, TileCoord_t& outX, TileCoord_t &outY) const
{
	Math::vec2  local = worldToLocal(world) + Math::vec2(0.5f,0.5f);
	local = Math::clamp(local, { 0, 0 }, { (float)width, (float)height });
	outX = (TileCoord_t) std::floor(local.x);
	outY = (TileCoord_t) std::floor(local.y);
}

#endif //NATIVESNAPSHOT_TACTICALMAP_BUILDER_H

