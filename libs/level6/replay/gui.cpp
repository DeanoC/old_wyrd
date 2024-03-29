#include "core/core.h"
#include "replay/gui.h"
#include "replay/replay.h"
#include "midrender/imguibindings.h"
#include "fmt/format.h"
#include "replay/items.h"
#include "render/encoder.h"
#include "midrender/meshmodrenderer.h"
#include "resourcemanager/resourceman.h"
#include "meshmod/mesh.h"
#include "meshmod/polygons.h"
#include "meshmod/scene.h"
#include "meshmod/vertices.h"
#include "meshops/platonicsolids.h"
#include "meshops/shapes.h"
#include "meshops/basicmeshops.h"
#include "fmt/format.h"
#include "tacticalmap/tacticalmap.h"
#include <cctype>
#include "picojson/picojson.h"

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
		rootScene->addObject(MeshOps::Shapes::CreateDiamond());
		diamondSceneIndex = meshModRenderer->addScene(rootScene);
	}
	{
		auto rootScene = std::make_shared<MeshMod::SceneNode>();
		rootScene->addObject(MeshOps::PlatonicSolids::CreateIcosahedron());
		fallbackSceneIndex = meshModRenderer->addScene(rootScene);
	}

	using namespace std::placeholders;
	replay->registerCallback(Items::SimpleMeshType, std::bind(&Gui::meshCallback, this, _1));
	replay->registerCallback(Items::TacticalMapType, std::bind(&Gui::tacmapCallback, this, _1));

}

Gui::~Gui()
{
	meshModRenderer->destroy();
	meshModRenderer.reset();
}

