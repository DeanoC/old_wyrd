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
#include "tacticalmap/builder.h"

#include <algorithm>
#include <tuple>
#include <array>

extern enki::TaskScheduler g_EnkiTS;

TacticalMapBuilder::TacticalMapBuilder( Math::vec2 const bottomLeft_,
										TileCoord_t width_,
										TileCoord_t height_ ) :
		bottomLeft( std::floor(bottomLeft_.x + 0.5f), std::floor(bottomLeft_.y + 0.5f)),
		width( width_ ),
		height( height_ ),
		tileBuilders( width_ * height_ )
{
}

TacticalMapBuilder::~TacticalMapBuilder()
{
	tacticalLevelDataHeap.clear();
	smoothLayerTexture.reset();
	globalFragmentTexture.reset();
	tileBuilders.clear();
	solids.clear();
}

TacticalMapTileBuilder::~TacticalMapTileBuilder()
{
	layers.clear();
	bonds.clear();
	structuralBoxs.clear();
	destructables.clear();
	boxes.clear();
	polygons.clear();
}

TacticalMap::Ptr TacticalMapBuilder::build()
{
	using namespace MeshMod;

	VertexIndexContainer polyIndices;
	polyIndices.reserve(10);

	// work out which polygons from which mesh intersect which tile
	// ultimately rasterasation would be better but current just using bounding area of polys
	for (size_t i = 0; i < solids.size(); i++)
	{
		auto& [mesh, box, levelDataOffset] = solids[i];
		auto levelData = (TacticalMapLevelDataHeader*)(tacticalLevelDataHeap.data() + levelDataOffset);

		if(mesh)
		{
			polyIndices.clear();
			auto localMesh = mesh;

			mesh->getPolygons().visitValid(
					[this, i, localMesh, &polyIndices]( PolygonIndex polygonIndex )
					{
						polyIndices.clear();
						Vertices const& vertices = localMesh->getVertices();
						Polygons const& polygons = localMesh->getPolygons();

						Geometry::AABB polyBox;
						polygons.getVertexIndices( polygonIndex, polyIndices );
						for(auto vi : polyIndices)
						{
							auto const vpos = vertices.position( vi );
							Math::vec3 pos( vpos.x, vpos.y, vpos.z );
							polyBox.expandBy( pos );
						}

						insertBox( polyBox,
								   [i, polygonIndex]( TacticalMapTileBuilder& tileBuilder )
								   {
									   tileBuilder.polygons[i].insert( polygonIndex );
								   } );
					} );
			if(levelData->flags & TacticalMapLevelFlags ::Destructable)
			{
				Geometry::AABB localBox = box;
				insertBox( box,
						   [i, localBox]( TacticalMapTileBuilder& tileBuilder )
						   {
							   tileBuilder.destructables[i] = localBox;
						   } );
			}
		} else
		{
			Geometry::AABB localBox = box;
			size_t localLevelDataOffset = levelDataOffset;
			insertBox( box,
					   [this, i, localBox, localLevelDataOffset]( TacticalMapTileBuilder& tileBuilder )
					   {
							auto levelData = (TacticalMapLevelDataHeader*)(tacticalLevelDataHeap.data() + localLevelDataOffset);

						   tileBuilder.boxes[i] = localBox;
						   if(levelData->flags & TacticalMapLevelFlags ::Destructable)
						   {
							   tileBuilder.destructables[i] = localBox;
						   }
					   } );

		}
	}

	// start to order and map things in this tile
	generateLayers();

	//------- now generate the actual tactical map
	// calculate size of memory chunk we need to allocate
	// count builder layers so we do a block alloc

	uint32_t levelCount = 0;
	for (auto y = 0; y < height; ++y)
	{
		for (auto x = 0; x < width; ++x)
		{
			auto const& tileBuilder = tileBuilders[y * width + x];
			levelCount += (uint32_t)tileBuilder.layers.size();
		}
	}
	size_t const levelMemorySize = sizeof(TacticalMapTileLevel) * levelCount;
	size_t const levelDataMemorySize = tacticalLevelDataSize * levelCount;
	size_t const mapMemorySize = sizeof(TacticalMapTile) * width * height;
	size_t const memorySize = sizeof(TacticalMap) + 
								levelMemorySize + 
								levelDataMemorySize + 
								mapMemorySize;

	auto memory = (uint8_t*) malloc(memorySize);

	auto const levels = (TacticalMapTileLevel*)(memory + sizeof(TacticalMap));
	auto const levelDatasByte = ((uint8_t*)levels) + levelMemorySize;
	auto const map = (TacticalMapTile*)(levelDatasByte + levelDataMemorySize);

	assert((((uint8_t*)map) + mapMemorySize) - memory == memorySize);

	minHeight = FLT_MAX;
	maxHeight = -FLT_MAX;

	// now update the real non builder data
	auto globalLevelIndex = 0u;
	for (auto y = 0; y < height; ++y)
	{
		for (auto x = 0; x < width; ++x)
		{
			if (levelCount > 0)
			{
				assert(globalLevelIndex <= levelCount);
			}
			auto const tileIndex = y * width + x;
			auto const& tileBuilder = tileBuilders[tileIndex];

			map[tileIndex].levelCount = (uint32_t)tileBuilder.layers.size();
			if (map[tileIndex].levelCount > 0)
			{
				map[tileIndex].levelStartIndex = globalLevelIndex;
			}
			else
			{
				map[tileIndex].levelStartIndex = ~0;
			}

			for (auto layerIndex = 0u; layerIndex < tileBuilder.layers.size(); ++layerIndex)
			{
				assert(map[tileIndex].levelCount != 0);

				auto const& layer = tileBuilder.layers[layerIndex];
				auto const levelIndex = globalLevelIndex + layerIndex;
				assert(levelIndex < levelCount);
				auto& level = levels[levelIndex];
				auto* levelDataByte = levelDatasByte + ((levelIndex) * tacticalLevelDataSize);
				auto* levelData = (TacticalMapLevelDataHeader*) levelDataByte;
				assert((uint8_t*)levelData < (uint8_t*)map);

				uint32_t const floorSolidIndex = layer.floorDominantSolidIndex;
				uint32_t const ceilSolidIndex = layer.ceilDominantSolidIndex;

				uint32_t solidIndex = floorSolidIndex;
				if (floorSolidIndex == ~0)
				{
					solidIndex = ceilSolidIndex;
				}
				assert(solidIndex != ~0);

				auto const& solid = solids[solidIndex];

				level.baseHeight = layer.minHeight;
				level.floorPlane = layer.floorPlane;

				std::memcpy(levelData, tacticalLevelDataHeap.data() + solid.extraLevelDataOffset, tacticalLevelDataSize);

				assert(levelData->layer <= 31);
				levelData->flags = 0;
				levelData->instance = 0;
				levelData->levelNum = (uint8_t) layerIndex;

				assert(!std::isnan(level.baseHeight));
				assert(level.baseHeight < 1e6f);

				// world structural type trumps everything
				// none always loses
				// floor trumps walls
				StructuralType stype = StructuralType::NotStructural;
				uint8_t structuralIntegrity = 255;

				// look up the destruction data in the solids and
				// decide on the type and integrity
				for (auto solidIndex : layer.destructionSolidIndices)
				{
					auto const& sbox = tileBuilder.structuralBoxs[solidIndex];
					uint8_t si = (uint8_t)Math::clamp(sbox.structuralIntegrity, 0, 255);
					structuralIntegrity = std::min(structuralIntegrity, si);

					switch (sbox.structuralType)
					{
					case StructuralType::NotStructural: break;
					case StructuralType::Wall:
						if (stype == StructuralType::NotStructural)
						{
							stype = StructuralType::Wall;
						}
						break;
					case StructuralType::Floor:
						if (stype != StructuralType::World)
						{
							stype = StructuralType::Floor;
						}
						break;
					case StructuralType::World:
						stype = StructuralType::World;
						break;
					}
				}
				levelData->structuralType = stype;
				levelData->structuralIntegrity = structuralIntegrity;

				if (!std::isnan(layer.ceilPlane.d))
				{
					levelData->flags |= TacticalMapLevelFlags ::RoofValid;
					level.roofPlane = layer.ceilPlane;
					level.roofDeltaHeight = layer.maxHeight - layer.minHeight;
				}
				else
				{
					level.roofDeltaHeight = FLT_MAX;
					maxHeight = FLT_MAX;
				}
				minHeight = std::min(minHeight, layer.minHeight);
				maxHeight = std::max(maxHeight, layer.maxHeight);
			}
			// verify indices
			for (auto levelIndex = 0u; levelIndex < map[tileIndex].levelCount; ++levelIndex)
			{
				auto const& level = levels[map[tileIndex].levelStartIndex + levelIndex];
				auto* levelDataByte = levelDatasByte + ((map[tileIndex].levelStartIndex + levelIndex) * tacticalLevelDataSize);
				auto* levelData = (TacticalMapLevelDataHeader*) levelDataByte;
				assert(levelData->levelNum == levelIndex);
			}

			globalLevelIndex += (uint32_t)tileBuilder.layers.size();
		}
	}
	// verify
	for (auto y = 0; y < height; ++y)
	{
		for (auto x = 0; x < width; ++x)
		{
			auto const tileIndex = y * width + x;
			TacticalMapTile const& tile = map[tileIndex];
			for (auto levelChk = 0u; levelChk < tile.levelCount; levelChk++)
			{
				auto const& level = levels[tile.levelStartIndex + levelChk];
				auto* levelDataByte = levelDatasByte + ((map[tileIndex].levelStartIndex + levelChk) * tacticalLevelDataSize);
				auto* levelData = (TacticalMapLevelDataHeader*) levelDataByte;
				assert(levelData->levelNum == levelChk);
				assert(levelData->layer <= 31);
			}
		}
	}
	TacticalMap* tmap = new(memory) TacticalMap();
	tmap->width = width;
	tmap->height = height;
	tmap->levelCount = levelCount;
	tmap->bottomLeft = bottomLeft;
	tmap->minHeight = minHeight;
	tmap->maxHeight = maxHeight;
	tmap->sizeOfTacticalMapTile = sizeof(TacticalMapTile);
	tmap->sizeOfTacticalMapTileLevel = sizeof(TacticalMapTileLevel);
	tmap->sizeOfTacticalLevelData = tacticalLevelDataSize;
	tmap->levels = levels;
	tmap->map = map;
	tmap->levelDataHeap = levelDatasByte;

	return std::shared_ptr<TacticalMap>(tmap,
		[](TacticalMap* ptr)
	{
		free(ptr);
	});
}

