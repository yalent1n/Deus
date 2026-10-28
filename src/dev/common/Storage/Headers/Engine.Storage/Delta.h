#pragma once
#include "CatalogTypes.h"
#include <Engine.Base/Vec.h>
#include <Engine.Base/Buffer/Adapter.h>
#include <Engine.ToolCore/CompressionHelper.h>

namespace rw
{
struct DeltaOp
{
    enum OpCode : u8
    {
        /*
        Read N bytes from base.
        <varint>
        */
        READ = 0x00,
    
        /*
        Skip N bytes from base.
        <varint>
        */
        SKIP = 0x01,
    
        /*
        Insert N bytes from delta.
        <varint>
        */
        INSERT = 0x02
    };

    static inline const char* opNames[] = { "READ", "SKIP", "INSERT" };

    OpCode opCode;
    size_t index;
    size_t count;

    DeltaOp()
        : opCode(READ)
        , index(0)
        , count(0)
    {}

    DeltaOp(OpCode opCode_, size_t index_, size_t count_)
        : opCode(opCode_)
        , index(index_)
        , count(count_)
    {}
};

typedef Vec<DeltaOp> DeltaOps_t;
typedef Vec<DeltaOp>::iterator DeltaOpIt_t;

class DeltaGenerator
{
public:
    static constexpr u32 c_minSlidingWindowSize = 128;

    FB_STORAGE_API bool computeDelta(const void* baseData, u32 baseSize, const void* headData, u32 headSize, DeltaOps_t& result);
};

class FluxPatchEncoder
{
public:
    FluxPatchEncoder(CompressionHelper& compressionHelper)
        : m_compressionHelper(compressionHelper)
    {}

    CompressionHelper& compressionHelper() const { return m_compressionHelper; }

    FB_STORAGE_API u32 encodeDelta(const void* baseData, u32 baseSize, const void* headData, u32 headSize, const DeltaOps_t& delta, Buffer& result);
    FB_STORAGE_API u32 decodeDelta(const void* baseData, u32 baseSize, const void* patchData, u32 patchSize, Buffer& result);

private:
    struct DeltaReplaceOp
    {
        u32 baseOffset;
        u32 baseSize;
        u32 headOffset;
        u32 headSize;

        DeltaReplaceOp()
            : baseOffset(0)
            , baseSize(0)
            , headOffset(0)
            , headSize(0)
        {}

        DeltaReplaceOp(u32 baseOffset, u32 baseSize, u32 headOffset, u32 headSize)
            : baseOffset(baseOffset)
            , baseSize(baseSize)
            , headOffset(headOffset)
            , headSize(headSize)
        {}
    };

    typedef Vec<DeltaReplaceOp> DeltaReplaceOps_t;
    typedef Vec<DeltaReplaceOp>::iterator DeltaReplaceOpIt_t;

private:
    void encodeUncompressedDelta(const u8* headData, const DeltaReplaceOps_t& replaceOps, Buffer& result);
    void decodeUncompressedDelta(const u8* baseData, u32& baseIndex, Adapter& patchData, Buffer& result);
    void encodeCompressedDelta(const u8* headData, const DeltaReplaceOps_t& replaceOps, Buffer& result);
    void decodeCompressedDelta(const u8* baseData, u32& baseIndex, Adapter& patchData, Buffer& result);
    void convertToReplaceOps(const DeltaOps_t& delta, DeltaReplaceOps_t& result);

private:
    CompressionHelper& m_compressionHelper;
};
}