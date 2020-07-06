#pragma once
#include <chrono>

/**
@brief event structure for iocp event.
@author Gurnwoo Kim
*/
struct EVENT {
	int obj_id; ///< target obj id.
	int event_type; ///< event type for iocp.
	std::chrono::high_resolution_clock::time_point wakeup_time; ///< after arriving this time, event will execute.

	constexpr bool operator<(const EVENT& _Left) const
	{
		return (wakeup_time > _Left.wakeup_time);
	}
};