#include <Engine.ToolCore/CompressionHelper.h>

namespace rw
{
inline bool isSegmented(CodecMethod type)
{
	switch(type)
	{
	case CodecMethod_Segmented:
	case CodecMethod_SegmentedZLib:
	case CodecMethod_SegmentedLZ4:
	case CodecMethod_SegmentedGZip:
	case CodecMethod_SegmentedZStd:
		return true;
	default:
		return false;
	}
}

CompressionHelper::CompressionHelper(CodecMethod codec)
    : m_codec(codec)
{}

CompressionHelper::~CompressionHelper()
{}

u32 CompressionHelper::decompress(const void* srcData, StreamSize srcSize, Vec<u8>& result)
{
    u32 decodedSize = Codec::decodeSegments(m_codec, srcData, srcSize, result.begin(), result.capacity());
    if(decodedSize > result.capacity())
        return E_CODEC_FAULT;
        
    result.set_size(decodedSize);
    return decodedSize;
}

u32 CompressionHelper::decompress(Buffer& b, StreamSize size, Vec<u8>& result)
{
    if(!size)
        return E_CODEC_FAULT;

    u8* ptr = (u8*) HeapAllocGlobal(size);
    b.read(ptr, size);

    u32 decodedSize = decompress(ptr, size, result);
    HeapFreeGlobal(ptr);
    return decodedSize;
}

u32 CompressionHelper::decompressOneSegment(const void* srcData, StreamSize srcSize, Vec<u8>& result)
{
    if(srcSize > CodecSegmentSize || m_codec)
        return E_CODEC_FAULT;

    if(!isSegmented(m_codec))
        return E_CODEC_FAULT;

    return decompress(srcData, srcSize, result);
}

u32 CompressionHelper::decompressOneSegment(Buffer& b, StreamSize size, Vec<u8>& result)
{
    if(size > CodecSegmentSize || m_codec)
        return E_CODEC_FAULT;

    if(!isSegmented(m_codec))
        return E_CODEC_FAULT;

    u8* ptr = (u8*) HeapAllocGlobal(size);
    b.read(ptr, size);

    u32 decodedSize = decompress(ptr, size, result);
    HeapFreeGlobal(ptr);
    return decodedSize;
}

u32 CompressionHelper::compress(const void* srcData, StreamSize srcSize, Vec<u8>& result)
{
    result.reserve(getEstimatedEncodedSize(m_codec, srcSize));
    u32 encodedSize = Codec::encodeSegments(m_codec, srcData, srcSize, result.begin(), result.capacity());
    result.set_size(encodedSize);
    return encodedSize;
}

u32 CompressionHelper::compress(Buffer& b, StreamSize size, Vec<u8>& result)
{
    if(!size)
        return E_CODEC_FAULT;

    u8* ptr = (u8*) HeapAllocGlobal(size);
    b.read(ptr, size);

    u32 decodedSize = compress(ptr, size, result);
    HeapFreeGlobal(ptr);
    return decodedSize;
}

u32 CompressionHelper::compressOneSegment(const void* srcData, StreamSize srcSize, Vec<u8>& result)
{
    if(srcSize > CodecSegmentSize || m_codec)
        return E_CODEC_FAULT;

    if(!isSegmented(m_codec))
        return E_CODEC_FAULT;

    return compress(srcData, srcSize, result);
}

u32 CompressionHelper::compressOneSegment(Buffer& b, StreamSize size, Vec<u8>& result)
{
    if(size > CodecSegmentSize || m_codec)
        return E_CODEC_FAULT;

    if(!isSegmented(m_codec))
        return E_CODEC_FAULT;

    u8* ptr = (u8*) HeapAllocGlobal(size);
    b.read(ptr, size);

    u32 decodedSize = compress(ptr, size, result);
    HeapFreeGlobal(ptr);
    return decodedSize;
}
}