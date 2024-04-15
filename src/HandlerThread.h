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
    
    bool quit();
    bool quitSafely();

private:
    Looper* mLooper;
    // Handler* mHandler;
    unsigned long mPid;
    bool mStarted;

};
