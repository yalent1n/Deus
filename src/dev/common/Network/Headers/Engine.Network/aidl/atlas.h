#pragma once
#include "schedule.h"
#include "sync.h"
#include "handoff.h"
#include "expansion.h"
#include <Engine.Base/HashMap.h>

namespace rw
{
    struct aidl_core;
    
    typedef void(*Aidl_Payload_Dispatcher)(const void*, u16);

    struct Aidl
    {
        Aidl_Sync_State sync;
        Aidl_Handoff_State handoff;
        Aidl_Expansion_State expansion;
        Aidl_Payload_Dispatcher payload_dispatcher;

        aidl_core*          data;

        ether_addr          hostaddr;
        ether_addr          dst;
    };

    // Initializes the state and opens as a server.
    // local_port: Where to open the current state
    // Returns an error code.
    AIDL_API int AidlOpen(Aidl* state, const u16 local_port, const u32 mask = 0);
    
    // Initializes the state and joins to the peer host.
    // host: Peer host in netname:port
    // local_port: Where to open the current state
    // Returns an error code.
    AIDL_API int AidlJoin(Aidl* state, const char* host, const u16 local_port, const u32 mask = 0);

    // Sends a message to the peer host.
    // state: The state
    // bytes: Bytes to send
    // length: Length of bytes
    // Returns the number of bytes sent on success, otherwise an error code.
    AIDL_API int AidlWrite(Aidl* state, const void* bytes, const int length);

    // Sends a frame TX to the peer host.
    // state: The state
    // frame: The frame containing the tags
    AIDL_API int AidlSendFrame(Aidl* state, const aidl_tags_t& tags);

    // Receives a message from the peer host. state's addr will receive the source address.
    // state: The state
    // target: Buffer receiving the bytes
    // length: Length of buffer
    AIDL_API int AidlDispatchFrame(Aidl* state, aidl_if* itf);

    // Returns the relay agent socket associated with the state.
    AIDL_API int _AidlGetSocket(Aidl* state, int* socket);

    // Frees any assigned resources.
    // state: The state
    AIDL_API int AidlClose(Aidl* state);
}