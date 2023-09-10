#include "Handler.h"
#include <thread>
#include "Looper.h"
#include "Timer.h"
#include "Message.h"
#include <stdio.h>

Handler* mHandler = nullptr;

bool handleMessage(Message* msg)
{
    printf("receive msg at %ld\n", SystemClock::uptimeMillis());
    return true;
}

void init()
{
    printf("init()\n");
    Looper::prepare();

    mHandler = new Handler(Looper::myLooper(), handleMessage);
    printf("mHandler finish()\n");
    printf("send msg at -> %ld ms\n", SystemClock::uptimeMillis());
    Message* msg1 = Message::obtain(mHandler, 1);
    mHandler->sendMessageDelay(msg1, 5000);
    Message* msg2 = Message::obtain(mHandler, 1);
    mHandler->sendMessageDelay(msg2, 10000);
    Message* msg3 = Message::obtain(mHandler, 1);
    mHandler->sendMessageDelay(msg3, 15000);

    Looper::loop();

}

int main(int argc, char const *argv[])
{
    std::thread newThread(init);
    newThread.detach();
    
    auto startTime = SystemClock::uptimeMillis();
    std::this_thread::sleep_for(std::chrono::seconds(20));
    auto duration = SystemClock::uptimeMillis() - startTime;
    printf("end sleep %ld ms -- %ld\n", duration, SystemClock::uptimeMillis());
    Message* msg1 = Message::obtain(mHandler, 1);
    mHandler->sendMessage(msg1);

    for(;;) { 
        std::this_thread::sleep_for(std::chrono::milliseconds(10000)); 
        printf("send msg at -> %ld ms\n", SystemClock::uptimeMillis());
        Message* msg1 = Message::obtain(mHandler, 1);
        mHandler->sendMessageDelay(msg1, 5000);
    }

    return 0;
}
