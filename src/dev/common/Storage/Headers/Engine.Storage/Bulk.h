#pragma once
#include "Delta.h"
#include "StoragePool.h"
#include <Engine.Base/Crypto/SHA.h>
#include <Engine.Base/Database/DbOid.h>
#include <Engine.Base/HashMap.h>
#include <Engine.Base/Buffer/Adapter.h>
#include <Engine.ToolCore/CompressionHelper.h>

namespace rw
{
struct bulk_encoder_options
{
    bool mUseDeltaCompression = true;
    bool mUsePlatformCompression = true; // this will compress delta scatter too
};

// the input/output datas are already prepared depending on the options (if compress, get estimated size, etc.)
struct BulkEncodeJob
{
    const void* decodeData;
    const void* encodeData;
    CatEntry entry;

    FB_STORAGE_API void writeEntry(Buffer& output) const;
    FB_STORAGE_API void writeEncodeData(Buffer& output) const;
};

class SwiftBatchPatchEncoder
{
public:
    typedef Dict<ContentId, BulkEncodeJob> EncodeJobs_t;

    FB_STORAGE_API  SwiftBatchPatchEncoder(const DbOid& signature, CodecMethod codec = CodecMethod_SegmentedZLib /*i guess all platform uses segment ZLib now*/);
    FB_STORAGE_API ~SwiftBatchPatchEncoder();

    FB_STORAGE_API const DbOid& getSignature() const;
    FB_STORAGE_API bulk_encoder_options& getOptions();
    FB_STORAGE_API const bulk_encoder_options& getOptions() const;

    // Prepares the encoder jobs for the given transfer type.
    FB_STORAGE_API bool prepareBulkTransfer(bool useDeltaCompression = true, bool usePlatformCompression = true);

    // Prepares a scatter data that will be included in the transfer.
    FB_STORAGE_API BulkEncodeJob* prepareScatter(const void* bytes, size_t length, bool disableOptions = false);

    // Prepares a delta scatter data (if allowed) that will be included in the transfer.
    // 
    // NOTE: This automatically prepends base data for transfer.
    FB_STORAGE_API BulkEncodeJob* prepareScatter(const void* headData, size_t headSize, const void* baseData, size_t baseSize, bool disableOptions = false);

    // Prepares a file from a storage pool that will be included in the transfer.
    // 
    // NOTE: This is the most 'hiearchial' method to manage storage files with patching feature.
    FB_STORAGE_API BulkEncodeJob* prepareScatter(StoragePool& pool, const ContentId& headSha, const ContentId& baseSha = ContentId(), bool disableOptions = false);

    // Finish a bulk transfer into a storage pool.
    FB_STORAGE_API bool finishBulkTransfer(StoragePool& pool);

    // Create bulk transfer data, for moving multiple files over the Internet.
    FB_STORAGE_API bool finishBulkTransferData(Buffer& result);

private:
    const DbOid& m_signature;
    EncodeJobs_t m_jobs;
    bulk_encoder_options m_encoderOptions;
    size_t m_transferScatterCount;
    CompressionHelper m_compressionHelper;
    DeltaGenerator m_deltaGenerator;
};
}