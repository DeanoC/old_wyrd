//
// Created by Computer on 01/08/2018.
//

#ifndef NATIVESNAPSHOT_TACTICALMAP_H
#define NATIVESNAPSHOT_TACTICALMAP_H

#include "core/core.h"
#include "math/vector_math.h"
#include "geometry/aabb.h"
#include "core/utils.h"
#include <memory>
#include <vector>
#include <functional>

namespace MeshMod { class Mesh; using MeshPtr = std::shared_ptr<Mesh>; }
namespace Binny { class Bundle; class WriteHelper; };

enum class Cardinal : uint8_t
{
	Below = 0,
	Above,
	Left,
	Right,
	Front,
	Back,
};

constexpr auto Flip(Cardinal s1) -> Cardinal
{
	switch (s1)
	{
	case Cardinal::Front:	return Cardinal::Back;
	case Cardinal::Back:	return Cardinal::Front;
	case Cardinal::Left:	return Cardinal::Right;
	case Cardinal::Right:	return Cardinal::Left;
	case Cardinal::Above:	return Cardinal::Below;
	case Cardinal::Below:	return Cardinal::Above;
	default: return Cardinal::Back;
	}
}

inline auto ToVector(Cardinal s1) -> Math::vec3
{
	using namespace Math;
	switch (s1)
	{
	case Cardinal::Above: return vec3(0, 1, 0);
	case Cardinal::Below: return vec3(0, -1, 0);
	case Cardinal::Left: return vec3(-1, 0, 0);
	case Cardinal::Right: return vec3(1, 0, 0);
	case Cardinal::Front: return vec3(0, 0, 1);
	case Cardinal::Back: return vec3(0, 0, -1);
	default: return vec3(0, 0, 0);
	}
}

inline auto ToCardinal(Math::vec3 v) -> Cardinal
{
	if (std::abs(v.x) > std::abs(v.y))
	{
		if (std::abs(v.x) > std::abs(v.z))
		{
			// x major
			if (v.x < 0) return Cardinal::Left;
			else return Cardinal::Right;
		}
		else
		{
			// z major
			if (v.z < 0) return Cardinal::Back;
			else return Cardinal::Front;
		}
	}
	else if (std::abs(v.y) > std::abs(v.z))
	{
		// y major
		if (v.y < 0) return Cardinal::Below;
		else return Cardinal::Above;
	}
	else
	{
		// z major
		if (v.z < 0) return Cardinal::Back;
		else return Cardinal::Front;
	}
}
// shared with managed code structures (managed struct should be sequentially annotated)
enum class StructuralType : uint8_t
{
	NotStructural = 0,
	World, // a foundation structural bond type used for world bonds
	Floor, // floors and ceilings are the same for structure
	Wall
};

struct TacticalMapVolume
{
	float volume;
	Math::vec3 floorNormal;
	float levelHeight;
	Math::vec3 roofNormal;
	float roofHeight;
};

enum TacticalMapLevelFlags
{
	Destructable = Core::Bit(0),
	Structural = Core::Bit(1),
	Destroyed = Core::Bit(2),
	RoofValid = Core::Bit(3),
};

// This is the header to a mostly opaque structure to the managed code
// However the entire structure (size is passed to builder) must
// be stored and retrieved at the managed code side request
struct TacticalMapLevelDataHeader
{
	uint32_t flags : 24;
	uint32_t instance : 8;
	uint32_t nameCrc;

	uint8_t layer;
	uint8_t structuralIntegrity;
	StructuralType structuralType;
	uint8_t levelNum;
};

// native only structures
struct TacticalMapTileLevel
{
	Math::Plane floorPlane;
	Math::Plane roofPlane;

	float baseHeight;
	float roofDeltaHeight;
	uint32_t padd[2];
	void write(Binny::WriteHelper& helper);
};
static_assert(sizeof(TacticalMapTileLevel) == 48);

struct TacticalMapTile
{
	uint32_t levelCount;
	uint32_t levelStartIndex;

	void write(Binny::WriteHelper& h_, TacticalMapTileLevel* base_);
};
static_assert(sizeof(TacticalMapTile) == 8);


// interface and classes

// builder makes tactical map from mesha and boxes
struct ITacticalMapBuilder
{
	using Ptr = std::shared_ptr<ITacticalMapBuilder>;

	virtual ~ITacticalMapBuilder() = default;

	virtual void setMinimumHeight(float height_) = 0;
	virtual void setMaximumFloorInclination(float angleInRadians_) = 0;
	virtual void setLevelDataSize(uint32_t size_) = 0;
	virtual void addMeshAt(MeshMod::MeshPtr const& mesh, TacticalMapLevelDataHeader const* levelData, Math::mat4x4 const& transform) = 0;
	virtual void addBoxAt(Geometry::AABB const& box, TacticalMapLevelDataHeader const* levelData, Math::mat4x4 const& transform) = 0;
	virtual std::shared_ptr<class TacticalMap> build() = 0;
};

// tactical map stiches map/parcels together into one big map
struct ITacticalMapStitcher
{
	using Ptr = std::shared_ptr<ITacticalMapStitcher>;
	virtual ~ITacticalMapStitcher() {};

	virtual void addTacticalMapInstance(std::shared_ptr<class TacticalMap const> map_, Math::vec3 const position, int rotationInDegrees_, int mapParcelId_) = 0;
	virtual std::shared_ptr<class TacticalMap> build() = 0;
};

