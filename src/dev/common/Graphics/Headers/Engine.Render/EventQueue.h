#pragma once
#include "WidgetDeclarations.h"
#include <Engine.Base/String/Util.h>

#define type_event(ev) \
inline constexpr rw::EventID event_ ## ev = (EventID)(hashQuickLowerCaseConstExpr(#ev "Event")); \
struct ev ## Event

namespace rw
{
using EventID = int;
inline constexpr EventID EventID_Invalid = -1;

using Realm = int;
inline constexpr EventID Realm_Invalid = -1;
inline constexpr EventID Realm_Client = 0;
inline constexpr EventID Realm_Server = 1;
inline constexpr EventID Realm_ClientAndServer = 2;

struct EventData
{
    Realm realm;
    EventID event;
};

using EventHandler = FastDelegate<int(const EventData*)>;
using EventStack = eastl::vector<const EventData*>;

class EventQueue
{
public:
    FB_RENDER_API EventQueue();
    FB_RENDER_API ~EventQueue();

    FB_RENDER_API int enqueueEvent(const EventData *e);
    FB_RENDER_API const EventData *pollEvent();
    FB_RENDER_API bool hasPendingEvents();

private:
    EventStack m_Stack;
};

using EventMap = eastl::hash_map<EventID, eastl::vector<EventHandler>>;

class EventBus
{
public:
    FB_RENDER_API EventBus();
    FB_RENDER_API ~EventBus();

    FB_RENDER_API int registerHandler(EventID ev, EventHandler h);
    FB_RENDER_API void unregisterHandler(EventID ev, int index);
    FB_RENDER_API void publish(const EventData *e);

private:
    EventMap m_Map;
};
}