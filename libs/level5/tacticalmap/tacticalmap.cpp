#include "core/core.h"
#include "meshmod/vertices.h"
#include "meshmod/halfedges.h"
#include "meshmod/polygons.h"
#include "meshmod/mesh.h"
#include "meshops/basicmeshops.h"
#include "geometry/ray.h"
#include "geometry/watertightray.h"
#include "enkiTS/src/TaskScheduler.h"
#include "meshops/layeredtexture.h"
#include "builder.h"
#include "stitcher.h"
#include "tacticalmap.h"
#include "binny/bundle.h"
#include "binny/bundlewriter.h"
#include <algorithm>
#include <sstream>
#include <array>
#include <unordered_set>
#include <stack>

namespace {
using namespace Binny;
static const uint32_t TacticalMapId = "TACM"_bundle_id;
}
extern enki::TaskScheduler g_EnkiTS;

ITacticalMapBuilder::Ptr TacticalMap::allocateBuilder(Math::vec2 const bottomLeft_, TileCoord_t width_,
													  TileCoord_t height_)
{
	return std::make_shared<TacticalMapBuilder>(bottomLeft_, width_, height_);
}

ITacticalMapStitcher::Ptr TacticalMap::allocateStitcher()
{
	return std::make_shared<TacticalMapStitcher>();
}

TacticalMap::ConstLevelDataPair TacticalMap::lookupAtWorld(
		Math::vec3 const& world_,
		float const range_, uint32_t
		const levelMask_) const
{

	TileCoord_t x, y;
	worldToLocal(world_, x, y);
	auto const& tile = map[y * getWidth() + x];
	if(tile.levelCount == 0) { return {}; }

	float const bottomY = world_.y - range_;
	float const topY = world_.y + range_;

	for(auto levelIndex = 0u; levelIndex < tile.levelCount; ++levelIndex)
	{
		TacticalMapTileLevel const& level = getLevel(tile, levelIndex);
		TacticalMapLevelDataHeader const* levelData = getLevelData(tile, levelIndex);

		if(levelData->flags & TacticalMapLevelFlags::Destroyed) continue;
		if((Core::Bit(levelData->layer) & levelMask_) == 0) continue;

		assert(levelData->levelNum == levelIndex);
		assert(!std::isnan(level.baseHeight));
		assert(level.baseHeight < 1e6f);

		float const bottomLevel = level.baseHeight;
		float const topLevel = (levelData->flags & TacticalMapLevelFlags::RoofValid) ?
							   level.baseHeight + level.roofDeltaHeight : std::numeric_limits<float>::infinity();

		// level is above the search range, stop the search
		if(topY < bottomLevel) return {};

		// level is below the search range, continue search to next level
		//		if (bottomY > topLevel ) continue;
		if(bottomY > bottomLevel) continue;

		return {&level, levelData};
	}

	// search range is beyond the top layer, shouldn't really happen i think...
	// but return the topmost layer
	TacticalMapTileLevel const& level = getLevel(tile, tile.levelCount - 1);
	TacticalMapLevelDataHeader const* levelData = getLevelData(tile, tile.levelCount - 1);

	return {&level, levelData};
}

TacticalMap::LevelDataPair TacticalMap::mutateLookupAtWorld(
		Math::vec3 const& world_,
		float const range_,
		uint32_t const levelMask_)
{
	auto[level, levelData] = lookupAtWorld(world_, range_, levelMask_);
	return {const_cast<TacticalMapTileLevel*>(level), const_cast<TacticalMapLevelDataHeader*>(levelData)};
}

