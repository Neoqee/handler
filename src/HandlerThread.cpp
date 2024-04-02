#include "HandlerThread.h"
#include "Handler.h"
#include "Looper.h"
#include "pthread.h"
#include "unistd.h"
#include "Log.h"

void* threadFunc(void *ptr)
{
    HandlerThread* pHT = (HandlerThread*)ptr;
    pHT->run();
    return nullptr;
}

HandlerThread::HandlerThread()
{
    mLooper = nullptr;
    mPid = 0;
    mStarted = false;
    LOG_I("HandlerThread#%x\n", this);
}
HandlerThread::~HandlerThread()
{
    LOG_I("~HandlerThread#%x\n", this);
    mLooper = nullptr;
}

void HandlerThread::run()
{
    LOG_I("HandlerThread%x in Thread#%x\n", this, pthread_self());
    // mPid = pthread_self();
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
