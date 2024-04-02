#include "Message.h"
#include "Handler.h"
#include "Log.h"

std::mutex sPoolSync;
Message* Message::sPool = nullptr;
int Message::sPoolSize = 0;
bool Message::gCheckRecycle = false;

Message::Message()
{
    what = 0;
    arg1 = 0;
    arg2 = 0;
    obj = nullptr;
    deletor = nullptr;
    flags = 0;
    target = nullptr;
    when = 0;
    next = nullptr;

    LOG_I("Message#%x\n", this);
}
Message::~Message()
{
    LOG_I("~Message#%x\n", this);
}

Message* Message::obtain()
{
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
    m->deletor = orig->deletor;
    m->what = orig->what;
    m->target = orig->target;

    return m;
}
Message* Message::obtain(Handler* h)
{
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
Message* Message::obtain(Handler* h, int what, void* obj, ObjDeletor deletor)
{
    Message* m = obtain();
    m->target = h;
    m->what = what;
    m->obj = obj;
    m->deletor = deletor;
    return m;
}
Message* Message::obtain(Handler* h, int what, int arg1, int arg2)
{
    Message* m = obtain();
    m->target = h;
    m->what = what;
    m->arg1 = arg1;
    m->arg2 = arg2;
    return m;
}
Message* Message::obtain(Handler* h, int what, int arg1, int arg2, void* obj, ObjDeletor deletor)
{
    Message* m = obtain();
    m->target = h;
    m->what = what;
    m->arg1 = arg1;
    m->arg2 = arg2;
    m->obj = obj;
    m->deletor = deletor;
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
    if (obj && deletor)
    {
        deletor(obj);
    }
    obj = nullptr;
    deletor = nullptr;
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

void Message::recycle(Message* m)
{
    if (m->isInUse())
    {
        if (gCheckRecycle) return;
    }
    if (!m->recycleUnchecked())
    {
        delete m;
        m = nullptr;
    }
}