void TacticalMapBuilder::insertBox( Geometry::AABB const& box,
									std::function<void( TacticalMapTileBuilder& )> func )
{
	TileCoord_t ixmin;
	TileCoord_t izmin;
	TileCoord_t ixmax;
	TileCoord_t izmax;

	worldToLocal( box.getMinExtent(), ixmin, izmin );
	worldToLocal( box.getMaxExtent(), ixmax, izmax );

	// fatten up slivers
	if (ixmin == ixmax)
	{
		if (ixmin > 0) ixmin--;
		else ixmax++;
	}
	if (izmin == izmax)
	{
		if (izmin > 0) izmin--;
		else izmax++;
	}

	ixmin = Math::clamp<TileCoord_t>(ixmin, 0, width);
	izmin = Math::clamp<TileCoord_t>(izmin, 0, height);
	ixmax = Math::clamp<TileCoord_t>(ixmax, 0, width);
	izmax = Math::clamp<TileCoord_t>(izmax, 0, height);

	// write into tiles
	for(auto z = izmin; z < izmax; ++z)
	{
		for(auto x = ixmin; x < ixmax; ++x)
		{
			auto& tileBuilder = tileBuilders[z * width + x];
			func( tileBuilder );
		}
	}
}

void TacticalMapBuilder::addMeshAt( MeshMod::MeshPtr const& mesh, TacticalMapLevelDataHeader const* levelData, Math::mat4x4 const& transform )
{
	using namespace MeshMod;
	Geometry::AABB wipAABB;

	auto wip = std::shared_ptr<Mesh>(mesh->clone());
	MeshOps::BasicMeshOps::transform(wip, transform);
	MeshOps::BasicMeshOps::triangulate( wip);
	MeshOps::BasicMeshOps::computeFacePlaneEquations(wip);
	MeshOps::BasicMeshOps::computeAABB( wip, wipAABB );

	assert(levelData->layer <= 31);
	assert(levelData->nameCrc != 0);

	auto heapOffset = tacticalLevelDataHeap.size();
	tacticalLevelDataHeap.resize(heapOffset + tacticalLevelDataSize);
	auto ptr = tacticalLevelDataHeap.data() + heapOffset;
	std::memcpy(ptr, levelData, tacticalLevelDataSize);
	solids.emplace_back(wip, wipAABB, heapOffset);
}

