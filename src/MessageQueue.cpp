#include "MessageQueue.h"
#include "Timer.h"
#include "math.h"
#include "NativeMessageQueue.h"

long MessageQueue::nativeInit()
{
    printf("nativeInit()\n");
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
    printf("nativePollOnce -> %d\n", timeoutMillis);
    NativeMessageQueue* nativeMessageQueue = reinterpret_cast<NativeMessageQueue*>(ptr);
    nativeMessageQueue->pollOnce(timeoutMillis);
}
void MessageQueue::nativeWake(long ptr)
{
    printf("nativeWake()\n");
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
    printf("MessageQueue()\n");
    mPtr = nativeInit();
}

MessageQueue::~MessageQueue()
{
    
}

void MessageQueue::dispose()
{
    try
    {
        nativeDestory(mPtr);
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
    printf("next\n");
    const long ptr = mPtr;
    int nextPollTimeoutMillis = 0;
    int count = 5;
    for (;;)
    {
        count--;
        if (count == 0)
        {
            return nullptr;
        }
        nativePollOnce(ptr, nextPollTimeoutMillis);
        printf("nativePollOnce finish\n");
        {
            std::lock_guard<std::mutex> lock(mClassLock);
            const long now = SystemClock::uptimeMillis();
            Message* prevMsg = nullptr;
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
                    if (prevMsg != nullptr)
                    {
                        prevMsg->next = msg->next;
                    }
                    else 
                    {
                        mMessages = msg->next;
                    }
                    msg->next = nullptr;
                    msg->markInUse();
                    printf("1 - mBlock = %d\n", mBlocked);
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
            printf("2 - mBlock = %d\n", mBlocked);


        }

        // nextPollTimeoutMillis = 0;

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

    }
    else 
    {

    }

    nativeWake(mPtr);
}

bool MessageQueue::enqueueMessage(Message* msg, long when)
{
    printf("MessageQueue::enqueueMessage()\n");
    if (msg->target == nullptr) return false;
    if (msg->isInUse()) return false;
    printf("...\n");
    {
        std::lock_guard<std::mutex> lock(mClassLock);
        if (mQuitting)
        {
            // msg.recycle();
            return false;
        }

        msg->markInUse();
        msg->when = when;
        Message* p = mMessages;
        bool needWake;
        if (p == nullptr || when == 0 || when < p->when)
        {
            printf("p == nullptr || when == 0 || when < p->when\n");
            printf("insert first\n");
            msg->next = p;
            mMessages = msg;
            needWake = mBlocked;
            printf("needWake = %d\n", needWake);
        }
        else 
        {
            printf("push back last\n");
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
            printf("needWake = %d\n", needWake);
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
        // p->recycleUnchecked();
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
                // n->recycleUnchecked();
                p->next = nn;
                continue;
            }
        }
        p = n;
    }
}
    
