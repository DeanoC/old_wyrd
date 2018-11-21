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
#include "meshops/platonicsolids.h"
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

	{
		auto rootScene = std::make_shared<MeshMod::SceneNode>();
		rootScene->addObject(MeshOps::Shapes::createDiamond());
		diamondSceneIndex = meshModRenderer->addScene(rootScene);
	}
	{
		auto rootScene = std::make_shared<MeshMod::SceneNode>();
		rootScene->addObject(MeshOps::PlatonicSolids::createIcosahedron());
		fallbackSceneIndex = meshModRenderer->addScene(rootScene);
	}
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

	log();

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
	for(auto const&[name, meshObject] : meshObjectMap)
	{
		auto tranMat = translate(glm::identity<glm::mat4x4>(), meshObject.position);
		auto rotXMat = rotate(tranMat, meshObject.rotation.x, glm::vec3(1, 0, 0));
		auto rotYMat = rotate(rotXMat, meshObject.rotation.y, glm::vec3(0, 1, 0));
		auto rotZMat = rotate(rotYMat, meshObject.rotation.z, glm::vec3(0, 0, 1));
		auto rootMat = scale(rotZMat, meshObject.scale);

		meshModRenderer->render(rootMat, meshObject.index, encoder_);
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
	double const time = (viewerTime < 0.0) ? replay->getCurrentTime() : viewerTime;

	// TODO add callback system to replay so mesh get processed when the packet is
	// received.
	auto smItems = replay->getRange(time - 1.0, time, Items::SimpleMeshType);
	for(auto const& item : smItems)
	{
		decodeSimpleMesh(item);
	}
	auto moItems = replay->getRange(time - 1.0, time, Items::MeshObjectType);
	for(auto const& item : moItems)
	{
		decodeMeshObject(item);
	}
}

auto Gui::log() -> void
{
	using namespace nlohmann;
	using namespace std::literals;

	ImGuiWindowFlags logWindowFlags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize;
	ImGui::SetNextWindowPos(ImVec2(20, 600), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(1200, 100), ImGuiCond_FirstUseEver);

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
		if(item.type == Items::LogType)
		{
			decodeLog(item);
		}
	}
	ImGui::EndChild();
	ImGui::End();

}

auto Gui::GetVec(std::string const& field_, nlohmann::json const& j_) -> Math::vec3
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

auto Gui::decodeLog(Item const& item_) -> void
{
	using namespace nlohmann;
	using namespace std::literals;

	try
	{
		nlohmann::json const j = json::parse(item_.data);
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

		logString = fmt::format("({}s):[{}]{}",
								item_.timeStamp,
								level,
								text);
		if(j.find("position") != j.cend() && ImGui::IsItemHovered())
		{
			glm::vec3 pos = GetVec("position"s, j);
			logString += fmt::format(" {},{},{}", pos.x, pos.y, pos.z);
			spatialMarkers.push_back({pos});
		}
		ImGui::Text(logString.c_str());

	} catch(json::parse_error error)
	{
		auto logString = fmt::format("({}s):!ERROR!Log Parse error {} from {}",
									 item_.timeStamp,
									 error.what(),
									 item_.data);
		LOG_S(WARNING) << logString;
		ImGui::Text(logString.c_str());
	}
}

auto Gui::decodeSimpleMesh(Item const& item_) -> void
{
	using namespace nlohmann;
	using namespace std::literals;

	try
	{
		nlohmann::json const j = json::parse(item_.data);
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
		auto logString = fmt::format("({}s):!ERROR!Log Parse error {} from {}",
									 item_.timeStamp,
									 error.what(),
									 item_.data);
		LOG_S(WARNING) << logString;
	}
}

auto Gui::decodeMeshObject(Item const& item_) -> void
{
	using namespace nlohmann;
	using namespace std::literals;

	try
	{
		nlohmann::json const j = json::parse(item_.data);
		auto name = j["name"].get<std::string>();

		MeshObject& o = meshObjectMap[name];
		if(o.index == MidRender::SceneIndex(~0))
		{
			o.index = fallbackSceneIndex;
		}

		if(j.find("meshname") != j.end())
		{
			auto meshname = j["meshname"].get<std::string>();
			auto meshIt = meshMap.find(meshname);
			if(meshIt != meshMap.end())
			{
				o.index = meshIt->second;
			}
		}
		if(j.find("position") != j.end())
		{
			o.position = GetVec("position", j);
		}
		if(j.find("rotation") != j.end())
		{
			o.scale = GetVec("rotation", j);
		}
		if(j.find("scale") != j.end())
		{
			o.scale = GetVec("scale", j);
		}

	} catch(json::parse_error error)
	{
		auto logString = fmt::format("({}s):!ERROR!Log Parse error {} from {}",
									 item_.timeStamp,
									 error.what(),
									 item_.data);
		LOG_S(WARNING) << logString;
	}

}

}