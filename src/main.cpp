#include "Handler.h"
#include <thread>
#include "Looper.h"
#include "Timer.h"
#include "Message.h"
#include <stdio.h>
#include "pthread.h"
#include <iostream>
#include "HandlerThread.h"

Handler* mHandler = nullptr;
HandlerThread* mHandlerThread = nullptr;

bool handleMessage(Message* msg)
{
    printf("receive msg at %ld\n", SystemClock::uptimeMillis());
    std::cout << "thread_id = " << std::this_thread::get_id() << std::endl;
    Looper::myLooper();
    return true;
}

void* init(void *)
{
    Looper::prepare();

    mHandler = new Handler(Looper::myLooper(), handleMessage);
    Message* msg1 = Message::obtain(mHandler, 1);
    mHandler->sendMessageDelayed(msg1, 5000);
    Message* msg2 = Message::obtain(mHandler, 1);
    mHandler->sendMessageDelayed(msg2, 10000);
    Message* msg3 = Message::obtain(mHandler, 1);
    mHandler->sendMessageDelayed(msg3, 15000);

    Looper::loop();
    return nullptr;
}

class Aaaa
{
public:
    Aaaa()
    {
        printf("Aaaa\n");
    }
    virtual ~Aaaa() {
        printf("~Aaaa\n");
    }

private:
    int a;
};

class Baaa: public Aaaa
{
public:
    Baaa(): Aaaa()
    {
        printf("Baaa\n");
    }
    ~Baaa()
    {
        printf("~Baaa\n");
    }
};

void testDelete()
{
    Baaa* pB1 = new Baaa();
    delete pB1;
    Aaaa* pB2 = new Baaa();
    delete pB2;
}

int main(int argc, char const *argv[])
{
    // std::thread newThread(init, nullptr);
    // newThread.detach();
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000)); 
    // pthread_t pid;
    // pthread_create(&pid, nullptr, init, nullptr);
    // pthread_detach(pid);

    
    mHandlerThread = new HandlerThread();
    mHandlerThread->start();
    mHandler = new Handler(mHandlerThread->getLooper(), handleMessage);
    
    auto startTime = SystemClock::uptimeMillis();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    auto duration = SystemClock::uptimeMillis() - startTime;
    Baaa* obj = new Baaa();
    Message* msg1 = Message::obtain(mHandler, 1, (void*)obj, [](void* msgObj){
        Baaa* a = (Baaa*)msgObj;
        delete a;
    });
    mHandler->sendMessage(msg1);

    // testDelete();

    for(;;) { 
        std::this_thread::sleep_for(std::chrono::milliseconds(10000)); 
        // Message* msg1 = Message::obtain(mHandler, 1);
        // mHandler->sendMessageDelay(msg1, 5000);
    }

    return 0;
}
