#include "Common/Utils/Time.h"

TimeManager gTimeManager;

void TimeManager::Initialize()
{
	timeBeginPeriod(1);
	QueryPerformanceFrequency(&mFreq);
	QueryPerformanceCounter(&mStart);
	mLast = mStart;
}

long long TimeManager::GetServerTime()
{
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);
	return (current.QuadPart - mStart.QuadPart) * 1000 / mFreq.QuadPart;
}

float TimeManager::GetCurrentTimeMS()
{
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);
	return (float)(current.QuadPart - mStart.QuadPart) * 1000.0f / mFreq.QuadPart;
}

float TimeManager::GetDeltaTime()
{
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);
	float delta = (float)(current.QuadPart - mLast.QuadPart) / mFreq.QuadPart;
	mLast = current;
	return delta;
}