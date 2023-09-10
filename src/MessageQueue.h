#ifndef __NEOQEE_MESSAGEQUEUE_H__
#define __NEOQEE_MESSAGEQUEUE_H__

#include "Message.h"
#include "sys/types.h"
#include <mutex>
#include "Handler.h"

class MessageQueue
{
public:
    MessageQueue(bool quitAllowed);
    ~MessageQueue();

    bool enqueueMessage(Message* msg, long when);
    bool hasMessages(Handler* h, int what, void* object);
    void removeMessages(Handler* h, int what, void* object);

    Message* next();

private:
    const bool mQuitAllowed;
    long mPtr;
    Message* mMessages;
    bool mQuitting;
    bool mBlocked;
    std::mutex mClassLock;

    static long nativeInit();
    static void nativeDestory(long ptr);
    void nativePollOnce(long ptr, int timeoutMillis);
    static void nativeWake(long ptr);
    static bool nativeIsPolling(long ptr);

    void dispose();
    bool isPolling();

    void quit(bool safe);

    
    
};

#endif
