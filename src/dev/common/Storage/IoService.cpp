#include <Engine.Storage/IoService.h>

#include <Engine.Base/HashSet.h>
#include <Engine.Base/Database/DbObjectBuilder.h>
#include <Engine.Base/Misc/Interlocked.h>

#include <Engine.Core/diag/ExecutionContext.h>

#include <Engine.Storage/Delta.h>
#include <Engine.Storage/Pools/FileStoragePool.h>
#include <Engine.Storage/Pools/FlexibleStoragePool.h>

#include "IoStream.h"

namespace rw
{
XVARIABLE(bool, ioServiceEnabled, "IoService.enabled");
XVARIABLE(const char*, ioServiceLocalName, "IoService.name");
XVARIABLE(const char*, ioServiceLocalPath, "IoService.path");
XVARIABLE(const char*, ioServiceLocalFormat, "IoService.format");

void ioServiceInit(IoService* io, VirtualFileSystem* vfs, StorageFormat format, const char* localPath, const char* localName)
{
    EDEBUGF("*** IO Service : Init ***");

    io->vfs = vfs;

    PathStringBuilder poolPath;
    poolPath << localPath;
    if(!issep(poolPath.back()))
        poolPath << '/';

    switch(format)
    {
        case StorageFormat::File:
            io->pool = new FileStoragePool(vfs, poolPath.detach(), localName);
            break;
        case StorageFormat::Flexible:
            io->pool = new CasStoragePool(vfs, poolPath.detach(), localName);
            break;
        default:
            io->pool = nullptr;
            break;
    }
}

void ioServiceDeinit(IoService* io)
{
    EDEBUGF("*** IO Service : Deinit ***");

    if(io->pool)
    {
        io->pool->flush();

        delete io->pool;
        io->pool = nullptr;
    }

    io->vfs = nullptr;
}

IoLane* ioService_find(IoService* io, u32 laneState)
{
    for(u32 i = 0; i < kIo_maxActiveLanes; i++)
    {
        if(lock_read(io->lanes[i].laneState) == laneState)
            return io->lanes + i;
    }

    return nullptr;
}

IoLane* ioService_match(IoService* io, u32 laneState)
{
    for(u32 i = 0; i < kIo_maxActiveLanes; i++)
    {
        if(lock_read(io->lanes[i].laneState) & laneState)
            return io->lanes + i;
    }

    return nullptr;
}

#define IO_LANE_BEGIN_SCOPE(state) if(lock_read(lane->laneState) != (state)) return false
#define IO_LANE_END_SCOPE(state) lock_xchg(lane->laneState, (state)); return true

bool ioLaneOpen(IoService* io, const char* localPath, u32* laneIndex = NO)
{
    if(isnil(localPath))
        return false;

    if(laneIndex)
        *laneIndex = kIo_maxActiveLanes;

    IoLane* lane = ioService_find(io, kFlux_idle);
    if(!lane)
        return false;

    if(laneIndex)
        *laneIndex = (u32)(lane - io->lanes);

    lane->filePath = localPath;
    lock_xchg(lane->laneState, kFlux_ready);
    return true;
}

bool ioLaneClose(IoService* io, u32 laneIndex)
{
    if(laneIndex >= kIo_maxActiveLanes)
        return false;

    IoLane* lane = io->lanes + laneIndex;
    IO_LANE_BEGIN_SCOPE(kFlux_idle);
    lane->filePath = nullptr;
    return true;
}

enum
{
    IoTask_send = 0x01,
    IoTask_receive = 0x02,
    IoTask_patch = 0x80
};

bool ioService_progressTask(IoLane* lane, u32 flags, u32 streamers, u32 from, u32 to)
{
    if(flags & IoTask_send)
    {
        IoLane::WriteTask* baseTask = lane->writeTasks + kIo_baseTask;
        IoLane::WriteTask* headTask = streamers & kIo_headTask ? lane->writeTasks + kIo_headTask : nullptr;
        IoLane::WriteTask* patchTask = streamers & kIo_headTask ? lane->writeTasks + kIo_headTask : nullptr;
        
        if(lock_read(baseTask->state) != from)
            return false;

        lock_xchg(baseTask->state, to);

        if(headTask)
        {
            if(lock_read(headTask->state) != from)
                return false;
            
            lock_xchg(headTask->state, to);
        }
        
        if(patchTask)
        {
            if(lock_read(patchTask->state) != from)
                return false;
            
            lock_xchg(patchTask->state, to);
        }
    }

    if(flags & IoTask_receive)
    {
        IoLane::ReadTask* baseTask = lane->readTasks + kIo_baseTask;
        IoLane::ReadTask* headTask = streamers & kIo_headTask ? lane->readTasks + kIo_headTask : nullptr;
        IoLane::ReadTask* patchTask = streamers & kIo_headTask ? lane->readTasks + kIo_headTask : nullptr;
        
        if(lock_read(baseTask->state) != from)
            return false;

        lock_xchg(baseTask->state, to);

        if(headTask)
        {
            if(lock_read(headTask->state) != from)
                return false;
            
            lock_xchg(headTask->state, to);
        }
        
        if(patchTask)
        {
            if(lock_read(patchTask->state) != from)
                return false;
            
            lock_xchg(patchTask->state, to);
        }
    }

    return true;
}

IoTaskHandle ioService_beginTask(IoService* io, u32 laneIndex, u32 flags)
{
    u32 streamers = kIo_baseTask;
    if(flags & IoTask_patch)
    {
        streamers |= (kIo_headTask | kIo_patchTask);
    }

    IoLane* lane = io->lanes + laneIndex;

    if(!ioService_progressTask(lane, flags, streamers, kFlux_idle, kFlux_opened))
        return InvalidIoTaskHandle;

    return packIoTaskHandle(laneIndex, streamers, flags);
}

struct IoServiceUploadParams
{
    CodecMethod codec;
};

struct IoServiceDownloadParams
{
    const FluxPlayContentManifest* manifest;
};

bool ioService_bindTask(IoService* io, IoTaskHandle task, void* userData)
{
    u32 laneIndex;
    u32 streamers;
    u32 flags;
    
    unpackIoTaskHandle(task, &laneIndex, &streamers, &flags);
    IoLane* lane = io->lanes + laneIndex;

    if(!ioService_progressTask(lane, flags, streamers, kFlux_opened, kFlux_busy))
        return false;
    
    // -- STREAMER POLICY --
    // SEND:
    // - Base task: If upload is a patch of a file, then base is the prev (or reverse next) version.
    // - Head task: If upload is a patch of a file, then head is the current, otherwise not used.
    // - Patch task: If upload is a patch of file, then patch is the result of base and head.
    // RECEIVE:
    // - Base task: If download is a patch of a file, then base is the prev (or reverse next) version.
    // - Head task: not used
    // - Patch task: if upload is a patch of a file, then patch is the delta between base and head.
    if(flags & IoTask_send)
    {
        IoServiceUploadParams* sendParams = (IoServiceUploadParams*) userData;
        lane->tx = new IoUpstream();
        lane->tx->io = io;
        lane->tx->codec = sendParams ? sendParams->codec : CodecMethod_SegmentedZStd;

        IoLane::WriteTask& baseTask = lane->writeTasks[kIo_baseTask];
        const SharedBuffer* baseDataBuffer = /*flags & IoTask_patch ? io->pool->getChunk(*sendParams->) : */io->vfs->shareFile(lane->filePath);
        if(!baseDataBuffer)
            return false;

        baseTask.buffer = baseDataBuffer->bytes();
        baseTask.totalBytes = baseDataBuffer->size();
        baseTask.availableBytes = baseTask.totalBytes;

        const SharedBuffer* headDataBuffer = NO;
        if(flags & IoTask_patch)
        {
            if(!sendParams)
                return false;

            IoLane::WriteTask& headTask = lane->writeTasks[kIo_headTask];
            headDataBuffer = io->vfs->shareFile(lane->filePath);
            if(!headDataBuffer)
                return false;

            headTask.buffer = headDataBuffer->bytes();
            headTask.totalBytes = headDataBuffer->size();
            headTask.availableBytes = headTask.totalBytes;

            DeltaOps_t delta;

            DeltaGenerator deltaGenerator;
            if(!deltaGenerator.computeDelta(baseTask.buffer, baseTask.totalBytes, headTask.buffer, headTask.totalBytes, delta))
                return false;

            CompressionHelper compressor(CodecMethod_ZStd);
            FluxPatchEncoder patchEncoder(compressor);
            
            AllocatingBuffer patchDataBuffer;
            u32 encodedSize = patchEncoder.encodeDelta(baseTask.buffer, baseTask.totalBytes, headTask.buffer, headTask.totalBytes, delta, patchDataBuffer);
            if(!encodedSize)
                return false;

            IoLane::WriteTask& patchTask = lane->writeTasks[kIo_patchTask];
            patchTask.totalBytes = encodedSize;
            patchTask.availableBytes = patchTask.totalBytes;
            patchTask.buffer = patchDataBuffer.detach();
        }
    }

    if(flags & IoTask_receive)
    {
        IoServiceDownloadParams* recvParams = (IoServiceDownloadParams*) userData;
        lane->rx = new IoDownstream();
        lane->rx->io = io;
        lane->rx->manifest = recvParams->manifest;
        lane->rx->numberOfAvailEntries = recvParams->manifest->numberOfEntries;

        IoLane::ReadTask& baseTask = lane->readTasks[kIo_baseTask];
        baseTask.buffer = NO;
        baseTask.totalBytes = 0;

        if(flags & IoTask_patch)
        {
            if(!recvParams)
                return false;

            IoLane::ReadTask& patchTask = lane->readTasks[kIo_patchTask];
            patchTask.buffer = NO;
            patchTask.totalBytes = 0;
        }
    }

    return true;
}

enum
{
    kIo_taskFailed,
    kIo_taskContinue,
    kIo_taskCompleted
};

int ioService_execTask(IoService* io, IoTaskHandle task)
{
    u32 laneIndex;
    u32 streamers;
    u32 flags;
    
    unpackIoTaskHandle(task, &laneIndex, &streamers, &flags);
    IoLane* lane = io->lanes + laneIndex;
    if(lock_read(lane->laneState) != kFlux_streaming)
        return kIo_taskFailed;

    ioserrno err = ios_success;
    if(flags & IoTask_send)
    {
        err = ios_push(lane, lane->tx);
    }
    else if(flags & IoTask_receive)
    {
        err = ios_pull(lane, lane->rx);
    }
    else /*invalid task method*/
    {
        return kIo_taskFailed;
    }

    if(err >= ios_error)
        return kIo_taskFailed;

    if(err != ios_eof)
        return kIo_taskContinue;

    if(!ioService_progressTask(lane, flags, streamers, kFlux_busy, kFlux_done))
        return kIo_taskFailed;

    return kIo_taskCompleted;
}

bool ioService_endTask(IoService* io, IoTaskHandle task)
{
    u32 laneIndex;
    u32 streamers;
    u32 flags;
    unpackIoTaskHandle(task, &laneIndex, &streamers, &flags);

    IoLane* lane = io->lanes + laneIndex;
    // TODO: free resources
    return ioService_progressTask(lane, flags, streamers, kFlux_done, kFlux_idle);
}

bool ioLaneBeginTask(IoService* io, u32 laneIndex, u32 flags, IoTaskHandle* result)
{
    IoLane* lane = io->lanes + laneIndex;

    if(result)
        *result = InvalidIoTaskHandle;

    IO_LANE_BEGIN_SCOPE(kFlux_ready);
    {
        IoTaskHandle handle = ioService_beginTask(io, laneIndex, flags);
        if(handle == InvalidIoTaskHandle)
        {
            lock_xchg(lane->laneState, kFlux_error);
            return false;
        }
    
        if(result)
            *result = handle;
        
    }
    IO_LANE_END_SCOPE(kFlux_binding);
}

bool ioLaneBindTask(IoService* io, IoTaskHandle task, void* userData = NO)
{
    u32 laneIndex;
    u32 flags;
    unpackIoTaskHandle(task, &laneIndex, NO, &flags);
    IoLane* lane = io->lanes + laneIndex;

    IO_LANE_BEGIN_SCOPE(kFlux_binding);
    {
        if(!ioService_bindTask(io, task, userData))
        {
            lock_xchg(lane->laneState, kFlux_error);
            return false;
        }
    }
    IO_LANE_END_SCOPE(kFlux_streaming);
}

int ioLaneExecTask(IoService* io, IoTaskHandle task)
{
    int err;
    u32 laneIndex;
    unpackIoTaskHandle(task, &laneIndex);
    IoLane* lane = io->lanes + laneIndex;

    if(lock_read(lane->laneState) != kFlux_streaming) 
        return false;

    err = ioService_execTask(io, task);
    switch(err)
    {
        case kIo_taskFailed:
            lock_xchg(lane->laneState, kFlux_error);
            break;

        case kIo_taskCompleted:
            lock_xchg(lane->laneState, kFlux_completed);
            break;
    }

    return err;
}

bool ioLaneEndTask(IoService* io, IoTaskHandle task)
{
    u32 laneIndex;
    unpackIoTaskHandle(task, &laneIndex);
    IoLane* lane = io->lanes + laneIndex;

    IO_LANE_BEGIN_SCOPE(kFlux_completed);
    {
        if(!ioService_endTask(io, task))
        {
            lock_xchg(lane->laneState, kFlux_error);
            return false;
        }
    }
    IO_LANE_END_SCOPE(kFlux_idle);
}

// TODO: Upload a file as patch.

class ComIoServiceImp : public ComIoService
{
public:
    typedef HashSet<const char*> FileSet_t;

public:
    AetherContext* aiContext;
    IoService* io;
    const char* mappingFile;
    FileSet_t mapping;

public:
    void onCreate();
    void onDestroy();

