#ifndef _UTIL_H_
#define _UTIL_H_

#include <chrono>

typedef std::chrono::high_resolution_clock::time_point time_value;
typedef long long millisecond_time_difference;

inline time_value get_current_time()
{
	return std::chrono::high_resolution_clock::now();
}

inline millisecond_time_difference time_diff(time_value time1, time_value time2)
{
	return (millisecond_time_difference) std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time2).count();
}
#endif // !_UTIL_H_