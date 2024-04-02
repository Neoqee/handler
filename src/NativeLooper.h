#ifndef __NEOQEE_NATIVELOOPER_H__
#define __NEOQEE_NATIVELOOPER_H__

#include "Timer.h"
#include <sys/epoll.h>
#include <mutex>
#include <sys/types.h>

class NativeLooper
{
public:
    
    enum {
        /**
         * Result from Looper_pollOnce() and Looper_pollAll():
         * The poll was awoken using wake() before the timeout expired
         * and no callbacks were executed and no other file descriptors were ready.
         */
        POLL_WAKE = -1,

        /**
         * Result from Looper_pollOnce() and Looper_pollAll():
         * One or more callbacks were executed.
         */
        POLL_CALLBACK = -2,

        /**
         * Result from Looper_pollOnce() and Looper_pollAll():
         * The timeout expired.
         */
        POLL_TIMEOUT = -3,

        /**
         * Result from Looper_pollOnce() and Looper_pollAll():
         * An error occurred.
         */
        POLL_ERROR = -4,
    };

    NativeLooper();
    virtual ~NativeLooper();

    int pollOnce(int timeoutMillis);

    int pollInner(int timeoutMillis);

    void wake();

    bool isPolling() const;

    void awoken();

    void rebuildEpollLocked();

private:
    // const bool mAllowNonCallbacks; // immutable

    int mWakeEventFd;  // immutable
    std::mutex mLock;

    // Whether we are currently waiting for work.  Not protected by a lock,
    // any use of it is racy anyway.
    volatile bool mPolling;

    int mEpollFd;  // guarded by mLock but only modified on the looper thread
    bool mEpollRebuildRequired; // guarded by mLock


    nsecs_t mNextMessageUptime; // set to LLONG_MAX when none

};

#endif
