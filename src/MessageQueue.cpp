#include "MessageQueue.h"
#include "Timer.h"
#include "math.h"
#include "NativeMessageQueue.h"
#include "Log.h"

long MessageQueue::nativeInit()
{
    NativeMessageQueue* nativeMessageQueue = new NativeMessageQueue();
    if (!nativeMessageQueue)
    {
        return 0;
    }
    return reinterpret_cast<long>(nativeMessageQueue);
}
void MessageQueue::nativeDestory(long ptr)
{
    NativeMessageQueue* nativeMessageQueue = reinterpret_cast<NativeMessageQueue*>(ptr);
    delete nativeMessageQueue;
}
void MessageQueue::nativePollOnce(long ptr, int timeoutMillis)
{
    NativeMessageQueue* nativeMessageQueue = reinterpret_cast<NativeMessageQueue*>(ptr);
    nativeMessageQueue->pollOnce(timeoutMillis);
}
void MessageQueue::nativeWake(long ptr)
{
    NativeMessageQueue* nativeMessageQueue = reinterpret_cast<NativeMessageQueue*>(ptr);
    nativeMessageQueue->wake();
}
bool MessageQueue::nativeIsPolling(long ptr)
{
    NativeMessageQueue* nativeMessageQueue = reinterpret_cast<NativeMessageQueue*>(ptr);
    return nativeMessageQueue->getLooper()->isPolling();
}


MessageQueue::MessageQueue(bool quitAllowed): mQuitAllowed(quitAllowed)
{
    mPtr = nativeInit();
    mMessages = nullptr;
    mQuitting = false;
    mBlocked = false;
    LOG_I("MessageQueue#%x, mPtr#%x\n", this, mPtr);
}

MessageQueue::~MessageQueue()
{
    LOG_I("~MessageQueue#%x, mPtr#%x\n", this, mPtr);
    dispose();
}

void MessageQueue::dispose()
{
    try
    {
        if (mPtr != 0)
        {
            nativeDestory(mPtr);
            mPtr = 0;
        }
    }
    catch(...)
    {
    }
}

bool MessageQueue::isPolling()
{
    std::lock_guard<std::mutex> lock(mClassLock);
    return !mQuitting && nativeIsPolling(mPtr);
}

Message* MessageQueue::next()
{
    const long ptr = mPtr;
    if (ptr == 0)
    {
        return nullptr;
    }
    int nextPollTimeoutMillis = 0;
    for (;;)
    {
        nativePollOnce(ptr, nextPollTimeoutMillis);
        {
            std::lock_guard<std::mutex> lock(mClassLock);
            const long now = SystemClock::uptimeMillis();
            Message* msg = mMessages;
            if (msg != nullptr)
            {
                if (now < msg->when)
                {
                    nextPollTimeoutMillis = (int) std::min(msg->when - now, INT64_MAX);
                }
                else 
                {
                    mBlocked = false;
                    mMessages = msg->next;
                    msg->next = nullptr;
                    msg->markInUse();
                    return msg;
                }
            }
            else 
            {
                nextPollTimeoutMillis = -1;
            }

            if (mQuitting)
            {
                dispose();
                return nullptr;
            }

            mBlocked = true;
        }
    }
}

void MessageQueue::quit(bool safe)
{
    if (!mQuitAllowed) return;

    std::lock_guard<std::mutex> lock(mClassLock);
    if (mQuitting) return;

    mQuitting = true;

    if (safe)
    {
        removeAllFutureMessagesLocked();
    }
    else 
    {
        removeAllMessageLocked();
    }

    nativeWake(mPtr);
}

void MessageQueue::removeAllMessageLocked()
{
    Message* p = mMessages;
    while (p != nullptr)
    {
        Message* n = p->next;
        Message::recycle(p);
        p = n;
    }
    mMessages = nullptr;
}   

void MessageQueue::removeAllFutureMessagesLocked()
{
    long now = SystemClock::uptimeMillis();
    Message* p = mMessages;
    if (p != nullptr) 
    {
        if (p->when> now)
        {
            removeAllMessageLocked();
        }
        else 
        {
            Message* n;
            for (;;)
            {
                n = p->next;
                if (n == nullptr)
                {
                    return;
                }
                if (n->when > now)
                {
                    break;
                }
                p = n;
            }
            p->next = nullptr;
            do
            {
                p = n;
                n = p->next;
                Message::recycle(p);
            } while(n != nullptr);
        }
    }
}

bool MessageQueue::enqueueMessage(Message* msg, long when)
{
    if (msg->target == nullptr) return false;
    if (msg->isInUse()) return false;
    {
        std::lock_guard<std::mutex> lock(mClassLock);
        if (mQuitting)
        {
            Message::recycle(msg);
            return false;
        }

        msg->markInUse();
        msg->when = when;
        Message* p = mMessages;
        bool needWake;
        if (p == nullptr || when == 0 || when < p->when)
        {
            msg->next = p;
            mMessages = msg;
            needWake = mBlocked;
        }
        else 
        {
            needWake = mBlocked && p->target == nullptr;
            Message* prev;
            for (;;)
            {
                prev = p;
                p = p->next;
                if (p == nullptr || when < p->when)
                {
                    break;
                }
                if (needWake)
                {
                    needWake = false;
                }
            }
            msg->next = p;
            prev->next = msg;
        }

        if (needWake)
        {
            nativeWake(mPtr);
        }
        return true;
    }
}

bool MessageQueue::hasMessages(Handler* h, int what, void* object)
{
    if (h == nullptr) return false;

    std::lock_guard<std::mutex> lock(mClassLock);
    Message* p = mMessages;
    while (p != nullptr)
    {
        if (p->target == h && p->what == what && (object == nullptr || p->obj == object))
        {
            return true;
        }
        p = p->next;
    }
    return false;
}

void MessageQueue::removeMessages(Handler* h, int what, void* object)
{
    if (h == nullptr) return;

    std::lock_guard<std::mutex> lock(mClassLock);
    Message* p = mMessages;
    
    while (p != nullptr && p->target == h && p->what == what
            && (object == nullptr || p->obj == object))
    {
        Message* n = p->next;
        mMessages = n;
        Message::recycle(p);
        p = n;
    }

    while (p != nullptr)
    {
        Message* n = p->next;
        if (n != nullptr)
        {
            if (n->target == h && n->what == what
                && (object == nullptr || n->obj == object))
            {
                Message* nn = n->next;
                Message::recycle(n);
                p->next = nn;
                continue;
            }
        }
        p = n;
    }
}
    
