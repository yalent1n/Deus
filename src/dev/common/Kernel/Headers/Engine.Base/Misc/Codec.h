#pragma once
#include "../Buffer/SharedBuffer.h"

// btultra2, check zstd.h.
#define CODEC_ZSTD_COMPRESSION_LEVEL 19

namespace rw
{
enum CodecMethod
{
	CodecMethod_None,						// no compression
	
	// Used when we don't give the decoder a specified codec (every segmentation method),
	// so the decoder will recognise from the segment header.
	CodecMethod_Segmented,					// no compression, just 64KiB blocks
	CodecMethod_ZLib,							// zlib compression
	CodecMethod_SegmentedZLib,				// zlib compression with 64KiB blocks
	CodecMethod_LZ4,							// lz4 compression
	CodecMethod_SegmentedLZ4,				// lz4 compression with 64KiB blocks
	CodecMethod_GZip,							// gz compression
	CodecMethod_SegmentedGZip,				// gz compression with 64KiB blocks
	CodecMethod_ZStd,							// zstandard compression
	CodecMethod_SegmentedZStd,				// zstandard compression with 64KiB blocks
	CodecMethod_Count_
};

FB_BASE_API const char* codecMethodToString(CodecMethod);

enum
{
	CodecSegmentSize = 64 KiB
};

FB_BASE_API u32 getCodecSegmentCount(size_t srcSize, size_t segmentSize);
FB_BASE_API size_t getEstimatedEncodedSize(CodecMethod codec, size_t srcSize, size_t segmentSize = NO);

class Codec
{
public:
	struct Hdr
	{
		u32 u1;
		u32 u2;

		CodecMethod segmentType() const;
		void setSegmentType(CodecMethod segmentType);
		
		u32 decodedSize() const;
		void setDecodedSize(u32 originalSize);

		u32 encodedSize() const;
		void setEncodedSize(u32 encodedSize);
	};

	static_assert(sizeof(Codec::Hdr) == 8);

	/*
	* Encodes the source data into destination data using the specified codec.
	* Returns the actual encoded size (with the segment headers) if the codec is valid, otherwise -1.
	* 
	* @param type: The codec which the source data will be encoded into destination data.
	* @param srcData: Source data to encode
	* @param srcSize: Source data size
	* @param dstData: Destination data that receives the encoded data
	* @param dstSize: Destination data size (estimated encoded size)
	*/
	FB_BASE_API static u32 encodeSegments(CodecMethod type, const void* srcData, size_t srcSize, void* dstData, size_t dstSize, size_t segmentSize = NO);

	/*
	* Decodes the encoded data from source data into destination data.
	* Returns the decoded size (original size) if it's succeeded.
	* 
	* @param type: Codec the data was encoded in
	* @param srcData: Source data to decode
	* @param srcSize: Source data size
	* @param dstData: Destination data that receives the decoded data
	* @param dstSize: Destination data size (original size)
	*/
	FB_BASE_API static u32 decodeSegments(CodecMethod type, const void* srcData, size_t srcSize, void* dstData, size_t dstSize);

	/*
	* Encodes the buffer with the specified codec type.
	* Returns the encoded buffer with original buffer set to the unencoded buffer.
	* 
	* @param type: The codec which the buffer will be encoded in.
	* @param buffer: Buffer to encode
	*/
	FB_BASE_API static SharedBuffer* encode(CodecMethod type, const SharedBuffer* buffer, size_t segmentSize);

	/*
	* Decodes the buffer.
	* Returns the decoded buffer with original buffer set to encoded buffer.
	* 
	* @param type: Codec the data was encoded in
	* @param buffer: Buffer to decode
	* @param decodedSize: Decoded data's size
	*/
	FB_BASE_API static SharedBuffer* decode(CodecMethod type, const SharedBuffer* buffer, size_t decodedSize);
};
}