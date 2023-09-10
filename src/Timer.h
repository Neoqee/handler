#ifndef __NEOQEE_TIMER_H__
#define __NEOQEE_TIMER_H__

#include "time.h"
#include "sys/types.h"
#include <stdint.h>

typedef int64_t nsecs_t;

namespace SystemClock
{
    long uptimeMillis();
}

enum {
    SYSTEM_TIME_REALTIME = 0,  // system-wide realtime clock
    SYSTEM_TIME_MONOTONIC = 1, // monotonic time since unspecified starting point
    SYSTEM_TIME_PROCESS = 2,   // high-resolution per-process clock
    SYSTEM_TIME_THREAD = 3,    // high-resolution per-thread clock
    SYSTEM_TIME_BOOTTIME = 4   // same as SYSTEM_TIME_MONOTONIC, but including CPU suspend time
};

nsecs_t systemTime(int type);

int toMillisecondTimeoutDelay(nsecs_t referenceTime, nsecs_t timeoutTime);


#endif