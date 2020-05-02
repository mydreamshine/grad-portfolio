#pragma once
#include <Windows.h>

class Timer
{
private:
	LARGE_INTEGER start;
	LARGE_INTEGER prev;
	LARGE_INTEGER current;
	LARGE_INTEGER freq;
	float FPS;
	float FPS_TIME;
public:
	Timer();
	~Timer();
	void setFPS(float fps);
	void reset();
	float tick();
	float total();
};