auto Gui::render(bool showUI_, double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void
{

	// reset every frame
	spatialMarkers.clear();

	if(showUI_)
	{
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
			ImGui::Text("%s", timeString.c_str());

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
	}

	processReplaySection();

	std::lock_guard lock(lockMutex);
	switch(mainView)
	{
		default:
		case MainViewType::Scene:
			sceneView(deltaT_, encoder_);
			break;
		case MainViewType::Mesh:
			meshView(deltaT_, encoder_);
			break;
		case MainViewType::TacticalMaps:
			tacticalMapView(deltaT_, encoder_);
			break;
	}
}

auto Gui::sceneView(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	cameraMode = CameraMode::FPS;
	yrot += Math::degreesToRadians(30.0f) * float(deltaT_);
	for(auto const& marker : spatialMarkers)
	{
		auto translationMat = translate(glm::identity<glm::mat4x4>(), marker.position);
		auto rootMat = rotate(translationMat, yrot, glm::vec3(0, 1, 0));

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

	std::string comboString = {};
	std::vector<std::string> nameLookup;
	nameLookup.reserve(tacticalMaps.size());
	for (auto const&[name, index] : tacticalMaps)
	{
		comboString += name + '\0';
		nameLookup.push_back(name);
	}

	ImGui::Combo("Select Mesh", &tacMapViewSelectedItem, comboString.c_str());
	if (tacMapViewSelectedItem >= (int)nameLookup.size()) return;

	auto const sceneIndex = tacticalMaps[nameLookup[tacMapViewSelectedItem]].first;
	auto const& tiles = tacticalMaps[nameLookup[tacMapViewSelectedItem]].second;

	Geometry::AABB total;
	for (auto const& tile : *tiles)
	{
		total.expandBy(tile.aabb);
	}
	arcBallFocusPoint = total.getBoxCenter();

	meshModRenderer->render(
		Math::identity<Math::mat4x4>(),
		sceneIndex,
		encoder_);
}

auto Gui::meshView(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	cameraMode = CameraMode::ArcBall;
	arcBallFocusPoint = Math::vec3(0, 0, 0);

	std::string comboString = {};
	std::vector<std::string> nameLookup;
	nameLookup.reserve(meshMap.size());
	for(auto const&[name, index] : meshMap)
	{
		comboString += name + '\0';
		nameLookup.push_back(name);
	}

	ImGui::Combo("Select Mesh", &meshViewSelectedItem, comboString.c_str());
	if(meshViewSelectedItem >= (int) nameLookup.size()) return;

	auto const sceneIndex = meshMap[nameLookup[meshViewSelectedItem]];

	yrot += float(deltaT_ * 0.5);
	/*		rootScene->transform.orientation = Math::rotate(
				Math::identity<Math::quat>(),
				yrot,
				Math::vec3(0, 1, 0));
		rootScene->transform.orientation = Math::rotate(
				rootScene->transform.orientation,
				yrot / 2.0f,
				Math::vec3(0, 0, 1));
	*/
	meshModRenderer->render(
			Math::identity<Math::mat4x4>(),
			sceneIndex,
			encoder_);
}

auto Gui::tacticalMapView(double deltaT_, std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	cameraMode = CameraMode::FPS;

	std::string comboString = {};
	std::vector<std::string> nameLookup;
	nameLookup.reserve(tacticalMaps.size());
	for(auto const&[name, index] : tacticalMaps)
	{
		comboString += name + '\0';
		nameLookup.push_back(name);
	}

	ImGui::Combo("Select Mesh", &tacMapViewSelectedItem, comboString.c_str());
	if(tacMapViewSelectedItem >= (int) nameLookup.size()) return;

	auto const sceneIndex = tacticalMaps[nameLookup[tacMapViewSelectedItem]].first;
	auto const& tiles = tacticalMaps[nameLookup[tacMapViewSelectedItem]].second;

	Geometry::AABB total;
	for(auto const& tile : *tiles)
	{
		total.expandBy(tile.aabb);
	}
	arcBallFocusPoint = total.getBoxCenter();

	yrot += float(deltaT_ * 0.5);
	/*		rootScene->transform.orientation = Math::rotate(
				Math::identity<Math::quat>(),
				yrot,
				Math::vec3(0, 1, 0));
		rootScene->transform.orientation = Math::rotate(
				rootScene->transform.orientation,
				yrot / 2.0f,
				Math::vec3(0, 0, 1));
	*/
	meshModRenderer->render(
			Math::identity<Math::mat4x4>(),
			sceneIndex,
			encoder_);
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
			if(ImGui::MenuItem("TacticalMaps", nullptr))
			{
				mainView = MainViewType::TacticalMaps;
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

			// special case some binary in json format
			switch(item.type)
			{
				case Items::TacticalMapType:
				{
					auto logString = fmt::format("({}s)[{}]: BINARY_DATA", item.timeStamp, typeString);
					ImGui::Text("%s", logString.c_str());
					break;
				}
				default:
				{
					auto logString = fmt::format("({}s)[{}]: {}", item.timeStamp, typeString, item.data);
					ImGui::Text("%s", logString.c_str());
					break;
				}
			}
		}
		if(item.type == Items::LogType)
		{
			decodeLog(item);
		}
	}
	ImGui::EndChild();
	ImGui::End();

}

template<typename T>
auto safe_get(picojson::object const& j_,
			  std::string_view const& s_,
			  T const&& default_ = {}) -> T const
{
	auto const it = j_.find(std::string(s_));
	if(it != j_.cend())
	{
		if(it->second.is<T>())
		{
			return std::move(it->second.get<T>());
		}
	}
	return std::move(default_);
}

template<typename T>
auto safe_get(picojson::array const& j_,
			  size_t index_,
			  T const&& default_ = {}) -> T const
{
	if(index_ < j_.size())
	{
		auto const& f = j_[index_];
		if(f.is<T>())
		{
			return std::move(f.get<T>());
		}
	}
	return std::move(default_);
}

template<typename T>
auto safe_get(picojson::array const& j_) -> T const
{
	using t = typename T::value_type;

	T result(j_.size());
	for(auto i = 0u; i < j_.size(); ++i)
	{
		result[i] = safe_get<t>(j_, i);
	}

	return result;
}

template<>
auto safe_get<std::vector<float>>(
		picojson::object const& j_,
		std::string_view const& s_,
		std::vector<float> const&& default_) -> std::vector<float> const
{
	auto const it = j_.find(std::string(s_));
	if(it != j_.cend())
	{
		if(it->second.is<picojson::array>())
		{
			auto pa = it->second.get<picojson::array>();
			return safe_get<std::vector<float>>(pa);
		}
	}
	return default_;
}

template<>
auto safe_get<std::vector<uint32_t>>(
		picojson::object const& j_,
		std::string_view const& s_,
		std::vector<uint32_t> const&& default_) -> std::vector<uint32_t> const
{
	auto const it = j_.find(std::string(s_));
	if(it != j_.cend())
	{
		if(it->second.is<picojson::array>())
		{
			auto pa = it->second.get<picojson::array>();
			return safe_get<std::vector<uint32_t>>(pa);
		}
	}
	return default_;
}

template<>
auto safe_get<float>(
		picojson::object const& j_,
		std::string_view const& s_,
		float const&& default_) -> float const
{
	auto d = safe_get<double>(j_, s_, double(default_));
	// TODO assert range?
	return (float) d;
}

template<>
auto safe_get<float>(
		picojson::array const& j_,
		size_t index_,
		float const&& default_) -> float const
{
	auto d = safe_get<double>(j_, index_, double(default_));
	// TODO assert range?
	return (float) d;
}

template<>
auto safe_get<uint32_t>(
		picojson::object const& j_,
		std::string_view const& s_,
		uint32_t const&& default_) -> uint32_t const
{
	auto d = safe_get<double>(j_, s_, double(default_));
	// TODO assert range?
	return (uint32_t) d;
}

template<>
auto safe_get<uint32_t>(
		picojson::array const& j_,
		size_t index_,
		uint32_t const&& default_) -> uint32_t const
{
	auto d = safe_get<double>(j_, index_, double(default_));
	// TODO assert range?
	return (uint32_t) d;
}

template<>
auto safe_get<Math::vec3>(
		picojson::object const& j_,
		std::string_view const& s_,
		Math::vec3 const&& default_) -> Math::vec3 const
{
	using namespace std::literals;
	auto const it = j_.find(std::string(s_));
	if(it != j_.cend())
	{
		picojson::value field = it->second;
		if(field.is<std::string>())
		{
			auto const& pj = it->second.get<std::string>();
			picojson::parse(field, pj);
			auto const& err = picojson::get_last_error();
			if(!err.empty())
			{
				auto logString = fmt::format("!ERROR!Vec3 Parse error {} from {}",
											 err,
											 pj);
				LOG_S(WARNING) << logString;
				ImGui::Text("%s", logString.c_str());
				return Math::vec3{};
			}
		}

		if(field.is<picojson::object>())
		{
			auto const& jo = field.get<picojson::object>();
			return Math::vec3{
					safe_get<float>(jo, "x"sv),
					safe_get<float>(jo, "y"sv),
					safe_get<float>(jo, "z"sv)};
		} else if(field.is<picojson::array>())
		{
			auto const& ja = field.get<picojson::array>();
			return Math::vec3{
					safe_get<float>(ja, 0),
					safe_get<float>(ja, 1),
					safe_get<float>(ja, 2)};
		}
	}

	return default_;
}

auto Gui::decodeLog(Item const& item_) -> void
{
	using namespace std::literals;

	picojson::value pj;
	picojson::parse(pj, item_.data);
	auto err = picojson::get_last_error();
	if(!err.empty())
	{
		auto logString = fmt::format("({}s):!ERROR!Log Parse error {} from {}",
									 item_.timeStamp,
									 err,
									 item_.data);
		LOG_S(WARNING) << logString;
		ImGui::Text("%s", logString.c_str());
		return;
	}

	picojson::object o;
	if(!pj.is<picojson::object>())
	{
		auto logString = fmt::format("({}s):!ERROR!Log not a json object {}",
									 item_.timeStamp,
									 item_.data);
		return;
	}

	o = pj.get<picojson::object>();

	auto text = safe_get<std::string>(o, "text"sv);
	std::string level = safe_get<std::string>(o, "level"sv, "info"s);

	transform(level.cbegin(),
			  level.cend(),
			  level.begin(), &tolower);

	std::string logString = fmt::format(
			"({}s):[{}]{}",
			item_.timeStamp,
			level,
			text);
	if(o.find("position") != o.cend())// && ImGui::IsItemHovered())
	{
		Math::vec3 pos = safe_get<Math::vec3>(o, "position"sv);
//		logString += fmt::format(" {},{},{}", pos.x, pos.y, pos.z);
		spatialMarkers.push_back({pos});
	}

	ImGui::Text("%s", logString.c_str());
}

auto Gui::decodeSimpleMesh(Item const& item_) -> void
{
	std::lock_guard lock(lockMutex);
	using namespace std::literals;

	picojson::value pj;
	picojson::parse(pj, item_.data);
	auto err = picojson::get_last_error();
	if(!err.empty())
	{
		auto logString = fmt::format("({}s):!ERROR!SimpleMesh Parse error {} from {}",
									 item_.timeStamp,
									 err,
									 item_.data);
		LOG_S(WARNING) << logString;
		ImGui::Text("%s", logString.c_str());
		return;
	}
	picojson::object o;
	if(!pj.is<picojson::object>())
	{
		auto logString = fmt::format("({}s):!ERROR!SimpleMesh not a json object {}",
									 item_.timeStamp,
									 item_.data);
		return;
	}

	o = pj.get<picojson::object>();
	auto name = safe_get<std::string>(o, "name"sv);

	// check cache
	if(meshMap.find(name) != meshMap.end())
	{
		return;
	}

	auto posCount = safe_get<uint32_t>(o, "positioncount"sv);
	auto triCount = safe_get<uint32_t>(o, "trianglecount"sv);
	auto positions = safe_get<std::vector<float>>(o, "positions"sv);
	auto indices = safe_get<std::vector<uint32_t>>(o, "indices"sv);

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
				VertexIndex(indices[(i * 3) + 0]),
				VertexIndex(indices[(i * 3) + 2]),
				VertexIndex(indices[(i * 3) + 1])
		};
		mesh->getPolygons().addPolygon(triIndices);
	}
	mesh->updateFromEdits();

	auto rootScene = std::make_shared<SceneNode>();
	rootScene->addObject(mesh);
	auto sceneIndex = meshModRenderer->addScene(rootScene);
	meshMap[name] = sceneIndex;
}

