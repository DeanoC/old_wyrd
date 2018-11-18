#include "core/core.h"
#include "timing/tickerclock.h"

namespace Timing
{
TickerClock::TickerClock() :
	ticks(0)
{
#if PLATFORM == WINDOWS
	LARGE_INTEGER tmp;
	if ( QueryPerformanceFrequency( &tmp ) != TRUE ) {
		throw std::exception( "QueryPerformanceFrequency unsupported." );
	}
	timeFrequency = tmp.QuadPart;
#elif PLATFORM == POSIX
	timeFrequency = 1000000; // micro seconds
#else
	static_assert(false, "No TickerClock implementation on this platform")
#endif
}

/// Updates internal 'frame' time ; returns delta time in seconds
auto TickerClock::update() -> double
{
	uint64_t const lastTicks = ticks;

#if PLATFORM == WINDOWS
	LARGE_INTEGER tmp;
	QueryPerformanceCounter( &tmp );
	ticks = tmp.QuadPart;
#elif PLATFORM == POSIX
	struct timeval time;
    /* Grab the current time. */
    gettimeofday(&time, NULL);
    ticks = time.tv_usec + time.tv_sec * 1000000;
#endif

	uint64_t const deltaTicks = ticks - lastTicks;

	return ticksToTime(deltaTicks);
}

}
