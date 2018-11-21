#include <cctype>
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
#include "meshmod/polygons.h"
#include "meshmod/scene.h"
#include "meshmod/vertices.h"
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
			ImGuiWindowFlags_MenuBar |
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

		menu();

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

	processReplaySection();

	switch(mainView)
	{
		default:
		case MainViewType::Scene:
			sceneView(deltaT_, encoder_);
			break;
		case MainViewType::Mesh:
			meshView(deltaT_, encoder_);
			break;
	}
}

auto Gui::sceneView(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	for(auto const& marker : spatialMarkers)
	{
		auto translationMat = translate(glm::identity<glm::mat4x4>(), marker.position);
		auto rootMat = rotate(translationMat, yrot, glm::vec3(0, 1, 0));
		yrot += Math::degreesToRadians(90.0f) * float(deltaT_);
		while(yrot > Math::two_pi<float>())
		{
			yrot -= Math::two_pi<float>();
		}

		meshModRenderer->render(rootMat, diamondSceneIndex, encoder_);
	}
}

auto Gui::meshView(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
}

auto Gui::pause() -> void
{
	viewerTime = replay->getCurrentTime();
}

auto Gui::play() -> void
{
	viewerTime = -1;
}

auto Gui::menu() -> void
{
	// Menu
	if(ImGui::BeginMenuBar())
	{
		if(ImGui::BeginMenu("Menu"))
		{
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("View"))
		{
			if(ImGui::MenuItem("Scene", nullptr))
			{
				mainView = MainViewType::Scene;
			}
			if(ImGui::MenuItem("Mesh", nullptr))
			{
				mainView = MainViewType::Mesh;
			}
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Help"))
		{
			ImGui::MenuItem("Help");
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

auto Gui::processReplaySection() -> void
{
	using namespace nlohmann;
	using namespace std::literals;

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
	ImGui::RadioButton("LOG", &tmpLF, (int) Items::LogType);
	logFilter = ItemType(tmpLF);

	double const time = (viewerTime < 0.0) ? replay->getCurrentTime() : viewerTime;
	auto items = replay->getRange(time - 1.0, time, logFilter);

	ImGui::BeginChild("");
	for(auto const& item : items)
	{
		// for all filter, log everything
		if(logFilter == ItemType(0) && (item.type != Items::LogType))
		{
			char typeString[5];
			std::memcpy(typeString, &item.type, sizeof(uint32_t));
			typeString[4] = 0;

			auto logString = fmt::format("({}s)[{}]: {}", item.timeStamp, typeString, item.data);
			ImGui::Text(logString.c_str());
		}

		switch(item.type)
		{
			default:
			case Items::TestType: break;

			case Items::LogType:
			{
				DecodeLog(item);
				break;
			}
			case Items::SimpleMeshType:
			{
				DecodeSimpleMesh(item);
				break;
			}
			case Items::MeshObjectType:
			{
				DecodeMeshObject(item);
				break;
			}
		}

	}
	ImGui::EndChild();
	ImGui::End();

}

auto Gui::getVec(std::string const& field_, nlohmann::json const& j_) -> Math::vec3
{
	using namespace nlohmann;

	glm::vec3 pos{};
	auto jfield = j_[field_];
	if(jfield.is_string())
	{
		jfield = json::parse(jfield.get<std::string>());
	}

	if(jfield.is_object())
	{
		pos.x = jfield["x"];
		pos.y = jfield["y"];
		pos.z = jfield["z"];
	} else if(jfield.is_array())
	{
		pos.x = jfield[0];
		pos.y = jfield[1];
		pos.z = jfield[2];
	}

	return pos;
}

auto Gui::DecodeLog(Item const& item) -> void
{
	using namespace nlohmann;
	using namespace std::literals;

	try
	{
		nlohmann::json const j = json::parse(item.data);
		auto text = j["text"];
		std::string logString;
		std::string level = "info";
		if(j.find("level") != j.cend())
		{
			auto jlevel = j["level"];
			if(jlevel.is_string())
			{
				std::string stringLevel = jlevel.get<std::string>();
				transform(
						stringLevel.cbegin(),
						stringLevel.cend(),
						stringLevel.begin(), &tolower);

				switch(Core::QuickHash(stringLevel))
				{
					default:
					case "info"_hash:
						level = "info";
						break;
					case "warning"_hash:
						level = "warning";
						break;
					case "error"_hash:
						level = "error";
						break;
				}
			}
		}

		bool hasPosition = false;
		glm::vec3 position{};
		if(j.find("position") != j.cend())
		{
			hasPosition = true;
		}
		logString = fmt::v5::format("({}s):[{}]{}",
									item.timeStamp,
									level,
									text);
		if(hasPosition && ImGui::IsItemHovered())
		{
			glm::vec3 pos = getVec("position"s, j);
			logString += fmt::v5::format(" {},{},{}", pos.x, pos.y, pos.z);
			spatialMarkers.push_back({pos});
		}
		ImGui::Text(logString.c_str());

	} catch(json::parse_error error)
	{
		auto logString = fmt::v5::format("({}s):!ERROR!Log Parse error {} from {}",
										 item.timeStamp,
										 error.what(),
										 item.data);
		LOG_S(WARNING) << logString;
		ImGui::Text(logString.c_str());
	}
}

auto Gui::DecodeSimpleMesh(Item const& item) -> void
{
	using namespace nlohmann;
	using namespace std::literals;

	try
	{
		nlohmann::json const j = json::parse(item.data);
		auto jname = j["name"];
		auto name = jname.get<std::string>();
		// check cache
		if(meshMap.find(name) != meshMap.end())
		{
			return;
		}

		auto jposCount = j["positioncount"];
		auto jpositions = j["positions"];
		auto jtriCount = j["trianglecount"];
		auto jindices = j["indices"];

		auto posCount = jposCount.get<uint32_t>();
		auto triCount = jtriCount.get<uint32_t>();

		std::vector<float> positions;
		std::vector<uint32_t> indices;
		positions = jpositions.get<std::vector<float>>();
		indices = jindices.get<std::vector<uint32_t>>();

		using namespace MeshMod;
		auto mesh = std::make_shared<Mesh>(name, false, true);
		Vertices& vertices = mesh->getVertices();
		for(auto i = 0u; i < posCount; ++i)
		{
			vertices.add(positions[(i * 3) + 0],
						 positions[(i * 3) + 1],
						 positions[(i * 3) + 2]);
		}

		Polygons& polygons = mesh->getPolygons();
		for(auto i = 0u; i < triCount; ++i)
		{
			VertexIndexContainer triIndices = {
					indices[(i * 3) + 0],
					indices[(i * 3) + 2],
					indices[(i * 3) + 1]
			};
			mesh->getPolygons().add(triIndices);
		}
		mesh->updateFromEdits();

		auto rootScene = std::make_shared<SceneNode>();
		rootScene->addObject(mesh);
		auto sceneIndex = meshModRenderer->addScene(rootScene);
		meshMap[name] = sceneIndex;

	} catch(json::parse_error error)
	{
		auto logString = fmt::v5::format("({}s):!ERROR!Log Parse error {} from {}",
										 item.timeStamp,
										 error.what(),
										 item.data);
		LOG_S(WARNING) << logString;
		ImGui::Text(logString.c_str());
	}
}
auto Gui::DecodeMeshObject(Item const& item) -> void
{
	using namespace nlohmann;
	using namespace std::literals;

	try
	{
		nlohmann::json const j = json::parse(item.data);

	} catch(json::parse_error error)
	{
		auto logString = fmt::v5::format("({}s):!ERROR!Log Parse error {} from {}",
										 item.timeStamp,
										 error.what(),
										 item.data);
		LOG_S(WARNING) << logString;
		ImGui::Text(logString.c_str());
	}

}

}