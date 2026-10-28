#include <Engine.Storage/Deployment/Deploy.h>
#include <Engine.Base/Database/DbObjectBuilder.h>

namespace rw
{
DeployBuildImageOpLog::DeployBuildImageOpLog(BuildImageLayout& layout, const BuildImageInstallChunkBuildLayer& icbl)
    : BuildImageOpLog(layout)
    , m_icBuildLayer(icbl)
    , m_ops(true)
{}

void DeployBuildImageOpLog::readOps(Buffer& b, OpsVec_t& ops)
{
    OpsLogHeader info;
    b.read(&info, sizeof info);

    if(info.magic != OpsLogHeader::MAGIC)
        return;

    for(u32 i = 0; i < info.opsCount; i++)
    {
        u32 opLength = b.readUint();
        u8* opData = static_cast<u8*>(HeapAllocGlobal(opLength));
        if(b.read(opData, opLength) < E_BIO_SUCCESS)
            return;

        ops.push_back(DbObject(opData, DatabaseOwnershipPolicy::Shared));
    }
}

void DeployBuildImageOpLog::shareOps(SharedBuffer& b, OpsVec_t& ops)
{
    u8* data = static_cast<u8*>(b.bytes());

    if(!data)
        return;

    const OpsLogHeader& info = *(const OpsLogHeader*)data;
    if(info.magic != OpsLogHeader::MAGIC)
        return;

    data += sizeof info;
    for(u32 i = 0; i < info.opsCount; i++)
    {
        u32 opLength = *(u32*)data;
        data += sizeof opLength;

        u8* opData = data;
        ops.push_back(DbObject(opData, DatabaseOwnershipPolicy::Shared));
    }
}

u32 DeployBuildImageOpLog::addOp(const DbObject& op)
{
    if(op.isNullOrEmpty())
        return c_invalidSerialNo;

    // we don't allow an operation that's already added (because at the next step we rebuild the op entry)
    u32 lsn = op["serialNo"].valueInteger(c_invalidSerialNo);
    if(lsn != c_invalidSerialNo)
        return lsn;

    lsn = m_ops.size() + 1;
    ELOGF("New operation %u", lsn);

    DbObjectBuilder newOp;
    newOp << "serialNo" << lsn;
    newOp.appendElements(op);

    DbObjectScatter scatter = newOp.doneScatter();
    new (&m_ops.push_back()) DbObject(scatter.data);
    return lsn;
}

DbObject DeployBuildImageOpLog::getOp(u32 lsn)
{
    if(lsn == c_invalidSerialNo)
        return DbObject();

    // map index
    --lsn;
    if(lsn >= m_ops.size())
        return DbObject();

    return m_ops[lsn];
}

bool DeployBuildImageOpLog::update()
{
    const auto& installRoot = getIcBuildLayer().getRoot();
    const auto& installChunk = getIcBuildLayer().getInstallChunk();
    const char* installBundle = installChunk.getInstallBundle();
    if(isnil(installBundle))
        return false;

    FormattedStringBuilder<260> logPath("%s/ops.log", installBundle);
    ScopedRef logFile(installRoot.createFile(BuildLayerFileFullPath(installRoot, c_str(logPath))));
    if(logFile.isNull())
        return false;

    OpsLogHeader info;
    info.magic = OpsLogHeader::MAGIC;
    info.opsCount = m_ops.size();
    logFile->write(&info, sizeof info);
    for(const DbObject& op : m_ops)
    {
        logFile->write(op.dataSize());
        logFile->write(op.data(), op.dataSize());
        ELOGF("Appended to op log with size %uB", op.dataSize());
    }

    ELOGF("Updated op log - %s", c_str(logPath));
    return true;
}

void DeployLayout::operator()(const DeployBuildImageLayout& layout, const BuildLayerRoot& root)
{
    // Build super bundle layout
    DbObjectArrayBuilder superBundles;
    for(const BuildImageSuperbundle* sb : layout.getSuperbundles())
    {
        superBundles << TDBOBJ(
            "name" << sb->getName()
        );
    }

    const auto& manifest = layout.getInstallManifest<DeployBuildImageInstallManifest>();

    // Build manifest for installs

    DbObjectArrayBuilder installGroupsList;
    for(const auto& igroup : manifest.getInstallGroups())
    {
        DbObjectBuilder spec;
        spec << "name" << igroup->name.c_str();

        DbObjectArrayBuilder chunks;
        for(const BuildImageInstallChunk* ichunk : igroup->chunks)
        {
            chunks << ichunk->getGuid();
        }

        spec << "chunks" << chunks.done();
    }

    DbObjectArrayBuilder installChunks;
    for(const auto& ichunk : manifest.getInstallChunks())
    {
        DbObjectBuilder spec;
        spec << "id" << ichunk->getGuid();
        spec << "name" << ichunk->getCatalogName();
        spec << "installBundle" << ichunk->getInstallBundle();

        DbObjectArrayBuilder icSuperBundles;
        for(const BuildImageSuperbundle* sb : ichunk->getSuperbundles())
        {
            icSuperBundles << sb->getName();
        }

        spec << "superbundles" << icSuperBundles.done();

        installChunks << spec.done();
    }

    DbObject installManifest = DBOBJ(
        "installChunks" << installChunks.done()
    );

    DbObject result = DBOBJ(
        "superBundles" << superBundles.done() <<
        "installManifest" << installManifest
    );

    JsonConversionParams convParams;
    convParams.indentRoots = true;
    convParams.indentChildren = true;

    StringBuilder<> json;
    result.toJsonString(json, convParams);

    BuildLayerFileLayout layoutFile(root);

    ScopedRef jsonLayout(root.createFile(layoutFile));
    jsonLayout->write(c_str(json), json.size());

    root.writeDbObject(layoutFile, result);
    root.sealFile(layoutFile);
}
}
