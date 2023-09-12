#include "NativeLooper.h"
#include <limits.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <string.h>

NativeLooper::NativeLooper()
    : mPolling(false),
      mEpollRebuildRequired(false),
      mNextMessageUptime(LLONG_MAX)
{
    mWakeEventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

    std::lock_guard<std::mutex> lock(mLock);
    rebuildEpollLocked();
}

NativeLooper::~NativeLooper()
{

}

static const int EPOLL_MAX_EVENTS = 16;

static pthread_once_t gTLSOnce = PTHREAD_ONCE_INIT;
static pthread_key_t gTLSKey = 0;

void NativeLooper::initTLSKey() {
    int error = pthread_key_create(&gTLSKey, threadDestructor);
    // LOG_ALWAYS_FATAL_IF(error != 0, "Could not allocate TLS key: %s", strerror(error));
}

void NativeLooper::threadDestructor(void *st) {
    // NativeLooper* self = static_cast<NativeLooper*>(st);
    // if (self != nullptr) {
    //     self->decStrong((void*)threadDestructor);
    // }
    // if (self != nullptr)
    // {
    //     delete self;
    // }
}

void NativeLooper::setForThread(NativeLooper* looper) {
    NativeLooper* old = getForThread(); // also has side-effect of initializing TLS

    // if (looper != nullptr) {
    //     looper->incStrong((void*)threadDestructor);
    // }

    pthread_setspecific(gTLSKey, looper);

    if (old != nullptr) {
        // old->decStrong((void*)threadDestructor);
        delete old;
    }
}

NativeLooper* NativeLooper::getForThread() {
    int result = pthread_once(& gTLSOnce, initTLSKey);
    // LOG_ALWAYS_FATAL_IF(result != 0, "pthread_once failed");

    return (NativeLooper*)pthread_getspecific(gTLSKey);
}

NativeLooper* NativeLooper::prepare(int opts) {
    bool allowNonCallbacks = opts & PREPARE_ALLOW_NON_CALLBACKS;
    NativeLooper* looper = NativeLooper::getForThread();
    if (looper == nullptr) {
        looper = new NativeLooper();
        NativeLooper::setForThread(looper);
    }
    // if (looper->getAllowNonCallbacks() != allowNonCallbacks) {
    //     ALOGW("Looper already prepared for this thread with a different value for the "
    //             "LOOPER_PREPARE_ALLOW_NON_CALLBACKS option.");
    // }
    return looper;
}

int NativeLooper::pollOnce(int timeoutMillis, int* outFd, int* outEvents, void** outData)
{
    int result = 0;
    for (;;)
    {
        if (result != 0)
        {
            return result;
        }
        result = pollInner(timeoutMillis);
    }
}

int NativeLooper::pollInner(int timeoutMillis) 
{
    printf("NativeLooper::pollInner -> %d\n", timeoutMillis);
    if (timeoutMillis != 0 && mNextMessageUptime != LLONG_MAX)
    {
        nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
        int messageTimeoutMillis = toMillisecondTimeoutDelay(now, mNextMessageUptime);
        if (messageTimeoutMillis >= 0
                && (timeoutMillis < 0 || messageTimeoutMillis < timeoutMillis)) {
            timeoutMillis = messageTimeoutMillis;
        }
    }

    // Poll.
    int result = POLL_WAKE;

    // We are about to idle.
    mPolling = true;

    struct epoll_event eventItems[EPOLL_MAX_EVENTS];
    printf("start epoll_wait -> %d\n", timeoutMillis);
    int eventCount = epoll_wait(mEpollFd, eventItems, EPOLL_MAX_EVENTS, timeoutMillis);

    // Acquire lock.
    mLock.lock();

    // Rebuild epoll set if needed.
    if (mEpollRebuildRequired) {
        mEpollRebuildRequired = false;
        rebuildEpollLocked();
        goto Done;
    }

    // Check for poll error.
    if (eventCount < 0) {
        if (errno == EINTR) {
            goto Done;
        }
        // ALOGW("Poll failed with an unexpected error: %s", strerror(errno));
        result = POLL_ERROR;
        goto Done;
    }

    // Check for poll timeout.
    if (eventCount == 0) {
#if DEBUG_POLL_AND_WAKE
        ALOGD("%p ~ pollOnce - timeout", this);
#endif
        result = POLL_TIMEOUT;
        goto Done;
    }

    for (int i = 0; i < eventCount; i++) {
        int fd = eventItems[i].data.fd;
        uint32_t epollEvents = eventItems[i].events;
        if (fd == mWakeEventFd) {
            if (epollEvents & EPOLLIN) {
                awoken();
            } else {
                // ALOGW("Ignoring unexpected epoll events 0x%x on wake event fd.", epollEvents);
            }
        } else {

        }
    }

Done: ;

    // Invoke pending message callbacks.
    mNextMessageUptime = LLONG_MAX;

    // Release lock.
    mLock.unlock();

    // Invoke all response callbacks.
    return result;
}

void NativeLooper::awoken() {
    printf("awoken\n");
    uint64_t counter;
    TEMP_FAILURE_RETRY(read(mWakeEventFd, &counter, sizeof(uint64_t)));
    printf("awoken -> %ld\n", counter);
}

void NativeLooper::wake() {
    printf("NativeLooper::wake()\n");
    uint64_t inc = 1;
    ssize_t nWrite = TEMP_FAILURE_RETRY(write(mWakeEventFd, &inc, sizeof(uint64_t)));
    if (nWrite != sizeof(uint64_t)) {
        if (errno != EAGAIN) {
            // LOG_ALWAYS_FATAL("Could not write wake signal to fd %d (returned %zd): %s",
            //                  mWakeEventFd.get(), nWrite, strerror(errno));
        }
    }
}

bool NativeLooper::isPolling() const {
    return mPolling;
}

void NativeLooper::rebuildEpollLocked() {
    // Close old epoll instance if we have one.
    if (mEpollFd >= 0) {
        close(mEpollFd);
        mEpollFd = 0;
    }

    // Allocate the new epoll instance and register the wake pipe.
    mEpollFd = (epoll_create1(EPOLL_CLOEXEC));
    // LOG_ALWAYS_FATAL_IF(mEpollFd < 0, "Could not create epoll instance: %s", strerror(errno));

    struct epoll_event eventItem;
    memset(& eventItem, 0, sizeof(epoll_event)); // zero out unused members of data field union
    eventItem.events = EPOLLIN;
    eventItem.data.fd = mWakeEventFd;
    int result = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mWakeEventFd, &eventItem);
    // LOG_ALWAYS_FATAL_IF(result != 0, "Could not add wake event fd to epoll instance: %s",
    //                     strerror(errno));

}
