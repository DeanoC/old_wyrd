#pragma once
#ifndef WYRD_REPLAY_REPLAY_H
#define WYRD_REPLAY_REPLAY_H

#include "core/core.h"
#include <vector>

namespace Replay {

struct Item
{
	double timeStamp;
	uint32_t id;
	std::string data;
};

class Replay
{
public:
	friend class Gui;
	Replay();

	auto update(double deltaT_) -> void;

	auto add(uint32_t type_, std::string const& data_) -> void;

	auto getCurrentTime() const -> double { return currentTime; }

protected:
	std::vector<Item> items;

	double currentTime;
};

}

#endif //WYRD_REPLAY_REPLAY_H
