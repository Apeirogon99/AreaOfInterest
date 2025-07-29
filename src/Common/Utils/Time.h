#pragma once

#pragma comment(lib, "winmm.lib")
#include <windows.h>

class TimeManager
{
public:
    void Initialize();
    long long GetServerTime();
    float GetCurrentTimeMS();
    float GetDeltaTime();

private:
    LARGE_INTEGER mFreq;
    LARGE_INTEGER mStart;
    LARGE_INTEGER mLast;
};

extern TimeManager gTimeManager;