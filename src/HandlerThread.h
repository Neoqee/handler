
class Looper;
class Handler;

class HandlerThread
{
public:
    HandlerThread();
    virtual ~HandlerThread();
    void start();
    int getThreadId();

    Looper* getLooper();
    // Handler* getThreadHandler();
    virtual void run();
    

private:
    Looper* mLooper;
    // Handler* mHandler;
    pthread_t mPid;
    bool mStarted;

};
