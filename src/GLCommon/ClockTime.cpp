

#include "ClockTime.h"
#include <windows.h>

namespace clk
{
	void Time::Start()
	{
		start = GetTime();
	}
	
	void Time::Stop()
	{
		stop = GetTime();
	}

	float Time::GetIntervalMsec()
	{
		return clk::GetIntervalMsec(start, stop);
	}

	float Time::GetIntervalSec()
	{
		return clk::GetIntervalSec(start, stop);
	}

	time GetTime()
	{
		LARGE_INTEGER now;
		static LARGE_INTEGER frequency;
		static int gotfrequency = 0;
		time seconds, nsec;

		QueryPerformanceCounter(&now);
		if (gotfrequency == 0) {
			QueryPerformanceFrequency(&frequency);
			gotfrequency = 1;
		}

		seconds = now.QuadPart / frequency.QuadPart;
		nsec = (1000000000I64 * (now.QuadPart - (seconds * frequency.QuadPart))) / frequency.QuadPart;
    
		return seconds * 1000000000I64 + nsec;
	}

	time GetInterval(const time& start, const time& end)
	{
		return end - start;
	}

	float GetIntervalMsec(const time& start, const time& end)
	{
		return (float) (end - start) * 0.000001f;
	}
	
	float GetIntervalSec(const time& start, const time& end)
	{
		return (float) (end - start) * 0.000000001f;
	}

	float Msec(const time& t)
	{
		return t * 0.000000001f;
	}

	float Sec(const time& t)
	{
		return t * 0.000000001f;
	}

}	// namespace clk
