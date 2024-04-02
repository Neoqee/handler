#ifndef __NEOQEE_MESSAGE_H__
#define __NEOQEE_MESSAGE_H__

// #include "Handler.h"
#include <mutex>
#include <functional>

class Handler;

class Message
{
public:
    typedef std::function<void(void*)> ObjDeletor;
    int what;
    int arg1;
    int arg2;
    void* obj;
    ObjDeletor deletor;

    static const int FLAG_IN_USE = 1 << 0;
    static const int FLAGS_TO_CLEAR_ON_COPY_FROM = FLAG_IN_USE;
    int flags;
    Handler* target;
    long when;
    
    Message* next;
private:
    // static std::mutex sPoolSync;
    static Message* sPool;
    static int sPoolSize;
    static const int MAX_POOL_SIZE = 0;
    static bool gCheckRecycle;

    

public:
    Message();
    ~Message();

    static Message* obtain();
    static Message* obtain(Message* orig);
    static Message* obtain(Handler* h);
    static Message* obtain(Handler* h, int what);
    static Message* obtain(Handler* h, int what, int arg1, int arg2);
    static Message* obtain(Handler* h, int what, int arg1, int arg2, void* obj, ObjDeletor deletor);
    static Message* obtain(Handler* h, int what, void* obj, ObjDeletor deletor);

    void sendToTarget();
    void markInUse();
    bool isInUse();
    bool recycleUnchecked();
    static void recycle(Message* m);


};

#endif