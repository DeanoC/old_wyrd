#include "core/core.h"
#include "replay/gui.h"
#include "replay/replay.h"
#include "midrender/imguibindings.h"
#include "fmt/format.h"

namespace Replay
{
Gui::Gui(std::shared_ptr<Replay> const& replay_) :
	replay(replay_),
	viewerTime(-1.0)
{
}

Gui::~Gui()
{
}

auto Gui::render() -> void
{
	if (!ImGui::Begin("Replay", &windowOpen))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	double const time = (viewerTime < 0.0) ? replay->getCurrentTime() : viewerTime;
	auto timeString = fmt::format("Time: {}s",time);
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

	ImGui::End();
}

auto Gui::pause() -> void
{
	viewerTime = replay->getCurrentTime();
}

auto Gui::play() -> void
{
	viewerTime = -1;
}

}