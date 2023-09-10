#include "Message.h"
#include "Handler.h"

std::mutex sPoolSync;
Message* Message::sPool = nullptr;
int Message::sPoolSize = 0;
bool Message::gCheckRecycle = false;

Message::Message()
{

}
Message::~Message()
{

}

Message* Message::obtain()
{
    printf("obtain()\n");
    std::lock_guard<std::mutex> lock(sPoolSync);
    if (sPool != nullptr)
    {
        Message* m = sPool;
        sPool = m->next;
        m->next = nullptr;
        m->flags = 0;
        sPoolSize--;
        return m;
    }
    return new Message();
}
Message* Message::obtain(Message* orig)
{
    Message* m = obtain();
    m->what = orig->what;
    m->arg1 = orig->arg1;
    m->arg2 = orig->arg2;
    m->obj = orig->obj;
    m->what = orig->what;
    m->target = orig->target;

    return m;
}
Message* Message::obtain(Handler* h)
{
    printf("obtain(Handler*)\n");
    Message* m = obtain();
    m->target = h;
    return m;
}
Message* Message::obtain(Handler* h, int what)
{
    Message* m = obtain();
    m->target = h;
    m->what = what;
    return m;
}
Message* Message::obtain(Handler* h, int what, void* obj)
{
    Message* m = obtain();
    m->target = h;
    m->what = what;
    m->obj = obj;
    return m;
}

void Message::sendToTarget()
{
    target->sendMessage(this);
}
void Message::markInUse()
{
    flags |= FLAG_IN_USE;
}
bool Message::isInUse()
{
    return ((flags & FLAG_IN_USE) == FLAG_IN_USE);
}
bool Message::recycleUnchecked()
{
    flags = FLAG_IN_USE;
    what = 0;
    arg1 = 0;
    arg2 = 0;
    if (obj)
    {
        delete obj;
    }
    obj = nullptr;
    when = 0;
    target = nullptr;

    {
        std::lock_guard<std::mutex> lock(sPoolSync);
        if (sPoolSize < MAX_POOL_SIZE)
        {
            next = sPool;
            sPool = this;
            sPoolSize++;
            return true;
        }
        else 
        {
            return false;
        }
    }
}