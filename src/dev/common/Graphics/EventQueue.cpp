#include <Engine.Render/EventQueue.h>

namespace rw
{
EventQueue::EventQueue()
    : m_Stack()
{}

EventQueue::~EventQueue()
{

}

int EventQueue::enqueueEvent(const EventData *e)
{
    if(!e)
        return NO;

    m_Stack.push_back(e);
    return YES;
}

const EventData *EventQueue::pollEvent()
{
    if(m_Stack.size())
    {
        const EventData *e = m_Stack.pop_front();
        return e;
    }

    return NO;
}

bool EventQueue::hasPendingEvents()
{
    return !!(m_Stack.size());
}

EventBus::EventBus()
    : m_Map()
{}

EventBus::~EventBus()
{

}

int EventBus::registerHandler(EventID ev, EventHandler h)
{
    auto &kvp = m_Map.insert(ev);
    int index = kvp.second.size();
    kvp.second.push_back(h);
    return index;
}

void EventBus::unregisterHandler(EventID ev, int index)
{
    auto kvp = m_Map.find(ev);
    if(kvp != m_Map.end())
    {
        kvp->second.erase(kvp->second.begin() + index);
    }
}

void EventBus::publish(const EventData *e)
{
    if(e)
    {
        auto kvp = m_Map.find(e->event);
        for(EventHandler *i = kvp->second.begin(), *end = kvp->second.end(); i < end; ++i)
        {
            int ret = (*i)(e);
        }
    }
}
}