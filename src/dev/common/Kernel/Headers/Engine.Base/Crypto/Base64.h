#pragma once
#include "../Bundle.h"

namespace rw
{
class StringEditor;

// Calculates the Base64 length from raw data length.
FB_BASE_API size_t base64_encodelength(size_t srcLen);

// Calculates the raw data length from Base64 length.
// This doesn't calculates padding
FB_BASE_API size_t base64_decodelength(size_t srcLen);

// Encodes the source data to destination in Base64.
// Returns the size of encoded string on success, otherwise zero.
// @param src: Source data
// @param srcLen: Source data length
// @param dst: Destination buffer
// @param dstLen: Destination buffer length
FB_BASE_API size_t base64_encode(const void* src, size_t srcLen, void* dst, size_t dstLen);

// Encodes the source data to destination in Base64.
// Returns the size of encoded string on success, otherwise zero.
// @param src: Source data
// @param srcLen: Source data length
// @param dst: This will receive the Base64 string
FB_BASE_API size_t base64_encode(const void* src, size_t srcLen, StringEditor& dst);

// Decodes the Base64 string to destination.
// Returns the size of decoded string on success, otherwise zero.
// @param src: Base64 string
// @param srcLen: Base64 string length
// @param dst: Destination buffer
// @param dstLen: Destination buffer length
FB_BASE_API size_t base64_decode(const void* src, size_t srcLen, void* dst, size_t dstLen);

// Decodes the Base64 string to destination.
// Returns the size of decoded string on success, otherwise zero.
// @param src: Base64 string
// @param srcLen: Base64 string length
// @param dst: This will receive the original data
FB_BASE_API size_t base64_decode(const void* src, size_t srcLen, StringEditor& dst);

// Omits the Base64 characters and replaces them with the URL ones.
// Returns the encoded length on success, otherwise zero.
// @param src: Raw source
// @param srcLen: Raw source length
FB_BASE_API size_t base64_urlencode(const void* src, size_t srcLen, void* dst, size_t dstLen);

// Omits the Base64 characters and replaces them with the URL ones.
// Returns the encoded length on success, otherwise zero.
// @param src: Raw source
// @param srcLen: Raw source length
FB_BASE_API size_t base64_urlencode(const void* src, size_t srcLen, StringEditor& dst);

// Omits the URL characters from src and replaces them with the Base64 ones to dst.
// Returns the decoded length on success, otherwise zero.
// @param src: Base64 string
// @param srcLen: Base64 string length
// @param dst: This will receive the original data
FB_BASE_API size_t base64_urldecode(const void* src, size_t srcLen, void* dst, size_t dstLen);

// Omits the URL characters from src and replaces them with the Base64 ones to dst.
// Returns the decoded length on success, otherwise zero.
// @param src: Base64 string
// @param srcLen: Base64 string length
// @param dst: This will receive the original data
FB_BASE_API size_t base64_urldecode(const void* src, size_t srcLen, StringEditor& dst);

}