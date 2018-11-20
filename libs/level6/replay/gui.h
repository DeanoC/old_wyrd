#pragma once
#ifndef WYRD_REPLAY_GUI_H
#define WYRD_REPLAY_GUI_H

#include "core/core.h"
#include "replay/replay.h"
#include "math/vector_math.h"

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

	auto log() -> void;

	auto pause() -> void;

	auto play() -> void;

	std::shared_ptr<Replay> replay;
	bool windowOpen = true;

	std::shared_ptr<ResourceManager::ResourceMan> rm;
	std::unique_ptr<MidRender::MeshModRenderer> meshModRenderer;
	MidRender::SceneIndex diamondSceneIndex;
	float yrot = 0.0f;
	SpatialMarkers spatialMarkers;

	double viewerTime;
	ItemType logFilter;
};

}

#endif //WYRD_REPLAY_GUI_H
