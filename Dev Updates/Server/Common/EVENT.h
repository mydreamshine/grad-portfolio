#pragma once
#include <chrono>

template <class T>
struct EVENT {
	int obj_id;
	std::chrono::high_resolution_clock::time_point wakeup_time;
	T event_type;

	constexpr bool operator<(const EVENT& _Left) const
	{
		return (wakeup_time > _Left.wakeup_time);
	}
};