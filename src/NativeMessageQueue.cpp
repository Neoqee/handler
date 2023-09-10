#include "NativeMessageQueue.h"

// Must be kept in sync with the constants in Looper.FileDescriptorCallback
static const int CALLBACK_EVENT_INPUT = 1 << 0;
static const int CALLBACK_EVENT_OUTPUT = 1 << 1;
static const int CALLBACK_EVENT_ERROR = 1 << 2;

NativeMessageQueue::NativeMessageQueue()
{
    printf("NativeMessageQueue()\n");
    mLooper = NativeLooper::getForThread();
    if (mLooper == nullptr)
    {
        mLooper = new NativeLooper();
        NativeLooper::setForThread(mLooper);
    }
}

NativeMessageQueue::~NativeMessageQueue()
{

}

void NativeMessageQueue::pollOnce(int timeoutMillis)
{
    printf("pollOnce()\n");
    mLooper->pollOnce(timeoutMillis);
}

void NativeMessageQueue::wake() {
    mLooper->wake();
}

int NativeMessageQueue::handleEvent(int fd, int looperEvents, void* data) {
    int events = 0;
    if (looperEvents & NativeLooper::EVENT_INPUT) {
        events |= CALLBACK_EVENT_INPUT;
    }
    if (looperEvents & NativeLooper::EVENT_OUTPUT) {
        events |= CALLBACK_EVENT_OUTPUT;
    }
    if (looperEvents & (NativeLooper::EVENT_ERROR | NativeLooper::EVENT_HANGUP | NativeLooper::EVENT_INVALID)) {
        events |= CALLBACK_EVENT_ERROR;
    }
    int oldWatchedEvents = reinterpret_cast<long>(data);
    // int newWatchedEvents = mPollEnv->CallIntMethod(mPollObj,
    //         gMessageQueueClassInfo.dispatchEvents, fd, events);
    // if (!newWatchedEvents) {
    //     return 0; // unregister the fd
    // }
    // if (newWatchedEvents != oldWatchedEvents) {
    //     setFileDescriptorEvents(fd, newWatchedEvents);
    // }
    return 1;
}