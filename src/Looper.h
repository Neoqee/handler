#ifndef __NEOQEE_LOOPER_H__
#define __NEOQEE_LOOPER_H__

#include "MessageQueue.h"

class Looper
{
public:
    inline MessageQueue* getQueue()
    {
        return mQueue;
    }

    static void prepare();
    static void loop();
    static Looper* myLooper();

    static void initTLSKey();
    static void threadDestructor(void *st);


private:
    Looper(bool quitAllowed);

private:
    MessageQueue* mQueue;

};

#endif