void TacticalMapBuilder::addBoxAt( Geometry::AABB const& box, TacticalMapLevelDataHeader const* levelData, Math::mat4x4 const& transform )
{
	auto heapOffset = tacticalLevelDataHeap.size();
	tacticalLevelDataHeap.resize(heapOffset + tacticalLevelDataSize);
	auto ptr = tacticalLevelDataHeap.data() + heapOffset;
	std::memcpy(ptr, levelData, tacticalLevelDataSize);
	solids.emplace_back( nullptr, box.transformAffine( transform ), heapOffset);
}

void TacticalMapBuilder::generateLayers()
{
	// phase 1 generates the height fields
	// cleans them up and generates mean and std basic statistics for outlier removal
	{
		enki::TaskSet phase1( width * height,
							  [this]( enki::TaskSetPartition range, uint32_t threadnum )
							  {
								  for(auto i = range.start; i < range.end; ++i)
								  {
									  TileCoord_t x = i % width;
									  TileCoord_t z = i / width;
									  generateHeightFragmentsForTileAt( x, z );
								  }
							  } );
//		phase1.m_MinRange = 10000;
		g_EnkiTS.AddTaskSetToPipe( &phase1 );
		g_EnkiTS.WaitforTask( &phase1 );
	}

	// phase 2 smooths the normals and heightfields across the entire map
	generateGlobalFragmentTexture();
	smoothGlobalFragmentTexture();

	// phase 3 - determine planes
	{
		enki::TaskSet phase3( width * height,
							  [this]( enki::TaskSetPartition range, uint32_t threadnum )
							  {
								  for(auto i = range.start; i < range.end; ++i)
								  {
									  TileCoord_t x = i % width;
									  TileCoord_t z = i / width;
									  generatePlanesForTileAt( x, z );
								  }
							  } );

		g_EnkiTS.AddTaskSetToPipe( &phase3 );
		g_EnkiTS.WaitforTask( &phase3 );
	}

	// phase 4 - plane level data
	{
		enki::TaskSet phase3( width * height,
							  [this]( enki::TaskSetPartition range, uint32_t threadnum )
							  {
								  for(auto i = range.start; i < range.end; ++i)
								  {
									  TileCoord_t x = i % width;
									  TileCoord_t z = i / width;
									  generatePlanesForTileAt( x, z );
								  }
							  } );

		g_EnkiTS.AddTaskSetToPipe( &phase3 );
		g_EnkiTS.WaitforTask( &phase3 );
	}

	// phase 5 - destruction
	{
		enki::TaskSet phase4( width * height,
							  [this]( enki::TaskSetPartition range, uint32_t threadnum )
							  {
								  for(auto i = range.start; i < range.end; ++i)
								  {
									  TileCoord_t x = i % width;
									  TileCoord_t z = i / width;
									  generateStructuralBoxesForTileAt( x, z );
								  }
							  } );

		g_EnkiTS.AddTaskSetToPipe( &phase4 );
		g_EnkiTS.WaitforTask( &phase4 );
	}

}