bool TacticalMap::lookupVolumeAtWorld(
		Math::vec3 const& world_,
		float const range_,
		uint32_t const levelMask_,
		TacticalMapVolume* out_) const
{
	assert(out_ != nullptr);

	auto[level, levelData] = lookupAtWorld(world_, range_, levelMask_);
	if(level == nullptr || levelData == nullptr) return false;

	constexpr float finf = std::numeric_limits<float>::infinity();
	float const bottomLevel = level->baseHeight;
	float const topLevel = level->baseHeight +
						   (levelData->flags & TacticalMapLevelFlags::RoofValid) ? level->roofDeltaHeight : finf;

	out_->floorNormal = level->floorPlane.normal();
	out_->levelHeight = bottomLevel;
	out_->roofNormal = Math::vec3(0, 1, 0);
	if(levelData->flags & TacticalMapLevelFlags::RoofValid) level->roofPlane.normal();

	out_->roofHeight = topLevel;
	return true;
}

bool TacticalMap::lookupLevelDataAtWorld(
		Math::vec3 const& world_,
		float const range_,
		uint32_t const levelMask_,
		TacticalMapLevelDataHeader* out_) const
{
	assert(out_ != nullptr);

	auto[level, levelData] = lookupAtWorld(world_, range_, levelMask_);
	if(level == nullptr || levelData == nullptr) return false;

	std::memcpy(out_, levelData, sizeOfTacticalLevelData);

	return true;
}

void TacticalMap::damageStructure(Geometry::AABB const& box)
{
	TileCoord_t ixmin, izmin;
	TileCoord_t ixmax, izmax;

	float const ymin = box.getMinExtent().y;
	float const ymax = box.getMaxExtent().y;

	worldToLocal(box.getMinExtent(), ixmin, izmin);
	worldToLocal(box.getMaxExtent(), ixmax, izmax);

	// fatten up slivers
	if(ixmin == ixmax)
	{
		if(ixmin > 0) ixmin--;
		else ixmax++;
	}
	if(izmin == izmax)
	{
		if(izmin > 0) izmin--;
		else izmax++;
	}

	ixmin = Math::clamp<TileCoord_t>(ixmin, 0, width);
	izmin = Math::clamp<TileCoord_t>(izmin, 0, height);
	ixmax = Math::clamp<TileCoord_t>(ixmax, 0, width);
	izmax = Math::clamp<TileCoord_t>(izmax, 0, height);

	using CoordPair = std::pair<TileCoord_t, TileCoord_t>;

	std::stack<CoordPair> tileStack;
	// find tiles inside the initial explosion box and place onto stack
	for(auto z = izmin; z < izmax; ++z)
	{
		for(auto x = ixmin; x < ixmax; ++x)
		{
			auto const& tile = map[z * getWidth() + x];
			auto levelIndex = 0u;
			for(; levelIndex < tile.levelCount; ++levelIndex)
			{
				TacticalMapTileLevel& level = getLevel(tile, levelIndex);
				TacticalMapLevelDataHeader* levelData = getLevelData(tile, levelIndex);

				if(levelData->flags & TacticalMapLevelFlags::Destroyed) continue;

				Math::vec3 const mincentre(0, ymin - level.baseHeight, 0);
				Math::vec3 const maxcentre(0, ymax - level.baseHeight, 0);

				// distance from floor and roof
				auto const& floorPlane = levels[levelIndex].floorPlane;
				float const minFloorD = Math::DotPoint(floorPlane, mincentre);
				float const maxFloorD = Math::DotPoint(floorPlane, maxcentre);

				// minimum above floor 
				if(minFloorD >= 0.0f)
				{
					// but floor below maximum
					if(maxFloorD < 0.0f)
					{
						tileStack.push({x, z});
						break;
					}
				}
			}
		}
	}

	using namespace Math;

	std::unordered_set<uint64_t> doneTiles;
	// follow collapse chains up and around if structural integrity has failed	
	while(!tileStack.empty())
	{
		auto[x, z] = tileStack.top();
		tileStack.pop();
		x = clamp(x, TileCoord_t(0), TileCoord_t(getWidth() - 1));
		z = clamp(z, TileCoord_t(0), TileCoord_t(getHeight() - 1));

		// ignore if tile has already been processed once in this collapse chain
		if(auto const mXZ = MortonCurve(x, z);
				doneTiles.find(mXZ) != doneTiles.end())
			continue;
		else doneTiles.insert(mXZ);

		auto const& tile = map[z * getWidth() + x];
		auto levelIndex = 0u;
		for(; levelIndex < tile.levelCount; ++levelIndex)
		{
			TacticalMapTileLevel& level = getLevel(tile, levelIndex);
			TacticalMapLevelDataHeader* levelData = getLevelData(tile, levelIndex);
			if(levelData->flags & TacticalMapLevelFlags::Destroyed) continue;

			Math::vec3 const mincentre(0, ymin - level.baseHeight, 0);

			// distance from floor and roof
			auto const& floorPlane = levels[levelIndex].floorPlane;
			float const minFloorD = Math::DotPoint(floorPlane, mincentre);

			// minimum above floor 
			if(minFloorD >= 0.0f)
			{
				if(levelData->structuralType == StructuralType::NotStructural) continue;
				if(levelData->structuralType == StructuralType::World) continue;

				levelData->structuralIntegrity--;

				// has structural integrity failed?
				if(levelData->structuralIntegrity < 2)
				{

					// test connected tiles
					int16_t const xm1 = x - 1;
					int16_t const xp1 = x + 1;
					int16_t const zm1 = z - 1;
					int16_t const zp1 = z + 1;

					if(auto const mXZ = MortonCurve(xm1, z);
							doneTiles.find(mXZ) == doneTiles.end())
						tileStack.push({xm1, z});
					if(auto const mXZ = MortonCurve(xp1, z);
							doneTiles.find(mXZ) == doneTiles.end())
						tileStack.push({xp1, z});
					if(auto const mXZ = MortonCurve(x, zm1);
							doneTiles.find(mXZ) == doneTiles.end())
						tileStack.push({x, zm1});
					if(auto const mXZ = MortonCurve(x, zp1);
							doneTiles.find(mXZ) == doneTiles.end())
						tileStack.push({x, zp1});
				} else
				{
					// stop further vertical collapse
					continue;
				}
			}
		}
	}
}