auto Gui::decodeMeshObject(Item const& item_) -> void
{
	std::lock_guard lock(lockMutex);
	using namespace std::literals;

	picojson::value pj;
	picojson::parse(pj, item_.data);
	auto err = picojson::get_last_error();
	if(!err.empty())
	{
		auto logString = fmt::format("({}s):!ERROR!MeshObject Parse error {} from {}",
									 item_.timeStamp,
									 err,
									 item_.data);
		LOG_S(WARNING) << logString;
		ImGui::Text("%s", logString.c_str());
		return;
	}
	picojson::object o;
	if(!pj.is<picojson::object>())
	{
		auto logString = fmt::format("({}s):!ERROR!MeshObject not a json object {}",
									 item_.timeStamp,
									 item_.data);
		return;
	}

	o = pj.get<picojson::object>();
	auto name = safe_get<std::string>(o, "name"sv);

	MeshObject& mo = meshObjectMap[name];
	if(mo.index == MidRender::SceneIndex(~0))
	{
		mo.index = fallbackSceneIndex;
	}

	auto meshname = safe_get<std::string>(o, "meshname"sv);
	if(!meshname.empty())
	{
		auto meshIt = meshMap.find(meshname);
		if(meshIt != meshMap.cend())
		{
			mo.index = meshIt->second;
		}
	}

	if(o.find("position") != o.cend())
	{
		mo.position = safe_get<Math::vec3>(o, "position"sv);
	}

	if(o.find("rotation") != o.cend())
	{
		mo.rotation = safe_get<Math::vec3>(o, "rotation"sv);
	}

	if(o.find("scale") != o.cend())
	{
		mo.scale = safe_get<Math::vec3>(o, "scale"sv);
	}
}