void TacticalMapBuilder::generateGlobalFragmentTexture()
{
	// construct single global layered texture for the entire map
	int const totalWidth = (int) width * fragmentSubSamples;
	int const totalHeight = (int) height * fragmentSubSamples;
	int numLayers = 0;

	// find max layers for map
	for(auto y = 0; y < height; ++y)
	{
		for(auto x = 0; x < width; ++x)
		{
			auto const& tileBuilder = tileBuilders[y * width + x];
			numLayers = std::max( numLayers, tileBuilder.maxLayers );
		}
	}

	globalFragmentTexture = std::make_unique<TacticalMapBuilder::FragmentTexture>( totalWidth, totalHeight );

	for(auto layerIndex = 0; layerIndex < numLayers; ++layerIndex)
	{
		std::string const layerName = std::string( "Layer_" ) + std::to_string( layerIndex );
		globalFragmentTexture->addLayer<TMapTBHeightFragment const*>( layerName, 2 );
	}

	enki::TaskSet task( numLayers, [&]( enki::TaskSetPartition range, uint32_t threadnum )
	{
		for(auto layerIndex = range.start; layerIndex < range.end; ++layerIndex)
		{
			for(auto z = 0; z < totalHeight; ++z)
			{
				for(auto x = 0; x < totalWidth; ++x)
				{
					auto const tileZ = z / fragmentSubSamples;
					auto const tileX = x / fragmentSubSamples;

					auto const tileIndex = tileZ * width + tileX;
					auto const& tileBuilder = tileBuilders[tileIndex];
					if(layerIndex >= tileBuilder.layers.size())
						continue;

					auto const& slayer = tileBuilder.layers[layerIndex];
					auto& heightlayer = globalFragmentTexture->getLayer(layerIndex);

					auto const sx = x - (tileX * fragmentSubSamples);
					auto const sz = z - (tileZ * fragmentSubSamples);

					auto const index = (sz * fragmentSubSamples) + sx;
					heightlayer.setAt( x, z, 0, slayer.validFloorFragments[index] );
					heightlayer.setAt( x, z, 1, slayer.validCeilFragments[index] );
				}
			}
		}
	} );

	g_EnkiTS.AddTaskSetToPipe( &task );
	g_EnkiTS.WaitforTask( &task );
}

void TacticalMapBuilder::smoothGlobalFragmentTexture()
{
	using namespace std::string_literals;
	auto const& srcTex = globalFragmentTexture;

	// clone the layer structure of the global fragment texture to a dual height map texture
	smoothLayerTexture = std::make_unique<MeshOps::LayeredTexture>( srcTex->getWidth(), srcTex->getHeight());
	for(auto i = 0u; i < srcTex->getLayerCount(); ++i)
	{
		auto const& srcLayer = srcTex->getLayer( i );
		smoothLayerTexture->addLayer<float>( std::string(srcLayer.getName()) + "_smooth"s, 2 );
		smoothLayerTexture->addLayer<uint32_t>( std::string(srcLayer.getName()) + "_solidIndex"s, 2 );
	}

	enki::TaskSet task( srcTex->getLayerCount(), [&]( enki::TaskSetPartition range, uint32_t threadnum )
	{
		for(auto layerIndex = range.start; layerIndex < range.end; ++layerIndex)
		{
			auto& slayer = srcTex->getLayer( layerIndex );
			MeshOps::ITextureLayer& heightlayer = smoothLayerTexture->getLayer(layerIndex * 2);
			MeshOps::ITextureLayer& solidlayer = smoothLayerTexture->getLayer((layerIndex * 2) + 1);

			for(int z = 0; z < (int) srcTex->getHeight(); ++z)
			{
				int const zm1 = std::max( z - 1, 0 );
				int const zp1 = std::min( z + 1, (int) srcTex->getHeight() - 1 );

				for(int x = 0; x < (int) srcTex->getWidth(); ++x)
				{
					int const xm1 = std::max( x - 1, 0 );
					int const xp1 = std::min( x + 1, (int) srcTex->getWidth() - 1 );

					std::array<std::pair<int,int>, 9> filterIndices = {
							std::pair{ zm1, xm1 }, std::pair{ zm1, x }, std::pair{ zm1, xp1 },
							std::pair{   z, xm1 }, std::pair{   z, x }, std::pair{   z, xp1 },
							std::pair{ zp1, xm1 }, std::pair{ zp1, x }, std::pair{ zp1, xp1 }
					};

					float ft = 0;
					float ct = 0;
					int fcount = 0;
					int ccount = 0;
					for(auto j = 0u; j < filterIndices.size(); ++j)
					{
						auto const [iz, ix] = filterIndices[j];
						auto floorHeight = slayer.getAt<TMapTBHeightFragment const*>(ix, iz, 0);
						auto ceilHeight = slayer.getAt<TMapTBHeightFragment const*>(ix, iz, 1);

						if(floorHeight != nullptr)
						{
							fcount++;
							ft += floorHeight->t;
							solidlayer.setAt<uint32_t>(x, z, 0, (uint32_t)floorHeight->solidIndex);
						}
						if(ceilHeight != nullptr)
						{
							ccount++;
							ct += ceilHeight->t;
							solidlayer.setAt<uint32_t>(x, z, 1, (uint32_t)ceilHeight->solidIndex);
						}
					}
					// TODO see if this average filter is okay...
					if(fcount > 0)
					{
						heightlayer.setAt( x, z, 0, ft / (float) fcount );
					} else
					{
						heightlayer.setAt( x, z, 0, std::numeric_limits<float>::quiet_NaN());
						solidlayer.setAt<uint32_t>(x, z, 0, ~0);
					}

					if(ccount > 0)
					{
						heightlayer.setAt( x, z, 1, ct / (float) ccount );
					} else
					{
						heightlayer.setAt( x, z, 1, std::numeric_limits<float>::quiet_NaN());
						solidlayer.setAt<uint32_t>(x, z, 1, ~0);
					}
				}
			}
		}
	} );

	g_EnkiTS.AddTaskSetToPipe( &task );
	g_EnkiTS.WaitforTask( &task );
}

