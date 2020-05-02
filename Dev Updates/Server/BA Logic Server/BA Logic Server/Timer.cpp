#include "Timer.h"

Timer::Timer()
{
	QueryPerformanceFrequency(&freq);
	reset();
	FPS = 0;
	FPS_TIME = 0;
}
Timer::~Timer() {}

void Timer::reset()
{
	QueryPerformanceCounter(&start);
	current = prev = start;
}

void Timer::setFPS(float fps)
{
	FPS = fps;
	FPS_TIME = 1.0f / FPS;
}

float Timer::total()
{
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	float tickTime = (float)(c.QuadPart - start.QuadPart) / (float)(freq.QuadPart);
	return tickTime;
}

float Timer::tick()
{
	QueryPerformanceCounter(&current);
	float tickTime = (float)(current.QuadPart - prev.QuadPart) / (float)(freq.QuadPart);

	while (tickTime < FPS_TIME)
	{
		QueryPerformanceCounter(&current);
		tickTime = (float)(current.QuadPart - prev.QuadPart) / (float)(freq.QuadPart);
	}
	
	prev = current;
	return tickTime;
}