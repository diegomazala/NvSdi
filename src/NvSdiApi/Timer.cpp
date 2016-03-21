#include "common.h"

//
// Return nanosecond clock value.
//
__int64 GetNanoClock()
{
	LARGE_INTEGER now;
	static LARGE_INTEGER frequency;
	static int gotfrequency = 0;
	__int64 seconds, nsec;

	QueryPerformanceCounter(&now);
	if (gotfrequency == 0) {
		QueryPerformanceFrequency(&frequency);
		gotfrequency = 1;
	}

	seconds = now.QuadPart / frequency.QuadPart;
	nsec = (1000000000I64 * (now.QuadPart - (seconds * frequency.QuadPart))) / frequency.QuadPart;
    
	return seconds * 1000000000I64 + nsec;
}

// 
// Calculate FPS and print to stderr
//
void PrintFPS(void)
{
	static __int64 t0 = -1;
	static int frames = 0;
	__int64 theTime;	
	__int64 t;
	float __fps = 0.0;

	theTime = GetNanoClock();
	t = theTime / 1000000000;

	if (t0 < 0)
		t0 = t;

	frames++;

	if (t - t0 >= 5.0) 
	{
		float seconds = t - t0;
		__fps = frames / seconds;
		std::cerr << frames << " frames in " << seconds << " seconds = " << __fps << " fps" << std::endl;
		t0 = t;
		frames = 0;
	}
}

// 
// Calculate FPS and return as a float.
//
float CalcFPS(void)
{
	static __int64 t0 = -1;
	static int frames = 0;
	__int64 theTime;	
	__int64 t;
	static float __fps = 0.0;

	theTime = GetNanoClock();
	t = theTime / 1000000000;

	if (t0 < 0)
		t0 = t;

	frames++;

	if (t - t0 >= 2.0) {
		GLfloat seconds = t - t0;
		__fps = frames / seconds;
		t0 = t;
		frames = 0;
	}

	return __fps;
}
