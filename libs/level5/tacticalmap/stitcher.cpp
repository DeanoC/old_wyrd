#include "stitcher.h"
#include <unordered_map>
#include <algorithm>

Math::vec3 TacticalMapStitcher::rotateVector(Math::vec3 const& v, int rotationInDegrees_)
{
	if(rotationInDegrees_ < 0)
	{
		rotationInDegrees_ = 360 - ((-rotationInDegrees_ )% 360);
	}

	switch(rotationInDegrees_ % 360)
	{
		case 0: return v;
		case 90: return { v.z, v.y, -v.x };
		case 180: return -v;
		case 270:
			return { -v.z, v.y, v.x };
		default: assert(false); return {0,0,0};
	}
}

Geometry::AABB TacticalMapStitcher::rotateAABB(Geometry::AABB const& v, int rotationInDegrees_)
{
	if (rotationInDegrees_ < 0)
	{
		rotationInDegrees_ = 360 - ((-rotationInDegrees_) % 360);
	}

	switch (rotationInDegrees_ % 360)
	{
	case 0: return v;
	case 90: return v.transform(Math::rotate(Math::identity<Math::mat4x4>(), 90.0f, Math::vec3(0,1,0)));
	case 180: return v.transform(Math::rotate(Math::identity<Math::mat4x4>(), 180.0f, Math::vec3(0,1,0)));
	case 270: return v.transform(Math::rotate(Math::identity<Math::mat4x4>(), 270.0f, Math::vec3(0,1,0)));
	default: assert(false); return v;
	}
}
void TacticalMapStitcher::addTacticalMapInstance(TacticalMap::ConstPtr map_, Math::vec3 const position_, int rotationInDegrees_, int mapParcelId_)
{
	assert(mapParcelId_ <= 0xFF);
	instances.emplace_back(map_, position_, rotationInDegrees_, mapParcelId_);
}
TacticalMap::Ptr TacticalMapStitcher::build()
{
	// determine size of the stitched together map
	static const float fmininit = std::numeric_limits<float>::max();
	static const float fmaxinit = -std::numeric_limits<float>::max();
	Math::vec3 totalMinExtents {fmininit, fmininit, fmininit};
	Math::vec3 totalMaxExtents {fmaxinit, fmaxinit, fmaxinit};
	size_t levelCount = 0;
	uint32_t tacticalLevelDataSize = 0;

	for(auto[map, position, rotationInDegrees, _] : instances)
	{
		Geometry::AABB aabb = map->getAABB();
		Math::vec3 minExtent = position + aabb.getMinExtent();
		Math::vec3 maxExtent = position + aabb.getMaxExtent();
		totalMinExtents = Math::min( totalMinExtents, minExtent);
		totalMaxExtents = Math::max( totalMaxExtents, maxExtent);
		levelCount += map->levelCount;
		if(tacticalLevelDataSize != 0)
		{
			assert(map->sizeOfTacticalLevelData == tacticalLevelDataSize);
		}
		tacticalLevelDataSize = std::max(tacticalLevelDataSize, map->sizeOfTacticalLevelData);
	}

	int width =  (int)floor(totalMaxExtents.x - totalMinExtents.x);
	int height = (int)floor(totalMaxExtents.z - totalMinExtents.z);

	size_t const levelMemorySize = sizeof(TacticalMapTileLevel) * levelCount;
	size_t const levelDataMemorySize = tacticalLevelDataSize * levelCount;
	size_t const mapMemorySize = sizeof(TacticalMapTile) * width * height;
	size_t const memorySize =	sizeof(TacticalMap) + 
								levelMemorySize + 
								levelDataMemorySize + 
								mapMemorySize;

	auto const bigmemory = (uint8_t*) malloc(memorySize);
	auto const biglevels = (TacticalMapTileLevel*)(bigmemory + sizeof(TacticalMap));
	auto const biglevelDatasByte = ((uint8_t*)biglevels) + levelMemorySize;
	auto const bigmap = (TacticalMapTile*)(biglevelDatasByte + levelDataMemorySize);

	std::memset(biglevels, 0, levelMemorySize);
	std::memset(biglevelDatasByte, 0, levelDataMemorySize);
	std::memset(bigmap, 0, mapMemorySize);

	TacticalMapTileLevel* curLevels = biglevels;
	uint8_t* curLevelDatasBytes = biglevelDatasByte;
	TacticalMapTile* curMap = bigmap;

	for(auto[map, position, rotationInDegrees, mapParcelId] : instances)
	{
		Geometry::AABB aabb = map->getAABB();
		Math::vec3 minExtent = position + aabb.getMinExtent();
		Math::vec3 maxExtent = position + aabb.getMaxExtent();

		// copy level and level data
		std::memcpy( curLevels, map->levels, map->levelCount * sizeof(TacticalMapTileLevel));
		std::memcpy( curLevelDatasBytes, map->levelDataHeap, map->levelCount * tacticalLevelDataSize);

		// relocate tiles to the correct orientation and position on the big map
		// also fixup pointers from the parcel map to the new big map

		Math::vec3 const bigMapPos = (minExtent - totalMinExtents);

		int dsy = (int)std::floor(bigMapPos.z);
		assert(dsy < height);
		for(auto y = 0u; y < map->height; ++y)
		{
			int dsx = (int)floor(bigMapPos.x);
			assert(dsx < width);
			for(auto x = 0u; x < map->width; ++x)
			{
				TacticalMap::TileCoord_t ssx = 0;
				TacticalMap::TileCoord_t ssy = 0;

				Math::vec3 v{ (float)x + map->getBottomLeft().x,  0, (float)y + map->getBottomLeft().y };
				Math::vec3 local = rotateVector( v, rotationInDegrees);
				map->worldToLocal(local, ssx, ssy);

				int const si = (int)ssy * (int)map->width + (int)ssx;
				int const di = (int)dsy * (int)width + (int)dsx;
				TacticalMapTile const* src = &map->map[si];
				TacticalMapTile* dest = &bigmap[di];

				// pre fixup source check
				for (auto levelChk = 0u; levelChk < src->levelCount; levelChk++)
				{
					auto srcLevelData = map->getLevelData(*src, levelChk);
					assert(srcLevelData->levelNum == levelChk);
				}

				// copy it from src to dest (another check for luck)
				assert((uint8_t*)dest < bigmemory + memorySize);
				std::memcpy(dest, src, sizeof(TacticalMapTile));

				if (dest->levelCount != 0)
				{
					// pre fixup check that dest came over okay
					auto curLevel = curLevels[dest->levelStartIndex];
					auto curLevelData = (TacticalMapLevelDataHeader*)(curLevelDatasBytes + (dest->levelStartIndex * tacticalLevelDataSize));
					assert(curLevelData->levelNum == 0);

					// do the fixup
					dest->levelStartIndex = uint32_t(curLevels - biglevels) + dest->levelStartIndex;

					for (auto levelChk = 0u; levelChk < dest->levelCount; levelChk++)
					{
						auto dstLevelDataBytes = biglevelDatasByte +
								((dest->levelStartIndex + levelChk) * tacticalLevelDataSize);
						auto dstLevelData = (TacticalMapLevelDataHeader*) dstLevelDataBytes;
						dstLevelData->instance = mapParcelId;
						// post fixup check
						assert(dstLevelData->levelNum == levelChk);
					}
				}
				dsx++;
			}
			dsy++;
		}
		curLevels += map->levelCount;
		curLevelDatasBytes += map->levelCount * tacticalLevelDataSize;
	}

	TacticalMap* tmap = new(bigmemory) TacticalMap();
	tmap->width = width;
	tmap->height = height;
	tmap->levelCount = (uint32_t) levelCount;
	tmap->bottomLeft = { totalMinExtents.x, totalMinExtents.z };
	tmap->minHeight = totalMinExtents.y;
	tmap->maxHeight = totalMaxExtents.y;
	tmap->sizeOfTacticalMapTile = sizeof(TacticalMapTile);
	tmap->sizeOfTacticalMapTileLevel = sizeof(TacticalMapTileLevel);
	tmap->sizeOfTacticalLevelData = tacticalLevelDataSize;
	tmap->levels = biglevels;
	tmap->map = bigmap;
	tmap->levelDataHeap = biglevelDatasByte;

	return std::shared_ptr<TacticalMap>(tmap,
										[](TacticalMap* ptr)
										{
											free(ptr);
										});
}