void TacticalMapTileLevel::write(Binny::WriteHelper& h)
{
	using namespace Binny;
	using namespace std::string_literals;


	h.write(floorPlane.a, floorPlane.b, floorPlane.c, floorPlane.d, "Floor Plane"s);
	h.write(roofPlane.a, roofPlane.b, roofPlane.c, roofPlane.d, "Roof Plane"s);
	h.write(baseHeight, "Base Height");
	h.write(roofDeltaHeight, "Roof Delta Height");
	h.write_as<uint32_t>(0xDE, "Padd");
	h.write_as<uint32_t>(0xDE, "Padd");
}

void TacticalMapTile::write(Binny::WriteHelper& h, TacticalMapTileLevel* base_)
{
	using namespace Binny;
	using namespace std::string_literals;

	h.write(levelCount, "levels in this tile"s);
	h.write(levelStartIndex, "start index of this tiles level array"s);
}

bool TacticalMap::saveTo(uint64_t const regenMarker, std::vector<uint8_t>& result)
{
	using namespace Binny;
	using namespace std::string_literals;
	BundleWriter writer;
	writer.setLogBinifyText();

	bool okay;
	okay = writer.addChunk(
			"TacticalMap"s,
			TacticalMapId,
			MajorVersion,
			MinorVersion,
			0,
			{},
			[this](WriteHelper& h)
			{
				h.allow_nan(false);

				h.allow_infinity(false);
				h.add_enum("TacticalMapTileLevel"s);
				h.add_enum_value("TacticalMapTileLevel"s, "Destroyed", TacticalMapLevelFlags::Destroyed);
				h.add_enum_value("TacticalMapTileLevel"s, "RoofValid", TacticalMapLevelFlags::RoofValid);
				h.add_enum_value("TacticalMapTileLevel"s,
								 "Destructable",
								 TacticalMapLevelFlags::Destructable);
				h.add_enum_value("TacticalMapTileLevel"s,
								 "Structural",
								 TacticalMapLevelFlags::Structural);

				// header
				h.write_as<uint16_t>(sizeOfTacticalMapTile,
									 "sizeof TacticalMapTile when this was built");
				h.write_as<uint16_t>(sizeOfTacticalMapTileLevel,
									 "sizeof TacticalMapTileLevel when this was built");
				h.write_as<uint32_t>(sizeOfTacticalLevelData,
									 "sizeof Tactical Level Data used for this map");
				h.write_as<uint16_t>(width, "map width"s);
				h.write_as<uint16_t>(height, "map height"s);
				h.write_as<uint16_t>(0xDE, "padd0"s);
				h.write_as<uint16_t>(0xDE, "padd0"s);

				h.write(levelCount, "layer count"s);
				h.write(bottomLeft.x, "bottom left x"s);
				h.write(bottomLeft.y, "bottom left y"s);
				h.write(minHeight, "min height of map");
				h.write(maxHeight, "max height of map");
				h.align(8);

				h.use_label("Levels"s, ""s, true, true, "ptr to beginning of the level structures"s);
				h.use_label("Map"s, ""s, true, true, "ptr to 2D tile map data"s);
				h.use_label("LevelDataHeap"s, ""s, true, true, "ptr to heap used to store level data"s);

				h.align();
				// levels
				h.write_label("Levels"s, false);
				for(auto i = 0u; i < levelCount; ++i)
				{
					TacticalMapTileLevel& level = levels[i];
					h.write_label("LevelEntry"s + std::to_string(i), true);
					level.write(h);
				}

				h.align();
				h.write_label("Map"s, false);
				for(auto i = 0; i < width * height; ++i)
				{
					TacticalMapTile& tile = map[i];
					tile.write(h, levels);
				}

				h.align();
				h.write_label("LevelDataHeap"s, false);
				h.write_byte_array(levelDataHeap, levelCount * sizeOfTacticalLevelData);
			}
	);
	if(!okay) return false;

	okay = writer.build(regenMarker, result);
	if(!okay) return false;
	return true;
}

