#pragma once
#ifndef WYRD_REPLAY_GUI_H
#define WYRD_REPLAY_GUI_H

#include "core/core.h"
#include "replay/replay.h"
#include "math/vector_math.h"
#include <string>

namespace picojson { class value; }
namespace ResourceManager { class ResourceMan; }
namespace Render { struct Encoder; }
namespace MidRender {
class MeshModRenderer;
enum class MeshIndex : uint32_t;
enum class SceneIndex : uint32_t;
}

namespace Replay {

// assumes imgui is already setup
class Gui
{
public:
	Gui(std::shared_ptr<ResourceManager::ResourceMan> const& rm_,
		std::shared_ptr<Replay> const& replay_);
	~Gui();

	// replay gui expects to be inside a simple forward render pass when render
	// is called

	auto render(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void;

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
	bool windowOpen = true;

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
	} mainView = MainViewType::Scene;


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

	auto meshCallback(Item const& item_) -> bool;

};

}

#endif //WYRD_REPLAY_GUI_H
