#pragma once
#include "Bundle.h"
#include <Engine.Base/Vec.h>
#include <Engine.Base/Buffer/Buffer.h>
#include <Engine.Base/Misc/Codec.h>

namespace rw
{
inline constexpr u32 E_CODEC_FAULT = u32(-1);

/*
    Helps to encode/decode data scatter as a series
    of segments or use a given codec freely.

    This helper class enforces the user to use
    one of the segmented codec when encoding/decoding 
    only one segment, otherwise E_CODEC_FAULT will be returned.
*/
class CompressionHelper
{
public:
    CORETOOL_API CompressionHelper(CodecMethod);
    CORETOOL_API ~CompressionHelper();

    // NOTE: result has to be the decompressed size.
    CORETOOL_API u32                     decompress(const void* srcData, StreamSize srcSize, Vec<u8>& result);

    // NOTE: result has to be the decompressed size.
    CORETOOL_API u32                     decompress(Buffer& b, StreamSize size, Vec<u8>& result);
    
    // NOTE: result has to be the decompressed size.
    CORETOOL_API u32                     decompressOneSegment(const void* srcData, StreamSize srcSize, Vec<u8>& result);
    
    // NOTE: result has to be the decompressed size.
    CORETOOL_API u32                     decompressOneSegment(Buffer& b, StreamSize size, Vec<u8>& result);
    
    // NOTE: result will be the compressed data scatter, compressed size will be returned.
    CORETOOL_API u32                     compress(const void* srcData, StreamSize srcSize, Vec<u8>& result);
    
    // NOTE: result will be the compressed data scatter, compressed size will be returned.
    CORETOOL_API u32                     compress(Buffer& b, StreamSize size, Vec<u8>& result);
    
    // NOTE: result will be the compressed data scatter, compressed size will be returned.
    CORETOOL_API u32                     compressOneSegment(const void* srcData, StreamSize srcSize, Vec<u8>& result);
    
    // NOTE: result will be the compressed data scatter, compressed size will be returned.
    CORETOOL_API u32                     compressOneSegment(Buffer& b, StreamSize size, Vec<u8>& result);

private:
    CodecMethod m_codec;
};
}