void TacticalMapBuilder::generatePlanesForTileAt( TileCoord_t x, TileCoord_t z )
{
	auto& tileBuilder = tileBuilders[z * width + x];
	if(tileBuilder.layers.size() == 0)
	{
		return;
	}

	auto const tileGlobalX = (x * fragmentSubSamples);
	auto const tileGlobalZ = (z * fragmentSubSamples);
	float const globalFX = (float) tileGlobalX + bottomLeft.x;
	float const globalFZ = (float) tileGlobalZ + bottomLeft.y;

	for(auto layerIndex = 0u; layerIndex < tileBuilder.layers.size(); ++layerIndex)
	{
		auto const& texLayer = smoothLayerTexture->getLayer(layerIndex * 2);
		auto const& solidIndexLayer = smoothLayerTexture->getLayer((layerIndex * 2) + 1);
		auto& layer = tileBuilder.layers[layerIndex];

		Math::vec3 fPoints[fragmentSubSamples * fragmentSubSamples];
		Math::vec3 cPoints[fragmentSubSamples * fragmentSubSamples];
		size_t fNumPoints = 0;
		size_t cNumPoints = 0;
		float minHeight = FLT_MAX;
		float maxHeight = -FLT_MAX;

		// pick any solidIndex as dominance phase should ensure all the same
		uint32_t floorSolidIndex = ~0;
		uint32_t ceilSolidIndex = ~0;

		for(auto lz = 0u; lz < fragmentSubSamples; ++lz)
		{
			for(auto lx = 0u; lx < fragmentSubSamples; ++lx)
			{
				float const fx = globalFX + (float) lx / (float) fragmentSubSamples;
				float const fz = globalFZ + (float) lz / (float) fragmentSubSamples;

				float floorHeight = texLayer.getAt<float>( tileGlobalX + lx, tileGlobalZ + lz, 0 );
				float ceilHeight = texLayer.getAt<float>( tileGlobalX + lx, tileGlobalZ + lz, 1 );
				uint32_t floorSI = solidIndexLayer.getAt<uint32_t>( tileGlobalX + lx, tileGlobalZ + lz, 0 );
				uint32_t ceilSI = solidIndexLayer.getAt<uint32_t>( tileGlobalX + lx, tileGlobalZ + lz, 1 );

				if(!std::isnan( floorHeight ))
				{
					fPoints[fNumPoints++] = Math::vec3( fx, floorHeight, fz );
					minHeight = std::min( minHeight, floorHeight );
					maxHeight = std::max( maxHeight, floorHeight );
				}
				if(!std::isnan( ceilHeight ))
				{
					cPoints[cNumPoints++] = Math::vec3( fx, ceilHeight, fz );
					minHeight = std::min( minHeight, ceilHeight );
					maxHeight = std::max( maxHeight, ceilHeight );
				}
				floorSolidIndex = std::min(floorSolidIndex, floorSI);
				ceilSolidIndex = std::min(ceilSolidIndex, ceilSI);
			}
		}
		layer.floorDominantSolidIndex = floorSolidIndex;
		layer.ceilDominantSolidIndex = ceilSolidIndex;
		layer.minHeight = minHeight;
		layer.maxHeight = maxHeight;
		Math::vec3 bl(bottomLeft.x, 0, bottomLeft.y);
		Math::vec3 tr(bl + Math::vec3(extentIncrement.x, 0, extentIncrement.y));
		bl.y = layer.minHeight;
		tr.y = layer.maxHeight;
		// handle degenerate case
		if (bl.y > 1e6f)
		{
			// remove all layers
			tileBuilder.layers.resize(0);
			return;
		}

		// handle very thin case
		if (Math::ApproxEqual(layer.minHeight, layer.maxHeight))
		{
			tr.y = layer.minHeight + 0.1f;
			layer.floorPlane = Math::Plane(0, 1, 0, layer.minHeight);
			layer.ceilPlane = Math::Plane(0, 0, 0, std::numeric_limits<float>::quiet_NaN());
			layer.aabb = Geometry::AABB(bl, tr);
			// truncate to this layer index
			tileBuilder.layers.resize(layerIndex + 1);
			return;
		}

		layer.aabb = Geometry::AABB(bl, tr);

		if(fNumPoints >= 3)
		{
			for(size_t i = 0; i < fNumPoints; i++)
			{
				fPoints[i].y -= minHeight;
			}

			auto& plane = layer.floorPlane;
			plane = Math::CreatePlaneFromPoints( fNumPoints, fPoints );
			if(plane.b < -0.0f)
			{
				plane.b = -plane.b;
				plane.d = -plane.d;
			}
		} else
		{
			layer.floorPlane = Math::Plane( 0, 1, 0, layer.minHeight);
		}
		if(cNumPoints >= 3)
		{
			for(size_t i = 0; i < cNumPoints; i++)
			{
				cPoints[i].y -= minHeight;
			}

			auto& plane = layer.ceilPlane;
			plane = Math::CreatePlaneFromPoints( cNumPoints, cPoints );
			if(plane.b > 0.0f)
			{
				plane.b = -plane.b;
				plane.d = -plane.d;
			}
		} else
		{
			layer.ceilPlane = Math::Plane( 0, 0, 0, std::numeric_limits<float>::quiet_NaN());
		}
	}
}

