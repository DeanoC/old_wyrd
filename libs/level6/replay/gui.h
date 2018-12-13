#pragma once
#ifndef WYRD_REPLAY_GUI_H
#define WYRD_REPLAY_GUI_H

#include "core/core.h"
#include "replay/replay.h"
#include "math/vector_math.h"
#include "geometry/aabb.h"
#include <string>

namespace picojson { class value; }
namespace ResourceManager { class ResourceMan; }
namespace Render { struct Encoder; }
namespace MidRender {
class MeshModRenderer;
enum class MeshIndex : uint32_t;
enum class SceneIndex : uint32_t;
}
namespace MeshMod{
class SceneNode;
class Mesh;
}

class TacticalMap;
struct TacticalMapTile;

namespace Replay {

// assumes imgui is already setup
class Gui
{
public:
	Gui(std::shared_ptr<ResourceManager::ResourceMan> const& rm_,
		std::shared_ptr<Replay> const& replay_);
	~Gui();

	enum class CameraMode
	{
		None,
		ArcBall,
		FPS,
	};

	auto getCameraMode() const->CameraMode { return cameraMode; }
	auto getArcBallFocusPoint() const -> Math::vec3 { return arcBallFocusPoint; }

	// replay gui expects to be inside a simple forward render pass when render
	// is called

	auto render(bool showUI_, double deltaT_, 
				std::shared_ptr<Render::Encoder> const& encoder_) -> void;

protected:
	struct SpatialMarker
	{
		Math::vec3 position;
	};
	struct MeshObject
	{
		MidRender::SceneIndex index = MidRender::SceneIndex(~0);
		Math::vec3 position{0, 0, 0};
		Math::vec3 rotation{0, 0, 0};
		Math::vec3 scale{1, 1, 1};
	};

	using SpatialMarkers = std::vector<SpatialMarker>;
	using MeshMap = std::unordered_map<std::string, MidRender::SceneIndex>;
	using MeshObjectMap = std::unordered_map<std::string, MeshObject>;

	std::shared_ptr<Replay> replay;
	mutable std::mutex lockMutex;
	bool windowOpen = true;
	CameraMode cameraMode = CameraMode::ArcBall;
	Math::vec3 arcBallFocusPoint = { 0,0,0 };

	std::shared_ptr<ResourceManager::ResourceMan> rm;
	std::unique_ptr<MidRender::MeshModRenderer> meshModRenderer;

	MidRender::SceneIndex diamondSceneIndex;
	MidRender::SceneIndex fallbackSceneIndex;

	float yrot = 0.0f;
	SpatialMarkers spatialMarkers;

	MeshMap meshMap;
	MeshObjectMap meshObjectMap;

	double viewerTime;
	ItemType logFilter;
	enum class MainViewType {
		Scene,
		Mesh,
		TacticalMaps,
	} mainView = MainViewType::Scene;

	int meshViewSelectedItem = 0;
	int tacMapViewSelectedItem = 0;

	struct TacMapTile
	{
		Geometry::AABB aabb;
		size_t objectIndex;
	};
	std::unordered_map<std::string, std::pair<MidRender::SceneIndex, std::shared_ptr<std::vector<TacMapTile>>>> tacticalMaps;

	auto menu() -> void;
	auto processReplaySection() -> void;
	auto log() -> void;
	auto pause() -> void;
	auto play() -> void;

	auto decodeLog(Item const& item_) -> void;
	auto decodeSimpleMesh(Item const& item_) -> void;
	auto decodeMeshObject(Item const& item_) -> void;

	auto sceneView(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void;
	auto meshView(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void;
	auto tacticalMapView(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void;

	auto meshCallback(Item const& item_) -> bool;
	auto tacmapCallback(Item const& item_) -> bool;

	auto tmapTileGenerateMesh(	int x_,
								int z_,
								std::shared_ptr<MeshMod::SceneNode>& rootScene_,
								std::shared_ptr<MeshMod::Mesh>& combinedMesh,
								std::shared_ptr<std::vector<TacMapTile>>& renderTiles_,
							  	std::shared_ptr<TacticalMap> const& map_) -> void;

};

}

#endif //WYRD_REPLAY_GUI_H
