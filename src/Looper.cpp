#include "Looper.h"
#include "pthread.h"
#include <exception>
#include <iostream>

static pthread_once_t _gTLSOnce = PTHREAD_ONCE_INIT;
static pthread_key_t _gTLSKey = 0;
Looper* _gLooper = nullptr;

Looper::Looper(bool quitAllowed)
{
    mQueue = new MessageQueue(quitAllowed);
}

void Looper::initTLSKey()
{
    try
    {
        int error = pthread_key_create(& _gTLSKey, threadDestructor);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}

void Looper::threadDestructor(void *st) {
    printf("Looper::threadDestructor\n");
    // NativeLooper* self = static_cast<NativeLooper*>(st);
    // if (self != nullptr) {
    //     self->decStrong((void*)threadDestructor);
    // }
    // if (self != nullptr)
    // {
    //     delete self;
    // }
}

void Looper::prepare()
{
    int result = pthread_once(&_gTLSOnce, initTLSKey);
    pthread_key_t gTLSKey = _gTLSKey;
    Looper* temp = (Looper*)pthread_getspecific(gTLSKey);
    if (temp != nullptr)
    {
        return;
    }
    temp = new Looper(true);
    pthread_setspecific(gTLSKey, temp);
}

Looper* Looper::myLooper()
{
    pthread_key_t gTLSKey = _gTLSKey;
    Looper* temp = (Looper*)pthread_getspecific(gTLSKey);
    return temp;
}

void Looper::loop()
{
    Looper* me = myLooper();
    if (me == nullptr) return;

    MessageQueue* queue = me->getQueue();
    
    for (;;)
    {
        Message* msg = queue->next();
        if (msg == nullptr) return;

        try
        {
            msg->target->dispatchMessage(msg);
        }
        catch(...)
        {

        }

        if (!msg->recycleUnchecked())
        {
            delete msg;
            msg = nullptr;
        }
    }
}

