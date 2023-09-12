#include "Handler.h"
#include "Message.h"
#include "Looper.h"
#include "MessageQueue.h"
#include "Timer.h"

Handler::Handler(Looper* looper, Callback callback)
{
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

Message* Handler::obtainMessage()
{
    return Message::obtain(this);
}
Message* Handler::obtainMessage(int what)
{
    return Message::obtain(this, what);
}
Message* Handler::obtainMessage(int what, void* obj, Message::ObjDeletor deletor)
{
    return Message::obtain(this, what, obj, deletor);
}
Message* Handler::obtainMessage(int what, int arg1, int arg2)
{
    return Message::obtain(this, what, arg1, arg2);
}
Message* Handler::obtainMessage(int what, int arg1, int arg2, void* obj, Message::ObjDeletor deletor)
{
    return Message::obtain(this, what, arg1, arg2, obj, deletor);
}

bool Handler::sendMessage(Message* msg)
{
    return sendMessageDelayed(msg, 0);
}
bool Handler::sendMessageDelayed(Message* msg, long delay)
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
bool Handler::sendMessageAtFrontOfQueue(Message* msg)
{
    MessageQueue* queue = mQueue;
    if (queue == nullptr) 
    {
        return false;
    }
    return enqueueMessage(queue, msg, 0);
}
bool Handler::executeOrSendMessage(Message* msg)
{
    if (mLooper == Looper::myLooper())
    {
        dispatchMessage(msg);
        return true;
    }
    return sendMessage(msg);
}

bool Handler::sendEmptyMessage(int what)
{
    return sendEmptyMessageDelayed(what, 0);
}
bool Handler::sendEmptyMessageDelayed(int what, long delay)
{
    Message* msg = Message::obtain();
    msg->what = what;
    return sendMessageDelayed(msg, delay);
}
bool Handler::sendEmptyMessageAtTime(int what, long uptimeMillis)
{
    Message* msg = Message::obtain();
    msg->what = what;
    return sendMessageAtTime(msg, uptimeMillis);
}

bool Handler::enqueueMessage(MessageQueue* queue, Message* msg, long uptimeMillis)
{
    msg->target = this;
    return queue->enqueueMessage(msg, uptimeMillis);
}

void Handler::removeMessages(int what)
{
    mQueue->removeMessages(this, what, nullptr);
}
void Handler::removeMessages(int what, void* obj)
{
    mQueue->removeMessages(this, what, obj);
}

bool Handler::hasMessages(int what)
{
    return mQueue->hasMessages(this, what, nullptr);
}
bool Handler::hasMessages(int what, void* obj)
{
    return mQueue->hasMessages(this, what, obj);
}

Looper* Handler::getLooper()
{
    return mLooper;
}

