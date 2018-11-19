#pragma once
#ifndef WYRD_REPLAY_GUI_H
#define WYRD_REPLAY_GUI_H

#include "core/core.h"
#include "replay/replay.h"
namespace Replay {

// assumes imgui is already setup
class Gui
{
public:
	Gui(std::shared_ptr<Replay> const& replay_);
	~Gui();

	auto render() -> void;

protected:
	auto log() -> void;

	auto pause() -> void;

	auto play() -> void;

	std::shared_ptr<Replay> replay;
	bool windowOpen = true;

	double viewerTime;
	ItemType logFilter;
};

}

#endif //WYRD_REPLAY_GUI_H
