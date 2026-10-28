#pragma once
#include <Engine.Storage/IoService.h>
#include <Engine.Base/Misc/Codec.h>

namespace rw
{
/* Error codes */
typedef int ioserrno;
inline constexpr ioserrno ios_eof = ioserrno(-1);
inline constexpr ioserrno ios_success = 0;
inline constexpr ioserrno ios_keep = 1; /*can still process*/

inline constexpr ioserrno ios_error         = 0x12C;
inline constexpr ioserrno ios_storage_error = ios_error + 1;
inline constexpr ioserrno ios_lane_error    = ios_error + 2;

inline constexpr u32 ios_pipewincap = 256 KiB;
inline constexpr u32 ios_pipecap = 1 MiB;

// Compresses a stream as 1 MiB chunks and push onto whatever device it can handle (file, storage pool, etc.)

typedef struct IoUpstream
{
    IoService* io;
    CodecMethod codec;
    eastl::vector<FluxPlayChunkId> chunkIds;
    eastl::vector<FluxPlayChunkFormat> chunkFormats;

} *iolane_upstream_handle;

typedef struct IoDownstream
{
    IoService* io;
    const FluxPlayContentManifest* manifest;
    u32 numberOfAvailEntries;
} *iolane_downstream_handle;

ioserrno ios_push(IoLane*, iolane_upstream_handle);
ioserrno ios_pull(IoLane*, iolane_downstream_handle);
}