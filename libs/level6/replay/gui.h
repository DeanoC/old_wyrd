#pragma once
#ifndef WYRD_REPLAY_GUI_H
#define WYRD_REPLAY_GUI_H

#include "core/core.h"
#include "replay/replay.h"
#include "math/vector_math.h"
#include "nlohmann/json.h"
#include <string>

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
	using SpatialMarkers = std::vector<SpatialMarker>;
	using MeshMap = std::unordered_map<std::string, MidRender::SceneIndex>;

	auto menu() -> void;
	auto processReplaySection() -> void;

	auto pause() -> void;

	auto play() -> void;

	std::shared_ptr<Replay> replay;
	bool windowOpen = true;

	std::shared_ptr<ResourceManager::ResourceMan> rm;
	std::unique_ptr<MidRender::MeshModRenderer> meshModRenderer;

	MidRender::SceneIndex diamondSceneIndex;
	float yrot = 0.0f;
	SpatialMarkers spatialMarkers;

	MeshMap meshMap;

	double viewerTime;
	ItemType logFilter;
	enum class MainViewType {
		Scene,
		Mesh,
	} mainView = MainViewType::Scene;


	static auto getVec(std::string const& field_, nlohmann::json const& j_) -> Math::vec3;
	auto DecodeLog(Item const& item) -> void;
	auto DecodeSimpleMesh(Item const&) -> void;
	auto DecodeMeshObject(Item const&) -> void;

	auto sceneView(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void;
	auto meshView(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void;
};

}

#endif //WYRD_REPLAY_GUI_H
