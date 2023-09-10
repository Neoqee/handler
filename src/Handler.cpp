#include "Handler.h"
#include "Message.h"
#include "Looper.h"
#include "MessageQueue.h"
#include "Timer.h"

Handler::Handler(Looper* looper, Callback callback)
{
    printf("Handler()\n");
    mLooper = looper;
    mQueue = looper->getQueue();
    mCallback = callback;
    Message* msg;
}

void Handler::dispatchMessage(Message* msg)
{
    if (mCallback != nullptr)
    {
        bool res = mCallback(msg);
        if (res)
        {
            return;
        }
    }
    handleMessage(msg);
}

void Handler::handleMessage(Message* msg)
{

}

bool Handler::sendMessage(Message* msg)
{
    return sendMessageDelay(msg, 0);
}
bool Handler::sendMessageDelay(Message* msg, long delay)
{
    if (delay < 0)
    {
        delay = 0;
    }
    return sendMessageAtTime(msg, SystemClock::uptimeMillis() + delay);
}
bool Handler::sendMessageAtTime(Message* msg, long uptimeMillis)
{
    MessageQueue* queue = mQueue;
    if (queue == nullptr) return false;
    return enqueueMessage(queue, msg, uptimeMillis);
}
// bool Handler::sendEmptyMessage(int what);
// bool Handler::sendEmptyMessageDelay(int what, long delay);

bool Handler::enqueueMessage(MessageQueue* queue, Message* msg, long uptimeMillis)
{
    printf("Handler::enqueueMessage()\n");
    msg->target = this;
    return queue->enqueueMessage(msg, uptimeMillis);
}

void Handler::removeMessages(int what)
{
    mQueue->removeMessages(this, what, nullptr);
}
bool Handler::hasMessages(int what)
{
    return mQueue->hasMessages(this, what, nullptr);
}

Looper* Handler::getLooper()
{
    return mLooper;
}

