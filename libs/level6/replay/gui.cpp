#include "core/core.h"
#include "replay/gui.h"
#include "replay/replay.h"
#include "midrender/imguibindings.h"
#include "fmt/format.h"
#include "replay/items.h"
#include "replay/propertyeditor.h"

namespace Replay {
Gui::Gui(std::shared_ptr<Replay> const& replay_) :
		replay(replay_),
		viewerTime(-1.0),
		logFilter(ItemType(0))
{
}

Gui::~Gui()
{
}

auto Gui::render() -> void
{
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
	ImGuiWindowFlags logWindowFlags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize;
	ImGui::SetNextWindowPos(ImVec2(20, 600), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(1260, 160), ImGuiCond_FirstUseEver);
	ImGui::Begin("Log", nullptr, logWindowFlags);

	int tmpLF = int(logFilter);
	ImGui::RadioButton("All", &tmpLF, 0); ImGui::SameLine();
	ImGui::RadioButton("TEST", &tmpLF, (int)Items::TestType); ImGui::SameLine();
	ImGui::RadioButton("LOG", &tmpLF, (int)Items::LogType);
	logFilter = ItemType(tmpLF);

	double const time = (viewerTime < 0.0) ? replay->getCurrentTime() : viewerTime;
	auto items = replay->getRange(time - 1.0, time, logFilter);

	ImGui::BeginChild("");
	for(auto const& item : items)
	{
		char typeString[5];
		std::memcpy(typeString, &item.type, sizeof(uint32_t));
		typeString[4] = 0;

		auto logString = fmt::format("({}s)[{}]: {}", item.timeStamp, typeString, item.data);
		ImGui::Text(logString.c_str());
	}
	ImGui::EndChild();
	ImGui::End();

}

}