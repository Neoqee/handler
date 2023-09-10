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
    void setFileDescriptorEvents(int fd, int events);

    virtual int handleEvent(int fd, int events, void* data);
protected:
    NativeLooper* mLooper;
};


#endif