class TacticalMap
{
public:
	friend class TacticalMapBuilder;
	friend class TacticalMapStitcher;

	using TileCoord_t = int16_t;
	using LevelDataPair = std::pair<TacticalMapTileLevel*, TacticalMapLevelDataHeader*>;
	using ConstLevelDataPair = std::pair<TacticalMapTileLevel const*, TacticalMapLevelDataHeader const*>;

	static bool createFromStream(std::istream& in, std::vector<std::shared_ptr<TacticalMap>>& out_);

	static ITacticalMapBuilder::Ptr allocateBuilder(Math::vec2 const bottomLeft_, TileCoord_t width_, TileCoord_t height_);
	static ITacticalMapStitcher::Ptr allocateStitcher();

	int getWidth() const { return (int)width; }
	int getHeight() const { return (int)height; }
	Math::vec2 const getBottomLeft() const { return bottomLeft; }

	Math::vec2 worldToLocal( Math::vec3 const& world ) const;
	void worldToLocal(Math::vec3 const& world, TileCoord_t& outX, TileCoord_t &outY) const;

	Math::vec3 localToWorld(TileCoord_t const x, TileCoord_t const y) const;

	LevelDataPair mutateLookupAtWorld(Math::vec3 const& world_, float const range_, uint32_t const levelMask_);

	ConstLevelDataPair lookupAtWorld(Math::vec3 const& world_, float const range_, uint32_t const levelMask_) const;

 	bool lookupVolumeAtWorld(Math::vec3 const& world_, float const range_, uint32_t const levelMask_, TacticalMapVolume* out_) const;

	bool lookupLevelDataAtWorld(Math::vec3 const& world_, float const range_, uint32_t const levelMask_, TacticalMapLevelDataHeader* out_) const;

	void damageStructure(Geometry::AABB const& box);

	TacticalMapTile const& getTile(int x, int z) const { return map[z * width + x]; }
	TacticalMapTile& getTile(int x, int z) { return map[z * width + x]; }

	TacticalMapTileLevel const& getLevel(TacticalMapTile const& tile_, uint32_t levelIndex_) const
	{
		assert(levelIndex_ < tile_.levelStartIndex + tile_.levelCount);
		return levels[tile_.levelStartIndex + levelIndex_];
	}

	TacticalMapTileLevel& getLevel(TacticalMapTile const& tile_, uint32_t levelIndex_)
	{
		assert(levelIndex_ < tile_.levelStartIndex +  tile_.levelCount);
		return levels[tile_.levelStartIndex + levelIndex_];
	}

	/// save to an empty byte vector
	bool saveTo(uint64_t const regenMarker, std::vector<uint8_t>& result);

	Geometry::AABB getAABB() const {
		return Geometry::AABB(Math::vec3(bottomLeft.x, minHeight, bottomLeft.y),
						  Math::vec3(bottomLeft.x + width, maxHeight, bottomLeft.y + height));
	}

private:
	TacticalMapLevelDataHeader* getLevelData(TacticalMapTile const& tile_, uint32_t levelIndex_)
	{
		assert(levelIndex_ < tile_.levelStartIndex + tile_.levelCount);
		return (TacticalMapLevelDataHeader*) (levelDataHeap+((tile_.levelStartIndex + levelIndex_) * sizeOfTacticalLevelData));
	}
	TacticalMapLevelDataHeader const* getLevelData(TacticalMapTile const& tile_, uint32_t levelIndex_) const
	{
		assert(levelIndex_ < tile_.levelStartIndex + tile_.levelCount);
		return (TacticalMapLevelDataHeader const*)(levelDataHeap + ((tile_.levelStartIndex + levelIndex_) * sizeOfTacticalLevelData));
	}

	static const uint16_t MajorVersion = 6;
	static const uint16_t MinorVersion = 0;

	TacticalMap() {};
	~TacticalMap() = delete; // destructor are never run, memory is just released

	uint16_t sizeOfTacticalMapTile;
	uint16_t sizeOfTacticalMapTileLevel;
	uint32_t sizeOfTacticalLevelData;

	uint16_t width, height;
	uint16_t padd0, padd1;

	uint32_t levelCount = 0;
	Math::vec2 bottomLeft;
	float minHeight, maxHeight;

	TacticalMapTileLevel* levels = nullptr;
	TacticalMapTile* map  = nullptr;
	uint8_t* levelDataHeap = nullptr;
};

inline Math::vec2 TacticalMap::worldToLocal( Math::vec3 const& world ) const
{
	return Math::vec2((world.x - bottomLeft.x), (world.z - bottomLeft.y));
}

inline void TacticalMap::worldToLocal(Math::vec3 const& world, TileCoord_t& outX, TileCoord_t &outY) const
{
	Math::vec2  local = worldToLocal(world) + Math::vec2(0.5f,0.5f);
	local = Math::clamp(local, Math::vec2(0, 0), Math::vec2((float)width - 1, (float)height - 1));
	outX = (TileCoord_t) std::floor(local.x);
	outY = (TileCoord_t) std::floor(local.y);
}

inline Math::vec3 TacticalMap::localToWorld(TileCoord_t const x, TileCoord_t const y) const
{

	Math::vec2 local{ (float)x + bottomLeft.x, (float)y + bottomLeft.y };
	local -= Math::vec2(0.5f, 0.5f);
	return { local.x, 0, local.y };
}

#endif
//NATIVESNAPSHOT_TACTICALMAP_H
