#include "IoStream.h"
#include <Engine.Base/Misc/Interlocked.h>

namespace rw
{
ioserrno ios_push(IoLane* lane, iolane_upstream_handle stream)
{
    IoLane::WriteTask* task = lane->writeTasks + kIo_baseTask;
    
    if(lock_read(task->state) != kFlux_busy)
        return ios_lane_error;
    
    {
        // if patch lane is active, push that instead of base
        IoLane::WriteTask* task2 = lane->writeTasks + kIo_patchTask;
        if(lock_read(task2->state) == kFlux_busy)
            task = task2;
    }

    if(task->availableBytes == 0)
        return ios_eof;

    const void* bytes = (const u8*)task->buffer + (task->totalBytes - task->availableBytes);
    size_t size = (task->availableBytes < ios_pipecap) ? task->availableBytes : ios_pipecap;
    MemorySharedBuffer msb(bytes, size);

    // force segmented codec by doing a modulo 2 in bitwise
    CodecMethod codec = (stream->codec & 1) == NO ? (CodecMethod)(stream->codec + 1) : stream->codec;
    SharedBuffer* encodedBuffer = Codec::encode(codec, &msb, ios_pipewincap);
    InsertResult result = stream->io->pool->putChunk(encodedBuffer);

    if(result != InsertResult::Inserted && result != InsertResult::AlreadyPresent)
        return ios_storage_error;
    
    stream->chunkFormats.push_back(kIo_simple);
        
    FluxPlayChunkId& chunkId = stream->chunkIds.push_back();
    chunkId.calculate(encodedBuffer->bytes(), encodedBuffer->size64());

    task->availableBytes -= size;
    return task->availableBytes ? ios_keep : ios_eof;
}

ioserrno ios_pull(IoLane* lane, iolane_downstream_handle stream)
{
    if(stream->numberOfAvailEntries == 0)
        return ios_eof;

    const FluxPlayContentManifest* manifest = stream->manifest;
    u32 contentIndex = manifest->numberOfEntries - stream->numberOfAvailEntries;
    FluxPlayChunkId contentId = manifest->chunkIds[contentIndex];

    IoLane::ReadTask* task = lane->readTasks + kIo_baseTask;
    if(lock_read(task->state) != kFlux_busy)
        return ios_lane_error;        

    IoLane::ReadTask* patchTask = lane->readTasks + kIo_patchTask;
    if(manifest->chunkFormats[contentIndex] == kIo_patched)
    {
        if(lock_read(patchTask->state) != kFlux_busy)
            return ios_lane_error;

        FluxPlayChunkId patchContentId;
        if(!stream->io->pool->getPatch(contentId, contentId, patchContentId))
            return ios_storage_error;

        if(!stream->io->pool->fetchChunk(patchContentId, NO, &patchTask->totalBytes))
            return ios_storage_error;

        patchTask->buffer = HeapAllocGlobal(patchTask->totalBytes);
        (void) stream->io->pool->fetchChunk(patchContentId, NO, &patchTask->totalBytes);
    }
    
    if(!stream->io->pool->fetchChunk(contentId, NO, &task->totalBytes))
        return ios_storage_error;

    task->buffer = HeapAllocGlobal(task->totalBytes);
    (void) stream->io->pool->fetchChunk(contentId, task->buffer, &task->totalBytes);
    
    stream->numberOfAvailEntries--;
    return (stream->numberOfAvailEntries ? ios_keep : ios_eof);
}
}