    void parseMapping();
    void writeMapping();

    ComIoServiceImp(AetherContext* aiContext_, IoService* io_, const IoServiceInitParams& params)
        : io(io_)
        , aiContext(aiContext_)
    {
        ioServiceInit(io, params.vfs, params.localFormat, params.localPath, params.localName);
        onCreate();
    }

    virtual ~ComIoServiceImp() override
    {
        onDestroy();
        ioServiceDeinit(io);
    }

    AetherContext* GetAetherContext() const
        { return aiContext; }

    bool FileUpload(const char* localPath, const char* remotePath) override;
    bool FileDownload(const char* localPath, const char* remotePath) override;
};

void ComIoServiceImp::onCreate()
{
    int result;
    StringBuilder<> localMappingFile;

    // phase 1: parse mapping
    localMappingFile << io->pool->getConfiguration().poolPath << "mapping.idx";
    mappingFile = localMappingFile.detach();
    parseMapping();

    // phase 2: connect to mesh
    if(aiContext->isServer)
        result = AidlOpen(&aiContext->ai, aiContext->localPort);
    else
        result = AidlJoin(&aiContext->ai, aiContext->host, aiContext->localPort);

    EASSERTF(result == DL_OK, "IO service: failed to %s the network (error=%d).", (aiContext->isServer ? "open" : "join to"), result);
}

void ComIoServiceImp::onDestroy()
{
    writeMapping();
}

void ComIoServiceImp::parseMapping()
{
    ScopedRef<SharedBuffer> mappingsBuffer = io->vfs->shareFile(mappingFile);
    if(mappingsBuffer.isNull())
        return;

    DbObjectArray toc;
    toc.fromJsonString((const char*)mappingsBuffer->bytes());

    for(DbObjectElementIterator it(toc); it.more();)
    {
        DbObject entry = it.next().valueObject();
        const char* file = entry["file"].valueString();
        EASSERT(!isnil(file), "Mapping has no file");
        
        eastl::vector<FluxPlayChunkId> fileChunkIds;
        eastl::vector<FluxPlayChunkFormat> fileChunkFormats;

        {
            DbObjectArray manifest = entry["manifest"].valueArray();
            for(DbObjectElementIterator manifestIt(manifest); manifestIt.more();)
            {
                DbObject chunkMeta = manifestIt.next().valueObject();

                FluxPlayChunkFormat& chunkFormat = fileChunkFormats.push_back();
                chunkFormat = chunkMeta.at(0).valueInteger();
                EASSERT(chunkFormat >= kIo_simple && chunkFormat <= kIo_journal, "Mapping has invalid chunk format");

                FluxPlayChunkId& chunkId = fileChunkIds.push_back();
                chunkId.parse(chunkMeta.at(1).valueString());

                EASSERT(chunkId != ContentId(), "Mapping has no hash");
            }
        }

        FluxPlayContentManifest manifest;
        manifest.numberOfEntries = fileChunkIds.size();
        manifest.chunkFormats = fileChunkFormats.detach();
        manifest.chunkIds = fileChunkIds.detach();

        file = safedup(file);
        mapping.insert(file);
        io->mapping.insert(file, manifest);
    }
}

void ComIoServiceImp::writeMapping()
{
    DbObjectArrayBuilder tocBuilder;

    for(const FluxPlayManifestMap_t::pair_type& mapping : io->mapping)
    {
        const FluxPlayContentManifest& manifest = mapping.second;

        DbObjectArrayBuilder manifestBuilder;
        for(u32 i = 0; i < manifest.numberOfEntries; ++i)
        {
            manifestBuilder << TDBARR(manifest.chunkFormats[i] << manifest.chunkIds[i]);
        }

        DbObjectBuilder fileBuilder;
        fileBuilder << "file" << mapping.first;
        fileBuilder << "manifest" << manifestBuilder.done();

        tocBuilder << fileBuilder.done();
    }

    JsonConversionParams convParams;
    convParams.indentRoots = true;
    convParams.indentChildren = false;

    StringBuilder<> json;
    tocBuilder.done().toJsonString(json, convParams);

    ScopedRef<Buffer> mappingsBuffer = io->vfs->createFile(mappingFile);
    if(mappingsBuffer.isNull())
        return;

    mappingsBuffer->write(json.c_str(), json.size());
}

bool ioService_stream(ComIoServiceImp* impl, const char* filePath, u32 flags, IoLane** lanePtr, void* loadParams = NO)
{
    EDEBUGF("*** IO Service : Streaming File ***");
    EDEBUGF("File path : %s", filePath);
    
    IoService* io = impl->io;

    u32 lane;
    if(!ioLaneOpen(io, filePath, &lane))
        return false;

    IoTaskHandle task;
    if(!ioLaneBeginTask(io, lane, flags/*IoTask_[flags]*/, &task))
    {
        ioLaneClose(io, lane);
        return false;
    }

    if(!ioLaneBindTask(io, task, loadParams))
    {
        ioLaneEndTask(io, task);
        ioLaneClose(io, lane);
        return false;
    }

    // This makes it block the thread till execution is not finished
    for(;;)
    {
        int ret = ioLaneExecTask(io, task);
        if(ret == kIo_taskCompleted)
            break;

        if(ret == kIo_taskFailed)
        {
            ioLaneEndTask(io, task);
            ioLaneClose(io, lane);
            return false;
        }
    }

    if(!ioLaneEndTask(io, task))
    {
        ioLaneClose(io, lane);
        return false;
    }

    if(lanePtr)
        *lanePtr = io->lanes + lane;

    return ioLaneClose(io, lane);
}

bool ComIoServiceImp::FileUpload(const char* localPath, const char* remotePath)
{
    // TODO: decide when it's patch.
    if(mapping.get(localPath))
        return true;

    IoServiceUploadParams loadParams;
    loadParams.codec = CodecMethod_SegmentedZStd;

    IoLane* lane;
    if(ioService_stream(this, localPath, IoTask_send, &lane, &loadParams))
    {
        IoUpstream* stream = lane->tx;
        
        FluxPlayContentManifest manifest;
        manifest.numberOfEntries = stream->chunkIds.size();
        manifest.chunkIds = stream->chunkIds.detach();
        manifest.chunkFormats = stream->chunkFormats.detach();

        mapping.insert(localPath);
        io->mapping.insert(localPath, manifest);
        return true;
    }

    return false;
}

bool ComIoServiceImp::FileDownload(const char* localPath, const char* remotePath)
{
    // TODO: decide when it's patch.
    if(mapping.get(localPath))
        return true;

    if(io->pool->isLocal())
    {
        
    }

    // TODO: request chunk IDs using the m802 (mesh80211) interface using remote path
    IoServiceDownloadParams loadParams;
    loadParams.manifest = nullptr;

    IoLane* lane;
    if(ioService_stream(this, remotePath, IoTask_receive, &lane, &loadParams))
    {
        // TODO: idk.
        return true;
    }

    return false;
}

ComIoService* ioService(AetherContext* ai)
{
    if(!g_ioServiceEnabled)
        return nullptr;

    static IoService m_io;
    static ComIoServiceImp* m_comIo;
    
    RUN_ONCE(m_comIo);
    IoServiceInitParams params;
    params.vfs = ExecutionContext::getVirtualFileSystem();
    params.localFormat = toStorageFormat(g_ioServiceLocalFormat);
    params.localPath = g_ioServiceLocalPath;
    params.localName = g_ioServiceLocalName;
    m_comIo = new ComIoServiceImp(ai, &m_io, params);

    RAN_ONCE = YES;
    return m_comIo;
}
}