#pragma once
#include "../Bundle.h"

namespace rw
{
namespace zipFile
{
inline constexpr size_t MaxFilenameLength = 256;

inline constexpr u16 CompressionMethod_Store = 0;
inline constexpr u16 CompressionMethod_Deflate = 8;

/// Zip64 extended information extra field (0x0001)
/// This tag can appear in the local file header if there
/// are "extra bytes" in that header.
inline constexpr u16 ExtraFileInfo_Zip64ExtensionField = 0x0001;

/// The maximum size we can represent with a 32-bit zip structure, as a 64-bit stream size.
/// (This is mostly for casting convenience)
inline constexpr s64 MaxZip32FileSize = size_t(0xFFFFFFFFu);

/// The maximum size we can represent with a 32-bit zip structure, as a 32-bit unsigned quantity.
/// (This is mostly for casting convenience)
inline constexpr u32 MaxZip32FileSize_u32 = 0xFFFFFFFFu;
}
}