void TacticalMapBuilder::getValidFragmentsForTile( TacticalMapTileBuilder& tileBuilder )
{
	auto hmIndex = 0u;

	// now we have statistics we can remove outliers
	for(auto& layer : tileBuilder.layers)
	{
		layer.validFloorFragments.resize( fragmentSubSamples * fragmentSubSamples );
		layer.validCeilFragments.resize( fragmentSubSamples * fragmentSubSamples );

		for(int sz = 0; sz < fragmentSubSamples; ++sz)
		{
			for(int sx = 0; sx < fragmentSubSamples; ++sx)
			{
				auto const& heightFrags = tileBuilder.heightMaps[sz * fragmentSubSamples + sx];
				TMapTBHeightFragment const *bfrag = (hmIndex < heightFrags.size()) ? &heightFrags[hmIndex]
																				   : nullptr;
				TMapTBHeightFragment const *tfrag = (hmIndex + 1 < heightFrags.size()) ? &heightFrags[hmIndex + 1]
																					   : nullptr;
				if(bfrag &&
				   fabs( bfrag->t - layer.floorMean ) < layer.floorStdDev)
				{
					layer.validFloorFragments[sz * fragmentSubSamples + sx] = bfrag;
				} else
				{
					layer.validFloorFragments[sz * fragmentSubSamples + sx] = nullptr;
				}

				if(tfrag &&
				   fabs( tfrag->t - layer.ceilMean ) < layer.ceilStdDev)
				{
					layer.validCeilFragments[sz * fragmentSubSamples + sx] = tfrag;
				} else
				{
					layer.validCeilFragments[sz * fragmentSubSamples + sx] = nullptr;
				}
			}
		}
		hmIndex += 2;
	}
}

void TacticalMapBuilder::generateHeightFragmentsForTileAt( TileCoord_t x, TileCoord_t z )
{
	auto& tileBuilder = tileBuilders[z * width + x];

	// generate a ray bundle
	typedef std::tuple<Geometry::WaterTightRay, int, int> RayTuple;
	RayTuple rayBundle[fragmentSubSamples * fragmentSubSamples];

	Math::vec2 const lb( bottomLeft.x + (x * extentIncrement.x),
							bottomLeft.y + (z * extentIncrement.y));
	Math::vec2 const inc( extentIncrement.x / fragmentSubSamples, extentIncrement.y / fragmentSubSamples );

	static Math::vec3 const rayDir( 0, 1, 0 );
	static Math::vec3 const upVector( 0, 1, 0 );
	static Math::vec3 const downVector( 0, -1, 0 );

	for(int sz = 0; sz < fragmentSubSamples; ++sz)
	{
		Math::vec3 origin( lb.x, rayMinHeight, lb.y + (sz * inc.y));
		for(int sx = 0; sx < fragmentSubSamples; ++sx)
		{
			rayBundle[sz * fragmentSubSamples + sx] = RayTuple( Geometry::WaterTightRay( origin, rayDir ), sx, sz );
			origin.x += inc.x;
		}
	}

	// boxes are easiest, lets start with them
	// TODO we know this an AABB so we don't need to use ray casting at all
	// but it fairly fast and
	// provides some sanity test for me for the next harder stage
	for(auto const& [solidIndex, box] : tileBuilder.boxes)
	{
		// todo simd and parallel this raybundle (tho see above before you do!)
		for(auto const[ray, sx, sz] : rayBundle)
		{
			float minT, maxT;
			bool hit = ray.intersectsAABB( box, minT, maxT );
			if(hit)
			{
				if(std::isfinite( minT ))
				{
					tileBuilder.heightMaps[sz * fragmentSubSamples + sx].push_back( { downVector, minT + rayMinHeight, solidIndex } );
				}
				if(std::isfinite( maxT ))
				{
					tileBuilder.heightMaps[sz * fragmentSubSamples + sx].push_back( { upVector, maxT + rayMinHeight, solidIndex } );
				}
			}
		}
	}
	// lets do the triangles (if this is too slow use KDTree/RayCaster)
	for(auto const[solidIndex, polygonIndexList] : tileBuilder.polygons)
	{
		auto [mesh, box, levelData] = solids[solidIndex];

		using namespace MeshMod;
		Vertices const& vertices = mesh->getVertices();
		Polygons const& polygons = mesh->getPolygons();
		auto const& planeEqs = polygons.getAttribute<PolygonData::PlaneEquations>();
		VertexIndexContainer polyVertIndices;
		polyVertIndices.reserve( 3 );

		for(auto const polygonIndex : polygonIndexList)
		{
			polyVertIndices.clear();
			polygons.getVertexIndices( polygonIndex, polyVertIndices );
			assert( polyVertIndices.size() == 3 );
			Math::vec3 v0, v1, v2;
			v0 = vertices.position( polyVertIndices[0] ).getVec3();
			v1 = vertices.position( polyVertIndices[1] ).getVec3();
			v2 = vertices.position( polyVertIndices[2] ).getVec3();

			// todo simd and parallel this raybundle
			for(auto const[ray, sx, sz] : rayBundle)
			{
				float v, w, t;
				bool hit = ray.intersectsTriangle( v0, v1, v2, v, w, t );
				if(hit)
				{
					t = t + rayMinHeight;
					tileBuilder.heightMaps[sz * fragmentSubSamples + sx].push_back(
							{planeEqs[polygonIndex].planeEq.normal(), t, solidIndex } );
				}
			}
		}
	}
	processHeightsForTile( tileBuilder );

}

