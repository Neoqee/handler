- 使用

```
Handler* mHandler = nullptr;
HandlerThread* mHandlerThread = nullptr;

bool handleMessage(Message* msg)
{
    printf("receive msg at %ld\n", SystemClock::uptimeMillis());
    std::cout << "thread_id = " << std::this_thread::get_id() << std::endl;
    Looper::myLooper();
    return true;
}

int main(int argc, char const *argv[])
{
    // 创建一个新的Handler线程
    mHandlerThread = new HandlerThread();
    // 运行线程，创建looper
    mHandlerThread->start();
    // 获取线程内的looper，然后传入处理回调函数
    mHandler = new Handler(mHandlerThread->getLooper(), handleMessage);
    
    auto startTime = SystemClock::uptimeMillis();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    auto duration = SystemClock::uptimeMillis() - startTime;
    Baaa* obj = new Baaa(); // 自定义的数据
    // 使用自定义数据时，转成 void* 之后，在处理完之后无法正确释放，所以为了能正常释放数据，
    // 定义了一个释放函数；不然的话，就得定义个基类来处理，然后所有传递的数据都继承这个基类
    Message* msg1 = Message::obtain(mHandler, 1, (void*)obj, [](void* msgObj){
        Baaa* a = (Baaa*)msgObj;
        delete a;
    });
    // 推送待处理的Message，使用对应的delay方法，可以在延迟delay后在handleessage回调中收到消息，然后进行处理
    mHandler->sendMessage(msg1);

    for(;;) { 
        std::this_thread::sleep_for(std::chrono::milliseconds(10000)); 
    }

    return 0;
}
```

## 加锁情况

1. Handler 无锁

2. Message

- obtain() 函数
- recycleUnchecked() 函数

3. MessageQueue

- isPolling()
- next()
- quit()
- enqueueMessages(Message* msg, long when)
- hasMessages(Handler* h, int what, void* object)
- removeMessages(Handler* h, int what, void* object)

## 异常情况

暂时没有对异常进行处理