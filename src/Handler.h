#ifndef __NEOQEE_HANDLER_H__
#define __NEOQEE_HANDLER_H__

// #include "MessageQueue.h"
#include <functional>
#include "Message.h"
// #include "Looper.h"

class Looper;
class MessageQueue;

class Handler
{
public:
    typedef std::function<bool (Message*)> Callback;
public:
    // Handler();
    Handler(Looper* looper, Callback callback);

    virtual void handleMessage(Message* msg);
    void dispatchMessage(Message* msg);

    bool sendMessage(Message* msg);
    bool sendMessageDelay(Message* msg, long delay);
    bool sendMessageAtTime(Message* msg, long uptimeMillis);
    // bool sendEmptyMessage(int what);
    // bool sendEmptyMessageDelay(int what, long delay);

    void removeMessages(int what);
    bool hasMessages(int what);

    Looper* getLooper();

private:
    bool enqueueMessage(MessageQueue* queue, Message* msg, long uptimeMillis);

private:
    Looper* mLooper;
    MessageQueue* mQueue;
    Callback mCallback;
};

#endif