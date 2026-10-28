#include <Engine.Base/Misc/Codec.h>
#include <Engine.Base/Buffer/Adapter.h>
#include <Engine.Base/Vec.h>

#include <lz4/lz4.h>
#include <zlib/zlib.h>
extern "C"
{
    #include <zstd/zstd.h>
}
#include "gzip.h"
#include <ppl.h> // for parallelism

namespace rw
{
enum : u32
{
	CodecHdr_SegmentType_Mask		= 0xFF000000,
	CodecHdr_SegmentType_Shift		= 24,
	CodecHdr_EncodedSize_Mask		= 0x00FFFFFF,
	CodecHdr_EncodedSize_Shift		= 0
};

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

// CodecMethod_None
// CodecMethod_Segmented
// CodecMethod_ZLib
// CodecMethod_SegmentedZLib
// CodecMethod_LZ4
// CodecMethod_SegmentedLZ4
// CodecMethod_GZip
// CodecMethod_SegmentedGZip
// CodecMethod_ZStd
// CodecMethod_SegmentedZStd
static inline const char* c_MethodStrings[] = {
	"none",
	"segmented",
	"zlib",
	"zlib_segment",
	"lz4",
	"lz4_segment",
	"gzip",
	"gzip_segment",
	"zstd",
	"zstd_segment"
};

const char* codecMethodToString(CodecMethod codec)
{
	if(codec < CodecMethod_Count_)
		return c_MethodStrings[int(codec)];

	return NULL;
}

u32 getCodecSegmentCount(size_t srcSize, size_t segmentSize)
{
	return (srcSize + segmentSize - 1) / segmentSize;
}

size_t getEstimatedEncodedSize(CodecMethod type, size_t srcSize, size_t segmentSize)
{
	switch(type)
	{
	case CodecMethod_None:				
		return srcSize;
	case CodecMethod_Segmented:		
		return size_t(srcSize + (getCodecSegmentCount(srcSize, segmentSize) * sizeof(Codec::Hdr)));
	
	case CodecMethod_ZLib:
	case CodecMethod_GZip:				
		return size_t(compressBound(uLong(srcSize)));
	
	case CodecMethod_SegmentedZLib:
	case CodecMethod_SegmentedGZip:		
		return size_t(compressBound(uLong(CodecSegmentSize)) + (getCodecSegmentCount(srcSize, segmentSize) * sizeof(Codec::Hdr)));
	
	case CodecMethod_LZ4:					
		return size_t(LZ4_compressBound(int(srcSize)));
	case CodecMethod_SegmentedLZ4:		
		return size_t(LZ4_compressBound(int(CodecSegmentSize)) + (getCodecSegmentCount(srcSize, segmentSize) * sizeof(Codec::Hdr)));
	
	case CodecMethod_ZStd:				
		return ZSTD_compressBound(srcSize);
	case CodecMethod_SegmentedZStd:		
		return ZSTD_compressBound(CodecSegmentSize) + (getCodecSegmentCount(srcSize, segmentSize) * sizeof(Codec::Hdr));
	default:
		EASSERT(false, "undefined encoding type");
		return size_t(-1);
	}
}

CodecMethod Codec::Hdr::segmentType() const
{
	return CodecMethod((u1 & CodecHdr_SegmentType_Mask) >> CodecHdr_SegmentType_Shift);
}
void Codec::Hdr::setSegmentType(CodecMethod segmentType)
{
	u1 |= u32(segmentType) << CodecHdr_SegmentType_Shift;
}

u32 Codec::Hdr::decodedSize() const
{
	return u2;
}
void Codec::Hdr::setDecodedSize(u32 decodedSize)
{
	u2 = decodedSize;
}

u32 Codec::Hdr::encodedSize() const
{
	return (u1 & CodecHdr_EncodedSize_Mask) >> CodecHdr_EncodedSize_Shift;
}
void Codec::Hdr::setEncodedSize(u32 encodedSize)
{
	EASSERT(encodedSize < CodecHdr_EncodedSize_Mask, "Encoded size is too large to fit in the header");
	u1 |= (encodedSize & CodecHdr_EncodedSize_Mask);
}

u32 Codec::encodeSegments(CodecMethod type, const void* srcData, size_t srcSize, void* dstData, size_t dstSize, size_t segmentSize)
{
	struct SegmentedJob
	{
		const void* sourceBuffer = nullptr;
		u32 sourceSize = 0;
		
		void* targetBuffer = nullptr;
		u32 targetSize = 0;

        SegmentedJob() = default;

		SegmentedJob(const void* sourceBuffer_, u32 sourceLen_, void* targetBuffer_, u32 targetLen_)
			: sourceBuffer(sourceBuffer_)
			, sourceSize(sourceLen_)
			, targetBuffer(targetBuffer_)
			, targetSize(targetLen_)
		{}
	};

	if(srcData == nullptr || srcSize == 0)
	{
		return u32(-1);
	}
	if(dstData == nullptr || dstSize == 0)
	{
		return u32(-1);
	}

	switch(type)
	{
	case CodecMethod_None:
		memcpy_s(dstData, dstSize, srcData, srcSize);
		return srcSize;

	case CodecMethod_GZip:
	{
		u32 encodedSize = dstSize;
		int err = gzipCompress(srcData, srcSize, dstData, &encodedSize, Z_BEST_COMPRESSION);
		EASSERT(err == Z_OK, "GZip compression failed.");
		return encodedSize;
	}

	case CodecMethod_ZLib:
	{
		uLongf encodedSize = (uLongf)dstSize;
		int err = compress2((Bytef*)dstData, &encodedSize, (const Bytef*)srcData, (uLong)srcSize, Z_BEST_COMPRESSION);
		EASSERT(err == Z_OK, "ZLib compression failed.");
		return u32(encodedSize);
	}	
	
	case CodecMethod_LZ4:
	{
		EASSERT(srcSize < LZ4_MAX_INPUT_SIZE, "Source data is too large for compressing it with LZ4");
		int encodedSize = LZ4_compress_default((const char*)srcData, (char*)dstData, (int)srcSize, (int)dstSize);
		EASSERT(encodedSize >= 0, "LZ4 compression failed.");
		return u32(encodedSize);
	}

	case CodecMethod_ZStd:
	{
		size_t encodedSize = ZSTD_compress(dstData, dstSize, srcData, srcSize, CODEC_ZSTD_COMPRESSION_LEVEL); // btultra2, if you check zstd.h.
		EASSERTF(!ZSTD_isError(encodedSize), "Zstd compression failed (%s)", ZSTD_getErrorName(encodedSize));
		return u32(encodedSize);
	}	
	default:
		break;
	}

	if(!isSegmented(type))
	{
		EASSERT(false, "Undefined encoding type when compressing");
		return u32(-1);
	}

	CodecMethod chunkCodec = CodecMethod(u32(type) - 1);
	size_t maxJobDestBufLen = getEstimatedEncodedSize(chunkCodec, segmentSize, 0);

	u32 jobCount = getCodecSegmentCount(srcSize, segmentSize);
	Vec<SegmentedJob> jobs;
	jobs.reserve(jobCount);

	const u8* sourceCursor = (const u8*)srcData;
	u8* targetCursor = (u8*) HeapAllocGlobal(jobCount * maxJobDestBufLen);

	while(srcSize > 0)
	{
		size_t jobSourceBufLen = srcSize < segmentSize ? srcSize : segmentSize;
        jobs.push_back(SegmentedJob(sourceCursor, u32(jobSourceBufLen), targetCursor, maxJobDestBufLen));
		targetCursor += maxJobDestBufLen;
		sourceCursor += jobSourceBufLen;
		srcSize -= jobSourceBufLen;
	}

	u8* destCursor = (u8*)dstData;
	u32 destLen = 0;
	
	Concurrency::parallel_for_each(jobs.begin(), jobs.end(), [chunkCodec, &destLen, &destCursor](SegmentedJob& job)
	{				
		u32 encodedSize = Codec::encodeSegments(chunkCodec, job.sourceBuffer, job.sourceSize, job.targetBuffer, job.targetSize, NO);
		job.targetSize = encodedSize;

		Codec::Hdr* segmentHdr = (Codec::Hdr*)destCursor;
		segmentHdr->setSegmentType(chunkCodec);
		segmentHdr->setEncodedSize(job.targetSize);
		segmentHdr->setDecodedSize(job.sourceSize);
		destCursor += sizeof(Codec::Hdr);

		memcpy_s(destCursor, job.targetSize, job.targetBuffer, job.targetSize);
		destCursor += job.targetSize;
		destLen += job.targetSize + sizeof(Codec::Hdr);
	});

	return destLen;
}

u32 Codec::decodeSegments(CodecMethod type, const void* srcData, size_t srcSize, void* dstData, size_t dstSize)
{
	EASSERT(srcData && srcSize, "Can't decode from null source");
	EASSERT(dstData && dstSize, "Can't decode into null destination");

	switch(type)
	{
	case CodecMethod_None:
		memcpy_s(dstData, dstSize, srcData, srcSize);
		return srcSize;

	case CodecMethod_ZLib:
	{
		uLong encodedSize = (uLong)srcSize;
		uLongf decodedSize = (uLongf)dstSize;
		int err = uncompress2((Bytef*)dstData, &decodedSize, (const Bytef*)srcData, &encodedSize);
		EASSERT(err == Z_OK, "ZLib decompression failed.");
		return u32(decodedSize);
	}	
	
	case CodecMethod_LZ4:
	{
		EASSERT(srcSize < LZ4_MAX_INPUT_SIZE, "Source data is too large for decompressing it with LZ4");
		int decodedSize = LZ4_decompress_safe((const char*)srcData, (char*)dstData, (int)srcSize, (int)dstSize);
		EASSERT(decodedSize >= 0, "LZ4 decompression failed.");
		return u32(decodedSize);
	}

	case CodecMethod_GZip:
	{
		u32 decodedSize = dstSize;
		int err = gzipDecompress(srcData, (unsigned)srcSize, dstData, &decodedSize);
		EASSERT(err == Z_OK, "GZip decompression failed.");
		return decodedSize;
	}

	case CodecMethod_ZStd:
	{
		size_t code = ZSTD_decompress(dstData, dstSize, srcData, srcSize);
		EASSERTF(!ZSTD_isError(code), "ZStd decompression failed. (%s)", ZSTD_getErrorName(code));
		u32 decodedSize = u32(code);
		return decodedSize;
	}
	default:
		break;
	}

	if(!isSegmented(type))
	{
		EASSERT(false, "Undefined encoding type when decompressing");
		return u32(-1);
	}

	const u8* sourceCursor = (const u8*)srcData;
	u8* targetCursor = (u8*)dstData;
	u32 left = srcSize;
	u32 decodedSize = 0;
	
	while(left > 0)
	{
		Codec::Hdr* hdr = (Codec::Hdr*) sourceCursor; 
		sourceCursor += sizeof(Codec::Hdr); 
		left -= sizeof(Codec::Hdr);

		decodeSegments(hdr->segmentType(), sourceCursor, hdr->encodedSize(), targetCursor, hdr->decodedSize());
		sourceCursor += hdr->encodedSize();
		targetCursor += hdr->decodedSize();
		decodedSize += hdr->decodedSize();
		left -= hdr->encodedSize();
	}

	EASSERT(((u8*)dstData + dstSize) == targetCursor, "Failed to decode encoded data (cursor isn't at the end of destination data)");
	return decodedSize;
}

SharedBuffer* Codec::encode(CodecMethod type, const SharedBuffer* buffer, size_t segmentSize)
{
	if(buffer == nullptr)
	{
		return nullptr;
	}

	// the function will add the CodecHdr size * segment count, if codec type is segmented
	size_t estimatedEncodedSize = getEstimatedEncodedSize(type, buffer->size64(), segmentSize);

	SharedBuffer* targetBuffer = new MemorySharedBuffer(estimatedEncodedSize);
	u32 encodedSize = encodeSegments(type, buffer->bytes(), buffer->size64(), targetBuffer->bytes(), targetBuffer->size64(), segmentSize);
	targetBuffer->setSize(encodedSize);
	targetBuffer->setMimeType(Mime_Compressed);
	return targetBuffer;
}

SharedBuffer* Codec::decode(CodecMethod type, const SharedBuffer* buffer, size_t decodedSize)
{
	SharedBuffer* targetBuffer = new MemorySharedBuffer(decodedSize);
	u32 size = decodeSegments(type, buffer->bytes(), buffer->size64(), targetBuffer->bytes(), targetBuffer->size64());
	EASSERT(size == decodedSize, "Decoded size doesn't match original size");
	return targetBuffer;
}
}