#include "NativeMessageQueue.h"
#include "Log.h"

NativeMessageQueue::NativeMessageQueue()
{
    mLooper = new NativeLooper();
    LOG_I("NativeMessageQueue#%x, mLooper#%x\n", this, mLooper);
}

NativeMessageQueue::~NativeMessageQueue()
{
    LOG_I("~NativeMessageQueue#%x, mLooper#%x\n", this, mLooper);
    delete mLooper;
    mLooper = nullptr;
}

void NativeMessageQueue::pollOnce(int timeoutMillis)
{
    mLooper->pollOnce(timeoutMillis);
}

void NativeMessageQueue::wake() {
    mLooper->wake();
}