#pragma once

#ifndef WYRD_TIMING_PULSAR_H
#define WYRD_TIMING_PULSAR_H

#include "core/core.h"
#include <functional>
#include "timing/tickerclock.h"

namespace Timing
{
class Pulsar
{
public:
	using CallbackFunc = std::function<void(void)>;

	Pulsar(double timePeriod_, CallbackFunc callback_) :
			timeElapsed(0.0),
			timePeriod(timePeriod_),
			callback(callback_),
			ticker(std::make_unique<Timing::TickerClock>()) {}

	auto update() -> void
	{
		timeElapsed += ticker->update();
		while(timeElapsed > timePeriod)
		{
			callback();
			timeElapsed -= timePeriod;
		}
	}

protected:
	double timeElapsed;
	double timePeriod;
	CallbackFunc callback;
	std::unique_ptr<Timing::TickerClock> ticker;
};

class Pulsars
{
public:
	using CallbackFunc = std::function<void(void)>;
	Pulsars() : ticker(std::make_unique<Timing::TickerClock>()){}

	auto add(double timePeriod_, CallbackFunc callback_) -> void
	{
		pulsars.push_back({timePeriod_, callback_, 0.0});
	}

	auto update() -> void
	{
		double elapsed = ticker->update();
		for(auto& pulsar : pulsars)
		{
			auto [timePeriod, callback, timeElapsed] = pulsar;
			timeElapsed += elapsed;
			while(timeElapsed > timePeriod)
			{
				callback();
				timeElapsed -= timePeriod;
			}
			std::get<2>(pulsar) = timeElapsed;
		}
	}

private:
	using PulsarTuple = std::tuple<double const, CallbackFunc, double>;
	std::vector<PulsarTuple> pulsars;

	std::unique_ptr<Timing::TickerClock> ticker;

};

}

#endif //WYRD_TIMING_PULSAR_H
