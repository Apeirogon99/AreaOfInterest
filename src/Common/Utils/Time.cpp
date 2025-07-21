#include "Common/Utils/Time.h"

__int64 Time::GetCurrentTimeMs()
{
	auto now = std::chrono::steady_clock::now();
	auto duration = now.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}