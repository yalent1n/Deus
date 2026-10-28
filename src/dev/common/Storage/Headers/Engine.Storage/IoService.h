#pragma once
#include "StoragePool.h"
#include <Engine.Base/Misc/Vars.h>
#include <Engine.Base/HashMap.h>
#include <Engine.Core/VirtualFileSystem.h>
#include <Engine.Network/aidl/aidl.h>

namespace rw
{
struct IoService;

typedef u32 IoTaskHandle;
enum
{
    InvalidIoTaskHandle = IoTaskHandle(-1)
};

inline constexpr IoTaskHandle packIoTaskHandle(u32 lane, u32 streamers, u32 flags)
{
    return (IoTaskHandle)((lane & 0x0000FFFF) | (streamers << 16) | (flags << 24));
}

inline void unpackIoTaskHandle(IoTaskHandle task, u32* lane = NO, u32* streamers = NO, u32* flags = NO)
{
    if(lane)
        *lane = (u32)(task & 0x0000FFFF);

    if(streamers)
        *streamers = (u32)((task & 0x00FF0000) >> 16);

    if(flags)
        *flags = u32((task & 0xFF000000) >> 24);
}

enum IoServiceCallbackReason
{
    IoServiceCallbackReason_Interrupted,
    IoServiceCallbackReason_LaneComplete,
    IoServiceCallbackReason_TaskComplete
};

struct IoServiceCallbackParams
{
    IoService* io;
    void* userData;
    IoTaskHandle task;
    IoServiceCallbackReason reason;
};

typedef void (*IoServiceCallback) (const IoServiceCallbackParams&);

enum /*task state*/
{
    kFlux_opened = 0x01,
    kFlux_busy = 0x02,
    kFlux_done = 0x04,
    kFlux_fault = 0x08
};

enum /*lane state*/
{
    kFlux_idle = 0x00,
    kFlux_ready = 0x01,
    kFlux_binding = 0x02,
    kFlux_streaming = 0x04,
    kFlux_completed = 0x08,
    kFlux_error = 0x80
};

enum /*streamer/task index*/
{ 
    kIo_baseTask = 0x00,
    kIo_headTask = 0x01,
    kIo_patchTask = 0x02,

    kIo_maxStreamerCount = 3 /*base, head, patch*/
};

enum /*constants*/
{
    kIo_largest = u32(-1),
    kIo_maxActiveLanes = 16, /*give or take*/

    kIo_simple = 0, /*chunk as-is*/
    kIo_patched = 1, /*chunk is a patch*/
    kIo_journal = 2, /*chunk is a journal*/
};

typedef u8 FluxPlayChunkFormat;
typedef SHA256 FluxPlayChunkId;

struct FluxPlayContentManifest
{
    const FluxPlayChunkId* chunkIds;
    const FluxPlayChunkFormat* chunkFormats;
    u32 numberOfEntries;
};

struct IoUpstream;
struct IoDownstream;

struct IoLane
{
    const char* filePath;
    IoUpstream* tx;
    IoDownstream* rx;
    volatile u32 laneState;
    
    struct ReadTask
    {
        void* buffer;
        u32 totalBytes;
        volatile u32 state;
    } readTasks[kIo_maxStreamerCount];
    
    struct WriteTask
    {
        const void* buffer;
        u32 totalBytes;
        u32 availableBytes;
        volatile u32 state;
    } writeTasks[kIo_maxStreamerCount];
};

struct IoServiceInitParams
{
    VirtualFileSystem* vfs;
    IoServiceCallback callback;
    StorageFormat localFormat;
    const char* localPath;
    const char* localName;
};

typedef Dict<const char*, FluxPlayContentManifest> FluxPlayManifestMap_t;

struct IoService
{
    VirtualFileSystem* vfs;
    StoragePool* pool;
    IoServiceCallback callback;
    FluxPlayManifestMap_t mapping;

    IoLane lanes[kIo_maxActiveLanes];
};

typedef Aidl AetherInterface;

struct AetherContext
{
    AetherInterface ai;
    const char* host;
    u16 localPort;
    bool isServer;
};

interface ComIoService
{
    nodefine ~ComIoService() {}

    nodefine AetherContext* GetAetherContext() const = 0;

    nodefine bool FileUpload(const char* localPath, const char* remotePath = NO) = NIL;
    nodefine bool FileDownload(const char* localPath, const char* remotePath = NO) = NIL;
};

// On first call initializes the IO service with the AI context, later then the parameter is not used.
FB_STORAGE_API ComIoService* ioService(AetherContext* = NO);
}