#pragma once
#ifndef WYRD_TIMING_TICKERCLOCK_H
#define WYRD_TIMING_TICKERCLOCK_H

namespace Timing {
// ticker clock is an internal 64 bit high precision timer
// but returns the time in double seconds for easy use
class TickerClock
{
public:
	TickerClock();

	// updates the internal time and return elapsed time
	auto  update() -> double;

private:
	// not this is a lossy conversion but is easier to use
 	auto ticksToTime(uint64_t ticks_) const -> double { return double(ticks_) / double(timeFrequency); }

	uint64_t ticks;
 	uint64_t timeFrequency;
};

}

#endif //TIMING_
