#pragma once
#include "DeployBuildImage.h"

namespace rw
{
inline constexpr u32 c_invalidSerialNo = 0;

struct OpsLogHeader
{
    inline static constexpr u32 MAGIC = make_cc('o', 'p', 's', '0');
    u32 magic;
    u32 opsCount;
    u32 pad[2] {0, 0};
};

struct OpLogStats
{
    u32 added;
    u32 removed;
    u32 patched;
};

class DeployBuildImageOpLog : public BuildImageOpLog
{
public:
    // case A (default): Owns the operation (rebuild base op to contain the lsn)
    // case B (parsing): Shares the operation (reading the oplog file and advancing the cursor)
    typedef Vec<DbObject> OpsVec_t;

public:
    FB_STORAGE_API DeployBuildImageOpLog(BuildImageLayout& layout, const BuildImageInstallChunkBuildLayer& icbl);

    const BuildImageInstallChunkBuildLayer& getIcBuildLayer() const { return m_icBuildLayer; }
    
    const OpsVec_t& getOps()                const { return m_ops; }
    OpsVec_t& getOps()                      { return m_ops; }

    static FB_STORAGE_API void readOps(Buffer& b, OpsVec_t& ops);
    static FB_STORAGE_API void shareOps(SharedBuffer& b, OpsVec_t& ops);

    // Adds the operation and returns the log serial number (LSN) of the operation.
    FB_STORAGE_API u32 addOp(const DbObject& op);

    // Returns the operation found by it's serial number.
    FB_STORAGE_API DbObject getOp(u32 lsn);

    // Update the operation log file as soon as possible.
    FB_STORAGE_API bool update();

    // Set operation log from either buffer.
    void setOpLog(Buffer& b)                { readOps(b, m_ops); }
    void setOpLog(SharedBuffer& b)          { shareOps(b, m_ops); }

private:
    const BuildImageInstallChunkBuildLayer& m_icBuildLayer;
    OpsVec_t m_ops;
};

struct DeployLayout
{
    FB_STORAGE_API void operator()(const DeployBuildImageLayout& layout, const BuildLayerRoot& root);
};
}