#include "Timer.h"
#include <limits.h>
#include <stdio.h>

namespace SystemClock
{
    long uptimeMillis()
    {
        // printf("uptimeMillis()\n");
        nsecs_t nti = systemTime(SYSTEM_TIME_MONOTONIC);
        return nti / 1000000LL;
    }
}

nsecs_t systemTime(int type)
{
    timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return nsecs_t(ts.tv_sec)*1000000000LL + ts.tv_nsec;
}

int toMillisecondTimeoutDelay(nsecs_t referenceTime, nsecs_t timeoutTime)
{
    nsecs_t timeoutDelayMillis;
    if (timeoutTime > referenceTime) {
        uint64_t timeoutDelay = uint64_t(timeoutTime - referenceTime);
        if (timeoutDelay > uint64_t((INT_MAX - 1) * 1000000LL)) {
            timeoutDelayMillis = -1;
        } else {
            timeoutDelayMillis = (timeoutDelay + 999999LL) / 1000000LL;
        }
    } else {
        timeoutDelayMillis = 0;
    }
    return (int)timeoutDelayMillis;
}
