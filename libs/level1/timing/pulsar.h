#pragma once

#ifndef WYRD_TIMING_PULSAR_H
#define WYRD_TIMING_PULSAR_H

#include "core/core.h"
#include <functional>

namespace Timing
{
class TickerClock;

class Pulsar
{
	using callback = std::function<void(void)>;

protected:
	double timeElapsed;
	std::unique_ptr<Timing::TickerClock> ticker;

};
}

#endif //WYRD_TIMING_PULSAR_H
