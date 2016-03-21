#ifndef __CLOCK_TIME_H__
#define __CLOCK_TIME_H__

#include <iostream>



namespace clk
{
	typedef __int64 time;

	struct Time
	{
		time start;
		time stop;

		void Start();
		void Stop();
		float GetIntervalMsec();
		float GetIntervalSec();
	};

	time GetTime();
	time GetInterval(const time& start, const time& end);
	
	float GetIntervalMsec(const time& start, const time& end);
	
	float GetIntervalSec(const time& start, const time& end);
	
	float Msec(const time& t);
	
	float Sec(const time& t);
	
}

// 
// Calculate FPS and print to stderr
//
static void PrintFPS(void)
{
	static __int64 t0 = -1;
	static int frames = 0;
	__int64 theTime;	
	__int64 t;
	float __fps = 0.0;

	theTime = clk::GetTime();
	t = clk::Sec(clk::GetTime());

	if (t0 < 0)
		t0 = t;

	frames++;

	if (t - t0 >= 5.0) 
	{
		float seconds = t - t0;
		__fps = frames / seconds;
		std::cout << frames << " frames in " << seconds << " seconds = " << __fps << " fps" <<std::endl;
		
		t0 = t;
		frames = 0;
	}
}

// 
// Calculate FPS and return as a float.
//
static float CalcFPS(void)
{
	static __int64 t0 = -1;
	static int frames = 0;
	__int64 theTime;	
	__int64 t;
	static float __fps = 0.0;

	theTime = clk::GetTime();
	t = theTime / 1000000000;

	if (t0 < 0)
		t0 = t;

	frames++;

	if (t - t0 >= 2.0) {
		float seconds = t - t0;
		__fps = frames / seconds;
		t0 = t;
		frames = 0;
	}

	return __fps;
}


#endif	//__CLOCK_TIME_H__