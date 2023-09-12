#include "HandlerThread.h"
#include "Handler.h"
#include "Looper.h"
#include "pthread.h"
#include "unistd.h"

void* threadFunc(void *ptr)
{
    HandlerThread* pHT = (HandlerThread*)ptr;
    pHT->run();
}

HandlerThread::HandlerThread()
{
    mPid = 0;
    mStarted = false;
}
HandlerThread::~HandlerThread()
{
    mLooper = nullptr;
}

void HandlerThread::run()
{
    mPid = pthread_self();
    Looper::prepare();
    mLooper = Looper::myLooper();
    Looper::loop();
}

void HandlerThread::start()
{
    mStarted = true;
    // pthread_t pid;
    pthread_create(&mPid, nullptr, threadFunc, (void*)this);
    pthread_detach(mPid);
}

int HandlerThread::getThreadId()
{
    return mPid;
}

Looper* HandlerThread::getLooper()
{
    if (!mStarted) return nullptr;
    while (mStarted && mLooper == nullptr)
    {
        sleep(1);
    }
    return mLooper;
}
// Handler* HandlerThread::getThreadHandler()
// {
//     if (mHandler == nullptr)
//     {
//         mHandler = new Handler(getLooper(), nullptr);
//     }
//     return mHandler;
// }
