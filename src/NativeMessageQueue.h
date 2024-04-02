#ifndef __NEOQEE_NATIVEMESSAGEQUEUE_H__
#define __NEOQEE_NATIVEMESSAGEQUEUE_H__

#include "NativeLooper.h"

class NativeMessageQueue
{
public:
    inline NativeLooper* getLooper() const {
        return mLooper;
    }
    NativeMessageQueue();
    virtual ~NativeMessageQueue();

    void pollOnce(int timeoutMillis);
    void wake();
protected:
    NativeLooper* mLooper;
};


#endif