bool TacticalMap::createFromStream(std::istream& in, std::vector<std::shared_ptr<TacticalMap>>& out_)
{
	using namespace Binny;
	std::vector<IBundle::ChunkHandler> handlers = {
			{{TacticalMapId, 0, 0,
					 [&out_](std::string_view, int, uint16_t majorVersion_, uint16_t minorVersion_, size_t size_,
							 std::shared_ptr<void> ptr_) -> bool
					 {
						 auto tmap = std::static_pointer_cast<TacticalMap>(ptr_);

						 if(majorVersion_ != MajorVersion) return false;
						 if(minorVersion_ > MinorVersion) return false;
						 if(sizeof(TacticalMapTile) != tmap->sizeOfTacticalMapTile) return false;
						 if(sizeof(TacticalMapTileLevel) != tmap->sizeOfTacticalMapTileLevel) return false;
						 if(sizeof(TacticalMapLevelDataHeader) > tmap->sizeOfTacticalLevelData) return false;

						 // verify remapping occured okay
						 for(auto y = 0; y < tmap->getHeight(); ++y)
						 {
							 for(auto x = 0; x < tmap->getWidth(); ++x)
							 {
								 TacticalMapTile const& tile = tmap->getTile(x, y);
								 for(auto levelChk = 0u; levelChk < tile.levelCount; levelChk++)
								 {
									 auto const& level = tmap->getLevel(tile, levelChk);
									 auto levelData = tmap->getLevelData(tile, levelChk);
									 if(levelData->levelNum != levelChk)
									 {
										 return false;
									 }
								 }
							 }
						 }

						 out_.emplace_back(tmap);

						 return true;
					 }
			 }}
	};

	Bundle bundle(&malloc, &free, &malloc, &free, in);
	auto const ret = bundle.read({}, handlers);
	if(ret.first != Bundle::ErrorCode::Okay)
	{
		return false;
	}

	return true;

}