void TacticalMapBuilder::processHeightsForTile( TacticalMapTileBuilder& tileBuilder )
{
	// count and sort fragments
	tileBuilder.maxLayers = 0;
	TacticalMapTileBuilder::TMapTBHeightFragmentList cloneList;

	for(int sz = 0; sz < fragmentSubSamples; ++sz)
	{
		for(int sx = 0; sx < fragmentSubSamples; ++sx)
		{
			auto& fragList = tileBuilder.heightMaps[sz * fragmentSubSamples + sx];

			std::sort( fragList.begin(), fragList.end(),
					   []( TMapTBHeightFragment const& lhs, TMapTBHeightFragment const& rhs )
					   {
						   return lhs.t < rhs.t;
					   } );

			// by counting crossing we know in/out
			bool outside = true;

			if(fragList.size() > 0)
			{
				cloneList = fragList;
				fragList.clear();
				float const closeEnough = 0.01f; // 1cm
				float lastT = -FLT_MAX;
				size_t lastSolid = ~0;

				for(auto i = 0; i < cloneList.size(); i++)
				{
					auto& frag = cloneList[i];
					bool reject = false;
					reject |= (outside && (frag.n.y < -0.9f));
					reject |= (!outside && (frag.n.y > 0.9f));

					// reject very close fragment from the same objects
//					reject |= (frag.solidIndex == lastSolid) && (abs( frag.t - lastT ) < closeEnough);

					if(!reject)
					{
						fragList.push_back( frag );
						lastT = frag.t;
						lastSolid = frag.solidIndex;
						outside ^= true;
					}
				}
				tileBuilder.maxLayers = std::max( tileBuilder.maxLayers, (int) fragList.size());
			}
		}
	}

	// integer round to nearest is wanted (not trunc to zero)
	tileBuilder.maxLayers = (tileBuilder.maxLayers & 0x1) ?
							(tileBuilder.maxLayers / 2) + 1 :
							(tileBuilder.maxLayers / 2);
	tileBuilder.layers.resize( tileBuilder.maxLayers );

	calculateHeightMapMeanAndStandardDeviation( tileBuilder );

}

void TacticalMapBuilder::calculateHeightMapMeanAndStandardDeviation( TacticalMapTileBuilder& tileBuilder )
{
	// calculate statistics of samples
	auto hmIndex = 0u;

	// mean and valid counts first
	// also a histrogram of the solidIndex to determine the dominant solid
	for(auto& layer : tileBuilder.layers)
	{
		int floorValidFragmentCount = 0;
		int ceilValidFragmentCount = 0;

		std::vector<size_t> floorSolidIndexHistogram(solids.size());
		std::vector<size_t> ceilSolidIndexHistogram(solids.size());

		for(auto const& heightFrags : tileBuilder.heightMaps)
		{
			TMapTBHeightFragment const *bfrag = (hmIndex < heightFrags.size()) ? &heightFrags[hmIndex] : nullptr;
			TMapTBHeightFragment const *tfrag = (hmIndex + 1 < heightFrags.size()) ? &heightFrags[hmIndex + 1]
																				   : nullptr;

			if(bfrag && bfrag->n.y > maxFloorInclination)
			{
				layer.floorMean += bfrag->t;
				floorSolidIndexHistogram[bfrag->solidIndex]++;
				floorValidFragmentCount++;
			}
			if(tfrag && tfrag->n.y < -maxFloorInclination)
			{
				layer.ceilMean += tfrag->t;
				floorSolidIndexHistogram[tfrag->solidIndex]++;
				ceilValidFragmentCount++;
			}
		}

		// determine dominant solid indices
		size_t floorDominantMaxSolidIndex = 0;
		size_t ceilDominantMaxSolidIndex = 0;
		for(auto i = 0u; i < solids.size(); ++i)
		{
			if(floorSolidIndexHistogram[i] > floorDominantMaxSolidIndex)
			{
				floorDominantMaxSolidIndex = floorSolidIndexHistogram[i];
				layer.floorDominantSolidIndex = i;
			}
			if(ceilSolidIndexHistogram[i] > ceilDominantMaxSolidIndex)
			{
				ceilDominantMaxSolidIndex = ceilSolidIndexHistogram[i];
				layer.ceilDominantSolidIndex = i;
			}
		}

		if(floorValidFragmentCount > 0)
		{
			layer.floorMean /= (float) floorValidFragmentCount;
		}
		if(ceilValidFragmentCount > 0)
		{
			layer.ceilMean /= (float) ceilValidFragmentCount;
		}

		// now std deviation
		for(auto const& heightFrags : tileBuilder.heightMaps)
		{
			TMapTBHeightFragment const *bfrag = (hmIndex < heightFrags.size()) ? &heightFrags[hmIndex] : nullptr;
			TMapTBHeightFragment const *tfrag = (hmIndex + 1 < heightFrags.size()) ? &heightFrags[hmIndex + 1]
																				   : nullptr;
			if(bfrag)
			{
				layer.floorStdDev += Math::square( bfrag->t - layer.floorMean );
			}
			if(tfrag)
			{
				layer.ceilStdDev += Math::square( tfrag->t - layer.ceilMean );
			}
		}

		if(floorValidFragmentCount > 0)
		{
			layer.floorStdDev = sqrt( layer.floorStdDev / (float) floorValidFragmentCount );
			layer.floorStdDev += 1e-5f; // add a small amount of tolerance
		}
		if(ceilValidFragmentCount > 0)
		{
			layer.ceilStdDev = sqrt( layer.ceilStdDev / (float) ceilValidFragmentCount );
			layer.ceilStdDev += 1e-5f; // add a small amount of tolerance
		}
		hmIndex += 2;
	}

	getValidFragmentsForTile( tileBuilder );

}

