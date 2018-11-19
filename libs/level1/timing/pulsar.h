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
}

#endif //WYRD_TIMING_PULSAR_H
