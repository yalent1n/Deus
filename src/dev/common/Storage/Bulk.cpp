#include <Engine.Storage/Bulk.h>
#include <Engine.Storage/Delta.h>

namespace rw
{
void BulkEncodeJob::writeEntry(Buffer& output) const
{
    output.write(&entry, sizeof(CatEntry));
}

void BulkEncodeJob::writeEncodeData(Buffer& output) const
{
    output.write(encodeData, entry.size);
}

SwiftBatchPatchEncoder::SwiftBatchPatchEncoder(const DbOid& signature, CodecMethod codec)
    : m_signature(signature)
    , m_transferScatterCount(0)
    , m_compressionHelper(codec)
{}

SwiftBatchPatchEncoder::~SwiftBatchPatchEncoder()
{}

const DbOid& SwiftBatchPatchEncoder::getSignature() const
{
    return m_signature;
}

bulk_encoder_options& SwiftBatchPatchEncoder::getOptions()
{
    return m_encoderOptions;
}

const bulk_encoder_options& SwiftBatchPatchEncoder::getOptions() const
{
    return m_encoderOptions;
}

bool SwiftBatchPatchEncoder::prepareBulkTransfer(bool useDeltaCompression, bool usePlatformCompression)
{
    m_encoderOptions.mUseDeltaCompression = useDeltaCompression;
    m_encoderOptions.mUsePlatformCompression = usePlatformCompression;
    return true;
}

BulkEncodeJob* SwiftBatchPatchEncoder::prepareScatter(const void* bytes, size_t length, bool disableOptions)
{
    if(!bytes || !length)
        return nullptr;

    bool doCompress = !disableOptions && m_encoderOptions.mUsePlatformCompression;

    const void* scatterData = bytes;
    size_t scatterOriginalSize = length;
    size_t scatterSize = scatterOriginalSize;

    if(doCompress)
    {
        Vec<u8> encoded;
        u32 encodedSize = m_compressionHelper.compress(scatterData, scatterSize, encoded);
        if(encodedSize == E_CODEC_FAULT)
            return nullptr;

        if(encodedSize < scatterOriginalSize)
        {
            scatterSize = encodedSize;
            scatterData = encoded.detach();
        }
        else
            doCompress = false;
    }

    CatEntry ce;
    ce.sha.calculate(scatterData, scatterSize);
    if(EncodeJobs_t::iterator it = m_jobs.find(ce.sha); it != m_jobs.end())
        return &it->second; // already in the transfer, don't allow duplicates

    ce.size = scatterSize;
    ce.flagsAndCluster = 0;
    ce.originalSize = scatterOriginalSize;    

    if(doCompress)
        ce.flagsAndCluster |= CatEntry::Compressed;    

    EDEBUGF("Cas File <%s> Size : %u, Flags : %u, Cluster : %u, Original Size : %u", 
        ContentIdStr(ce.sha).str, ce.size, ce.flagsAndCluster & ~0x000000FF, ce.cluster(), ce.originalSize);
    EncodeJobs_t::pair_type& job = m_jobs.insert(ce.sha);
    job.second.entry = ce;
    job.second.decodeData = bytes;
    job.second.encodeData = scatterData;
    ++m_transferScatterCount;
    return &job.second;
}

BulkEncodeJob* SwiftBatchPatchEncoder::prepareScatter(const void* headData, size_t headSize, const void* baseData, size_t baseSize, bool disableOptions)
{
    if(!headData || !headSize || !baseData || !baseSize)
        return nullptr;

    bool doDelta = !disableOptions && m_encoderOptions.mUseDeltaCompression;
    bool doCompress = !disableOptions && m_encoderOptions.mUsePlatformCompression;

    // Delta scatters should be allowed when preparing them.
    if(!doDelta)
        return nullptr;

    // Prepend base data
    if(!prepareScatter(baseData, baseSize))
        return nullptr;

    const void* scatterData = headData;
    size_t scatterOriginalSize = headSize;
    size_t scatterSize = scatterOriginalSize;

    {
        // make sure we store changes, don't compare the size cost
        DeltaOps_t delta;
        if(!m_deltaGenerator.computeDelta(baseData, baseSize, headData, headSize, delta))
            return nullptr;
        
        FluxPatchEncoder encoder(m_compressionHelper);
        
        AllocatingBuffer patchDataBuffer;
        encoder.encodeDelta(baseData, baseSize, headData, headSize, delta, patchDataBuffer);

        scatterSize = patchDataBuffer.length();
        scatterData = patchDataBuffer.detach();
    }

    if(doCompress)
    {
        Vec<u8> compressed;
        u32 compressedSize = m_compressionHelper.compress(scatterData, scatterSize, compressed);
        
        // NOTE: This esentially handles E_CODEC_FAULT, because unsigned -1 is 42... a lot.
        if(compressedSize < scatterOriginalSize)
        {
            scatterSize = compressedSize;
            scatterData = compressed.detach();
        }
        else
            doCompress = false;
    }

    CatEntry ce;
    ce.sha.calculate(scatterData, scatterSize);
    if(EncodeJobs_t::iterator it = m_jobs.find(ce.sha); it != m_jobs.end())
        return &it->second; // already in the transfer, don't allow duplicates

    ce.size = scatterSize;
    ce.flagsAndCluster = CatEntry::Delta;
    ce.originalSize = scatterOriginalSize;
    
    if(doCompress)
        ce.flagsAndCluster |= CatEntry::Compressed;
    
    ce.setCluster(0xFF);
    EncodeJobs_t::pair_type& job = m_jobs.insert(ce.sha);
    job.second.entry = ce;
    job.second.decodeData = headData;
    job.second.encodeData = scatterData;
    ++m_transferScatterCount;
    return &job.second;
}

BulkEncodeJob* SwiftBatchPatchEncoder::prepareScatter(StoragePool& pool, const ContentId& headSha, const ContentId& baseSha, bool disableOptions)
{
    ScopedRef<SharedBuffer> head;
    if(pool.getChunk(headSha, &head) < FetchResult::Found)
        return nullptr;

    static const ContentId c_emptySha = ContentId();

    ScopedRef<SharedBuffer> base;
    if(pool.getChunk(baseSha, &base) < FetchResult::Found)
        return nullptr;

    ++m_transferScatterCount;
    return prepareScatter(head->bytes(), head->size64(), base->bytes(), base->size64(), disableOptions);
}

bool SwiftBatchPatchEncoder::finishBulkTransfer(StoragePool& pool)
{
    for(const auto& it : m_jobs)
    {
        const BulkEncodeJob& job = it.second;
        MemorySharedBuffer msb(job.encodeData, job.entry.size);
        pool.putChunk(&msb);
    }

    m_jobs.clear();
    return true;
}

bool SwiftBatchPatchEncoder::finishBulkTransferData(Buffer& result)
{
    for(const auto& it : m_jobs)
    {
        const BulkEncodeJob& job = it.second;
        job.writeEntry(result);
        job.writeEncodeData(result);
    }

    m_jobs.clear();
    return true;
}
}