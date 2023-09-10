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

    /**
     * Flags for file descriptor events that a looper can monitor.
     *
     * These flag bits can be combined to monitor multiple events at once.
     */
    enum {
        /**
         * The file descriptor is available for read operations.
         */
        EVENT_INPUT = 1 << 0,

        /**
         * The file descriptor is available for write operations.
         */
        EVENT_OUTPUT = 1 << 1,

        /**
         * The file descriptor has encountered an error condition.
         *
         * The looper always sends notifications about errors; it is not necessary
         * to specify this event flag in the requested event set.
         */
        EVENT_ERROR = 1 << 2,

        /**
         * The file descriptor was hung up.
         * For example, indicates that the remote end of a pipe or socket was closed.
         *
         * The looper always sends notifications about hangups; it is not necessary
         * to specify this event flag in the requested event set.
         */
        EVENT_HANGUP = 1 << 3,

        /**
         * The file descriptor is invalid.
         * For example, the file descriptor was closed prematurely.
         *
         * The looper always sends notifications about invalid file descriptors; it is not necessary
         * to specify this event flag in the requested event set.
         */
        EVENT_INVALID = 1 << 4,
    };

    enum {
        /**
         * Option for Looper_prepare: this looper will accept calls to
         * Looper_addFd() that do not have a callback (that is provide NULL
         * for the callback).  In this case the caller of Looper_pollOnce()
         * or Looper_pollAll() MUST check the return from these functions to
         * discover when data is available on such fds and process it.
         */
        PREPARE_ALLOW_NON_CALLBACKS = 1<<0
    };

    NativeLooper();
    virtual ~NativeLooper();

    int pollOnce(int timeoutMillis, int* outFd, int* outEvents, void** outData);
    inline int pollOnce(int timeoutMillis) {
        return pollOnce(timeoutMillis, nullptr, nullptr, nullptr);
    }

    int pollInner(int timeoutMillis);

    void wake();

    bool isPolling() const;

    void awoken();

    void rebuildEpollLocked();

    /**
     * Prepares a looper associated with the calling thread, and returns it.
     * If the thread already has a looper, it is returned.  Otherwise, a new
     * one is created, associated with the thread, and returned.
     *
     * The opts may be PREPARE_ALLOW_NON_CALLBACKS or 0.
     */
    static NativeLooper* prepare(int opts);

    /**
     * Sets the given looper to be associated with the calling thread.
     * If another looper is already associated with the thread, it is replaced.
     *
     * If "looper" is NULL, removes the currently associated looper.
     */
    static void setForThread(NativeLooper* looper);

    /**
     * Returns the looper associated with the calling thread, or NULL if
     * there is not one.
     */
    static NativeLooper* getForThread();

private:
    // const bool mAllowNonCallbacks; // immutable

    int mWakeEventFd;  // immutable
    std::mutex mLock;

    // Whether we are curre__NEOQEE_MESSAGEQUEUE_H__y anyway.
    volatile bool mPolling;

    int mEpollFd;  // guarded by mLock but only modified on the looper thread
    bool mEpollRebuildRequired; // guarded by mLock

    // Locked list of file descriptor monitoring requests.

    nsecs_t mNextMessageUptime; // set to LLONG_MAX when none

    static void initTLSKey();
    static void threadDestructor(void *st);
    static void initEpollEvent(struct epoll_event* eventItem);
};

#endif
