#include "core/core.h"
#include "replay/gui.h"
#include "replay/replay.h"
#include "midrender/imguibindings.h"
#include "fmt/format.h"
#include "replay/items.h"
#include "replay/propertyeditor.h"
#include "render/encoder.h"
#include "midrender/meshmodrenderer.h"
#include "resourcemanager/resourceman.h"
#include "meshmod/mesh.h"
#include "meshmod/scene.h"
#include "meshops/shapes.h"

namespace Replay {
Gui::Gui(std::shared_ptr<ResourceManager::ResourceMan> const& rm_,
		 std::shared_ptr<Replay> const& replay_) :
		rm(rm_),
		replay(replay_),
		viewerTime(-1.0),
		logFilter(ItemType(0))
{
	meshModRenderer.reset(new MidRender::MeshModRenderer());
	meshModRenderer->init(rm);

	auto rootScene = std::make_shared<MeshMod::SceneNode>();
	rootScene->addObject(MeshOps::Shapes::createDiamond());
	diamondSceneIndex = meshModRenderer->addScene(rootScene);

}

Gui::~Gui()
{
	meshModRenderer->destroy();
	meshModRenderer.reset();

}

auto Gui::render(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	// reset every frame
	spatialMarkers.clear();

	ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(1240, 100), ImGuiCond_FirstUseEver);

	if(!ImGui::Begin("Replay", &windowOpen, window_flags))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
	} else
	{
		double time = (viewerTime < 0.0) ? replay->getCurrentTime() : viewerTime;
		auto timeString = fmt::format("Time: {}s", time);
		ImGui::Text(timeString.c_str());

		if(ImGui::Button(ICON_FA_PAUSE))
		{
			pause();
		}
		ImGui::SameLine();
		if(ImGui::Button(ICON_FA_PLAY))
		{
			play();
		}
		static double const timeMin = 0.0;
		double const timeMax = replay->getCurrentTime();

		if(ImGui::SliderScalar("", ImGuiDataType_Double, &time, &timeMin, &timeMax))
		{
			pause();
			viewerTime = time;
		}
		ImGui::End();
	}

	log();


	for(auto const& marker : spatialMarkers)
	{
		auto translationMat = Math::translate(Math::identity<Math::mat4x4>(), marker.position);
		auto rootMat = Math::rotate(translationMat, yrot, Math::vec3(0, 1, 0));
		yrot += Math::degreesToRadians(90.0f) * float(deltaT_);
		while(yrot > Math::two_pi<float>())
		{
			yrot -= Math::two_pi<float>();
		}

		meshModRenderer->render(rootMat, diamondSceneIndex, encoder_);
	}
}

auto Gui::pause() -> void
{
	viewerTime = replay->getCurrentTime();
}

auto Gui::play() -> void
{
	viewerTime = -1;
}

auto Gui::log() -> void
{
	using namespace nlohmann;
	ImGuiWindowFlags logWindowFlags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize;
	ImGui::SetNextWindowPos(ImVec2(20, 600), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(1260, 160), ImGuiCond_FirstUseEver);
	ImGui::Begin("Log", nullptr, logWindowFlags);

	int tmpLF = int(logFilter);
	ImGui::RadioButton("All", &tmpLF, 0);
	ImGui::SameLine();
	ImGui::RadioButton("TEST", &tmpLF, (int) Items::TestType);
	ImGui::SameLine();
	ImGui::RadioButton("LOG", &tmpLF, (int) Items::LogType);
	logFilter = ItemType(tmpLF);

	double const time = (viewerTime < 0.0) ? replay->getCurrentTime() : viewerTime;
	auto items = replay->getRange(time - 1.0, time, logFilter);

	ImGui::BeginChild("");
	for(auto const& item : items)
	{

		switch(item.type)
		{
			default:
			case Items::TestType:
			{
				char typeString[5];
				std::memcpy(typeString, &item.type, sizeof(uint32_t));
				typeString[4] = 0;

				auto logString = fmt::format("({}s)[{}]: {}", item.timeStamp, typeString, item.data);
				ImGui::Text(logString.c_str());
				break;
			}
			case Items::LogType:
			{
				try
				{
					json const j = json::parse(item.data);
					auto text = j["text"];
					std::string logString;
					std::string level = "info";
					if(j.find("level") != j.cend())
					{

					}

					bool hasPosition = false;
					Math::vec3 position{};
					if(j.find("position") != j.cend())
					{
						hasPosition = true;
					}
					logString = fmt::format("({}s):[{}]{}",
											item.timeStamp,
											level,
											text);
					if(hasPosition && ImGui::IsItemHovered())
					{
						Math::vec3 pos{};
						auto jpos = j["position"];
						if (jpos.is_string())
						{
							jpos = json::parse(jpos.get<std::string>());
						}

						if(jpos.is_object())
						{
							pos.x = jpos["x"];
							pos.y = jpos["y"];
							pos.z = jpos["z"];
						} else if(jpos.is_array())
						{
							pos.x = jpos[0];
							pos.y = jpos[1];
							pos.z = jpos[2];
						} 
						logString += fmt::format(" {},{},{}", pos.x, pos.y, pos.z);
						spatialMarkers.push_back({pos});
					}
					ImGui::Text(logString.c_str());

				} catch(json::parse_error error)
				{
					auto logString = fmt::format("({}s):!ERROR!Log Parse error {} from {}",
												 item.timeStamp,
												 error.what(),
												 item.data);
					LOG_S(WARNING) << logString;
					ImGui::Text(logString.c_str());
				}
				break;
			}
		}

	}
	ImGui::EndChild();
	ImGui::End();

}

}