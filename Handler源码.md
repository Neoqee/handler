- 文件相关

frameworks/base/core/java/android/os
    Looper.java
    Message.java
    Handler.java
    MessageQueue.java

frameworks/base/core/jni
    android_os_MessageQueue.cpp
    android_os_MessageQueue.h

system/core/libutils/include/utils
    Looper.h
    Timers.h

system/core/libutils
    Looper.cpp


整理一下Handler的调用链

handler->sendMessage(msg);
-> sendMessageDelayed(msg, 0);
-> sendMessageAtTime(msg, now + delay);
```
MessageQueue queue = mQueue;    // 这个队列是在looper中创建的
if (queue == null) {
    RuntimeException e = new RuntimeException(
            this + " sendMessageAtTime() called with no mQueue");
    Log.w("Looper", e.getMessage(), e);
    return false;
}
return enqueueMessage(queue, msg, uptimeMillis);
```

```
// 表明后续处理的handler，不过从handler发起的请求，最终也是由这个handler的回调进行处理；
msg.target = this;
msg.workSourceUid = ThreadLocalWorkSource.getUid();

if (mAsynchronous) {    // 表示是否是异步消息
    msg.setAsynchronous(true);
}
return queue.enqueueMessage(msg, uptimeMillis);
```

由此进入MessageQueue队列
queue.enqueueMessage(msg, uptimeMillis);
```
// 先判断msg是否有效，然后抛出异常
if (msg.target == null)
if (msg.isInUse())

// 加锁
synchronized (this){
    if (mQuitting) {    // 准备退出的队列，回收消息
        msg.recycle(); 
        return false;
    } 

    msg.markInUse();    // 标记为正在使用
    ...
    Message p = mMessages;  // Message链表
    boolean needWake;       // 是否需要唤醒的标记

    // 插入表头 
    if (p == null || when == 0 || when < p.when) {
        msg.next = p;
        mMessages = msg;
        needWake = mBlocked;    // 这个字段是在next()函数中赋值的，表示锁定正在等待pollOnce的延时
    } else {
        // 有点疑问，正常应该是不会有target == null的消息的
        needWake = mBlocked && p.target == null && msg.isAsynchronous();

        // 将消息插入到第一个时间点比when大的位置
        // 消息是以时间顺序排序的
        Message prev;
        for (;;) {
            prev = p;
            p = p.next;
            if (p == null || when < p.when) {
                break;
            }
            if (needWake && p.isAsynchronous()) {
                needWake = false;   // 当消息插入到异步消息后面时，置为false
            }
            msg.next = p; // invariant: p == prev.next
            prev.next = msg;
        }

        if (needWake) {
            nativeWake(mPtr);   // 如果需要唤醒，那么从此处进入native层进行唤醒
        }
    }
    return true;
}
```

nativeWake(mPtr);
```
frameworks/base/core/jni
    android_os_MessageQueue.cpp

// jni方法
static void android_os_MessageQueue_nativeWake(JNIEnv* env, jclass clazz, jlong ptr) {
    NativeMessageQueue* nativeMessageQueue = reinterpret_cast<NativeMessageQueue*>(ptr);
    nativeMessageQueue->wake();
}

// cpp层的Messageueue 
// native层也有一套相应的消息机制
// 主要是NativeMessageQueue 和 Looper
// NativeMessageQueue主要充当中间人，连接java和cpp层的传递
void NativeMessageQueue::wake() {
    mLooper->wake();    
}
```

mLooper->wake();  
```
system/core/libutils
    Looper.cpp

// mWakeEventFd是一个event句柄，这里是往里写个数据，epoll的唤醒机制；
uint64_t inc = 1;
ssize_t nWrite = TEMP_FAILURE_RETRY(write(mWakeEventFd.get(), &inc, sizeof(uint64_t)));
if (nWrite != sizeof(uint64_t)) {
    if (errno != EAGAIN) {
        LOG_ALWAYS_FATAL("Could not write wake signal to fd %d (returned %zd): %s",
                            mWakeEventFd.get(), nWrite, strerror(errno));
    }
}

后续会在pollnner内唤醒
int Looper::pollInner(int timeoutMillis){
    // looper执行loop后，NessageQueue的next()方法获取待执行的msg时，每次查询之前都会尝试poll，最终执行到这里，并进行等待
    // 正常是会等待timeoutMillis ms，然后让上层因此被卡住的地方继续执行下去，处理相应的msg，并查询下一次的消息在什么时候执行，重复；
    // 但是如果主动调用了上面的wake方法，也就是往epoll监听的fd中写入数据，触发epoll的消息机制
    struct epoll_event eventItems[EPOLL_MAX_EVENTS];
    int eventCount = epoll_wait(mEpollFd.get(), eventItems, EPOLL_MAX_EVENTS, timeoutMillis);
}
```

至此，从handler发起一次请求的全过程就完成了；

Message next(); 
```
int nextPollTimeoutMillis = 0;  // 下一次要执行的时间点
for (;;) {
    nativePollOnce(ptr, nextPollTimeoutMillis); // 上面说的，根据epoll机制卡住的点

    synchronized (this) {
        final long now = SystemClock.uptimeMillis();    

        Message prevMsg = null;
        Message msg = mMessages;
        if (msg != null && msg.target == null) {    // 这里还有一个消息屏障之类的机制，就是target == null的消息
            // Stalled by a barrier.  Find the next asynchronous message in the queue.
            // 寻找下一个非异步的消息
            do {
                prevMsg = msg;
                msg = msg.next;
            } while (msg != null && !msg.isAsynchronous());
        }

        if (msg != null) {
            // 找到一个消息
            if (now < msg.when) {
                // Next message is not ready.  Set a timeout to wake up when it is ready.
                // 还没到预定的执行时间点，设置下一次唤醒需要的等待时间
                nextPollTimeoutMillis = (int) Math.min(msg.when - now, Integer.MAX_VALUE);
            } else {
                // 表示该消息是需要立刻执行的， 修改一下链表
                // Got a message.
                mBlocked = false;   // 因为要退出循环了，然后loop循环中执行完后，会重新进入，先清除锁定标记
                if (prevMsg != null) {
                    prevMsg.next = msg.next;
                } else {
                    mMessages = msg.next;
                }
                msg.next = null;
                if (DEBUG) Log.v(TAG, "Returning message: " + msg);
                msg.markInUse();
                return msg;
            }
        } else {
            // 队列中没有符合的消息，所以设置为-1，相当于设置了一个非常大的等待时间
            // No more messages.
            nextPollTimeoutMillis = -1;
        }

        ...
        // 后面还有一些Idle类的Handler消息处理，如果没有的话
        mBlock = true；
        continue;

        // 但是如果有的话，会执行idle的消息
    }
    // 然后在这里重置唤醒时间
    nextPollTimeoutMillis = 0;
}
```
