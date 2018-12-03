#include "core/core.h"
#include "replay/replay.h"
#include <mutex>
#include <algorithm>
namespace Replay
{

Replay::Replay() : currentTime(0.0)
{
}

auto Replay::update(double deltaT_) -> void
{
	// don't start timer until we get out first bit of data
	if(items.empty()) return;

	currentTime += deltaT_;
}

auto Replay::add(ItemType type_, std::string const& data_) -> void
{
	std::lock_guard guard(lookupMutex);

	Item item{currentTime, type_, data_};
	auto callbackIt = callbacks.find(type_);
	if(callbackIt != callbacks.cend())
	{
		item.hidden = callbackIt->second(item);
	}
	items.emplace_back(item);

	// not neccesary are currentTime is always same or + currently
	std::sort(
			items.begin(),
			items.end(),
			[](Item const& a_, Item const& b_)
			{
				return a_.timeStamp	< b_.timeStamp;
			});
}

auto Replay::getRange(double const startTime_, double const endTime_, ItemType typeFilter_) const -> std::vector<Item>
{
	std::lock_guard guard(lookupMutex);

	auto pred = [](Item const& a_,Item const& b_ ) -> bool
	{
		return a_.timeStamp < b_.timeStamp;
	};

	auto lower = std::lower_bound(items.cbegin(), items.cend(), Item{ startTime_ }, pred);
	auto upper = std::upper_bound(items.cbegin(), items.cend(), Item{ endTime_ }, pred);

	std::vector<Item> out;
	for (auto it = lower; it != upper; ++it )
	{
		auto const& item = *it;
		if(typeFilter_ != ItemType(0))
		{
			if(item.hidden) continue;
			if(item.type != typeFilter_) continue;
		}
		out.push_back(*it);
	}

	return out;
}
auto Replay::registerCallback(ItemType type_, Replay::CallbackFunc const& callback_) -> void
{
	std::lock_guard guard(lookupMutex);
	callbacks[type_] = callback_;
}

}