void TacticalMapBuilder::generateStructuralBoxesForTileAt( TileCoord_t x, TileCoord_t z )
{
	auto& tileBuilder = tileBuilders[z * width + x];

	Math::vec3 bl( bottomLeft.x, 0, bottomLeft.y );
	Math::vec3 tr( bl + Math::vec3( extentIncrement.x, 0, extentIncrement.y ));

	// pass 1 determine structural type and calculate bonds
	tileBuilder.structuralBoxs.clear();
	tileBuilder.structuralBoxs.resize(solids.size());
	for (size_t i = 0; i < tileBuilder.structuralBoxs.size(); i++)
	{
		WorldSolid const& solid = solids[i];
		auto& sbox = tileBuilder.structuralBoxs[i];
		sbox.structuralIntegrity = 0;
		sbox.structuralType = DetermineStructuralType(solid);

		Geometry::AABB bbox = solid.aabb;
		bbox.expandBy(Math::vec3(1.01f, 1.01f, 1.01f));
		for (size_t j = i + 1; j < tileBuilder.structuralBoxs.size(); j++)
		{
			TMapTBStructuralBox& osbox = tileBuilder.structuralBoxs[j];
			WorldSolid const& other = solids[j];
			Geometry::AABB obox = other.aabb;
			if (bbox.intersects(obox))
			{
				Math::vec3 dir = obox.getBoxCenter() - bbox.getBoxCenter();
				Cardinal cdir = ToCardinal(dir);
				sbox.bonds.emplace_back(i, j, cdir);
				osbox.bonds.emplace_back(j, i, Flip(cdir));
			}
		}
	}

	
	// pass 2 calculate structural integrity
	for (size_t i = 0; i < tileBuilder.structuralBoxs.size(); i++)
	{
		TMapTBStructuralBox& sbox = tileBuilder.structuralBoxs[i];

		bool hasDownwardBond = false;
		for (auto const& bond : sbox.bonds)
		{
			TMapTBStructuralBox& osbox = tileBuilder.structuralBoxs[bond.otherIndex];
			switch (bond.direction)
			{
			case Cardinal::Above: sbox.structuralIntegrity++; break;
			case Cardinal::Below: hasDownwardBond = true; break;
			case Cardinal::Left: sbox.structuralIntegrity++; break;
			case Cardinal::Right: sbox.structuralIntegrity++; break;
			case Cardinal::Front: break;
			case Cardinal::Back: break;
			}
		}

		if (hasDownwardBond == false)
		{
			// nothing holding it up?! make it a world structure
			sbox.structuralType = StructuralType::World;
		}
	}

	// propogate structural boxes into the layers
	for (auto const&[solidIndex, box] : tileBuilder.destructables)
	{
		for (auto& layer : tileBuilder.layers)
		{
			if (layer.aabb.intersects(box))
			{
				layer.destructionSolidIndices.push_back(solidIndex);
			}
		}
	}
}

// determine major axis to catergorise floor/wall
auto TacticalMapBuilder::DetermineStructuralType(WorldSolid const& solid_) -> StructuralType
{
	auto const&[meshPtr, aabb, levelData] = solid_;

	// use bounding bounds face area to determine wall or floor
	Math::vec3 const halfLen = aabb.getHalfLength();
	auto xyArea = halfLen.x * halfLen.y;
	auto xzArea = halfLen.x * halfLen.z;
	auto zyArea = halfLen.z * halfLen.y;

	if (xzArea > xyArea && xzArea > zyArea)
	{
		return StructuralType::Floor;
	}
	else
	{
		return StructuralType::Wall;
	}
}
