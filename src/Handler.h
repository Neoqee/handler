#ifndef __NEOQEE_HANDLER_H__
#define __NEOQEE_HANDLER_H__

#include <functional>
#include "Message.h"

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

    Message* obtainMessage();
    Message* obtainMessage(int what);
    Message* obtainMessage(int what, void* obj, Message::ObjDeletor deletor);
    Message* obtainMessage(int what, int arg1, int arg2);
    Message* obtainMessage(int what, int arg1, int arg2, void* obj, Message::ObjDeletor deletor);

    bool sendMessage(Message* msg);
    bool sendMessageDelayed(Message* msg, long delay);
    bool sendMessageAtTime(Message* msg, long uptimeMillis);
    bool sendMessageAtFrontOfQueue(Message* msg);
    bool executeOrSendMessage(Message* msg);

    bool sendEmptyMessage(int what);
    bool sendEmptyMessageDelayed(int what, long delay);
    bool sendEmptyMessageAtTime(int what, long uptimeMillis);

    void removeMessages(int what);
    void removeMessages(int what, void* obj);
    bool hasMessages(int what);
    bool hasMessages(int what, void* obj);

    Looper* getLooper();

private:
    bool enqueueMessage(MessageQueue* queue, Message* msg, long uptimeMillis);

private:
    Looper* mLooper;
    MessageQueue* mQueue;
    Callback mCallback;
};

#endif