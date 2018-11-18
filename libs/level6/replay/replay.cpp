#include "core/core.h"
#include "replay/replay.h"

namespace Replay
{

Replay::Replay() : currentTime(0.0)
{

}

auto Replay::update(double deltaT_) -> void
{
	currentTime += deltaT_;
}

auto Replay::add(uint32_t type_, std::string const& data_) -> void
{
	items.emplace_back(Item{currentTime, type_, data_});
}


}