auto Gui::meshCallback(Item const& item_) -> bool
{
	decodeSimpleMesh(item_);
	return true;
}

auto Gui::tacmapCallback(Item const& item_) -> bool
{
	using namespace std::literals;
	std::lock_guard lock(lockMutex);

	std::istringstream stream(item_.data);

	std::vector<std::shared_ptr<TacticalMap>> tactMaps;
	bool okay = TacticalMap::createFromStream(stream, tactMaps);
	if(okay)
	{
		for(auto const& tmap : tactMaps)
		{
			auto rootScene = std::make_shared<MeshMod::SceneNode>();
			auto renderTiles = std::make_shared<std::vector<TacMapTile>>();
			auto combinedMesh = std::make_shared<MeshMod::Mesh>("Tactical Map Combined Mesh");

			for(auto z = 0; z < tmap->getHeight(); ++z)
			{
				for(auto x = 0; x < tmap->getWidth(); ++x)
				{
					tmapTileGenerateMesh(x, z, rootScene, combinedMesh, renderTiles, tmap);
				}
			}
			rootScene->addObject(combinedMesh);

			tacticalMaps[tmap->getName()] = {
					meshModRenderer->addScene(rootScene),
					renderTiles
			};
		}
	}

	return okay;
}

auto Gui::tmapTileGenerateMesh(
		int x_,
		int z_,
		std::shared_ptr<MeshMod::SceneNode>& rootScene_,
		std::shared_ptr<MeshMod::Mesh>& combinedMesh, 
		std::shared_ptr<std::vector<TacMapTile>>& renderTiles_,
		std::shared_ptr<TacticalMap> const& tmap_) -> void
{
	auto const& tile = tmap_->getTile(x_, z_);
	Math::vec2 tilePos = tmap_->getBottomLeft() + Math::vec2(x_, z_);

	TacMapTile renderTile;

	float minHeight = std::numeric_limits<float>::max();
	float maxHeight = std::numeric_limits<float>::lowest();

	for(auto i = 0u; i < tile.levelCount; ++i)
	{
		auto const& level = tmap_->getLevel(tile, i);
		minHeight = std::min(minHeight, level.baseHeight);
		maxHeight = std::max(maxHeight, level.baseHeight + level.roofDeltaHeight);

		MeshOps::BasicMeshOps::combine(				
				MeshOps::Shapes::CreateSquare(
						{tilePos.x, level.baseHeight, tilePos.y},
						{0.0f, 1.0f, 0.0f}
				),
				combinedMesh);
	}

	renderTile.aabb = Geometry::AABB(
			Math::vec3(tilePos.x, minHeight, tilePos.y),
			Math::vec3(tilePos.x + 1.0f, maxHeight, tilePos.y + 1.0f)
	);
	renderTile.objectIndex = rootScene_->getObjectCount();
	//	rootScene_->addObject(MeshOps::Shapes::CreateAABB(renderTile.aabb));

	renderTiles_->push_back(renderTile);
}

}