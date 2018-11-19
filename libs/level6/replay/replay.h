#pragma once
#ifndef WYRD_REPLAY_REPLAY_H
#define WYRD_REPLAY_REPLAY_H

#include "core/core.h"
#include <vector>
#include <mutex>

namespace Replay {

enum class ItemType : uint32_t;

struct Item
{
	double timeStamp;
	ItemType type;
	std::string data;
};

class Replay
{
public:
	friend class Gui;
	Replay();

	auto update(double deltaT_) -> void;

	auto add(ItemType type_, std::string const& data_) -> void;

	auto getCurrentTime() const -> double { return currentTime; }

	// returns a range of item between the start time and end time
	// if typeFilter != 0 only grabs items of that type
	auto getRange(double const startTime_, double const endTime_, ItemType typeFilter_ = ItemType(0)) const -> std::vector<Item>;

protected:
	using ItemContainer = std::vector<Item>;
	ItemContainer items;
	mutable std::mutex lookupMutex;

	double currentTime;
};

}

#endif //WYRD_REPLAY_REPLAY_H


