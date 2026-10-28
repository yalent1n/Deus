#include <Engine.Storage/Pools/RemoteStoragePool.h>

namespace rw
{
RemoteStoragePool::RemoteStoragePool(const char* host)
    : m_StorageSession(host)
{
    EASSERT(m_StorageSession.handle(), "Failed to open session");
}

RemoteStoragePool::~RemoteStoragePool()
{

}

void RemoteStoragePool::updateConfig(const StoragePoolConfig& config)
{

}

// TODO: oldd meg valahogy az authentikaciot.
void RemoteStoragePool::flush(bool trim)
{
    FixedStringBuilder<> uri;
    uri << "/diskOps/save";

    HttpClientRequest req(m_StorageSession);
    (void) req.rpcOBJECT(uri.c_str(), TDBOBJ(
        "clean" << trim
    ));
}

DbObject RemoteStoragePool::describe()
{
    FixedStringBuilder<> uri;
    uri << "/disk";

    HttpClientRequest req(m_StorageSession);
    return req.rpcOBJECT(uri.c_str(), c_NullObject, HTTP_VERB_GET);
}

InsertResult RemoteStoragePool::putPatch(ContentId* manifestId, const ContentId& base, const SharedBuffer* delta)
{
    EASSERT(delta, "Null delta chunk");
    EASSERT(manifestId, "No target manifest ID");

    FixedStringBuilder<> uri;
    uri << "/disk/" << ContentIdStr(base).str << "/patch";

    HttpClientRequest req(m_StorageSession);
    SharedBuffer* buf = req.rpcPAYLOAD(uri.c_str(), delta);
    if(!buf)
    {
        return InsertResult::Error;
    }

    *manifestId = *(ContentId*)buf->bytes();
    return InsertResult::Inserted;
}

InsertResult RemoteStoragePool::putPatch(ContentId* patchContentID, const ContentId& base, const ContentId& delta)
{
    EASSERT(patchContentID, "No target manifest ID");

    FixedStringBuilder<> uri;
    uri << "/diskOps/patchadd";

    HttpClientRequest req(m_StorageSession);
    DbObject msg = req.rpcOBJECT(uri.c_str(), TDBOBJ(
        "base" << ContentIdStr(base).str <<
        "delta" << ContentIdStr(delta).str
    ));

    s32 err;
    if(msg["hint.error"].getValueSafe(err))
    {
        return (InsertResult) err;
    }

    FixedStringBuilder<> sPatchContentID;
    if(!msg["hint.patchContentID"].getValueSafe(sPatchContentID))
        return InsertResult::NoSourceSpecified;

    fromhexstr(sPatchContentID.c_str(), sPatchContentID.size(), patchContentID, sizeof(ContentId));
    return InsertResult::Inserted;
}

FetchResult RemoteStoragePool::getPatch(const ContentId& patchContentID, ContentId& base, ContentId& delta)
{
    FixedStringBuilder<> uri;
    uri << "/diskOps/patchinfo";

    HttpClientRequest req(m_StorageSession);
    DbObject msg = req.rpcOBJECT(uri.c_str(), TDBOBJ(
        "patchContentID" << ContentIdStr(patchContentID).str
    ));

    s32 error;
    if(msg["hint.error"].getValueSafe(error))
    {
        return (FetchResult) error;
    }

    FixedStringBuilder<> sBase;
    if(!msg["hint.base"].getValueSafe(sBase))
        return FetchResult::BaseNotFound;
        
    FixedStringBuilder<> sDelta;
    if(!msg["hint.delta"].getValueSafe(sDelta))
        return FetchResult::DeltaNotFound;

    fromhexstr(sBase.c_str(), sBase.size(), &base, sizeof(ContentId));
    fromhexstr(sDelta.c_str(), sDelta.size(), &delta, sizeof(ContentId));
    return FetchResult::Found;
}

InsertResult RemoteStoragePool::putChunk(const SharedBuffer* chunk)
{
    if(!chunk)
        return InsertResult::NoSourceSpecified;

    if(!chunk->size())
        return InsertResult::NoSourceSpecified;

    FixedStringBuilder<> uri;
    uri << "/disk";

    HttpClientRequest req(m_StorageSession);
    const SharedBuffer* res = req.rpcPAYLOAD(uri.c_str(), chunk);

    if(!res)
    {
        return InsertResult::NullFeedback;
    }

    const char* chars = (const char*) res->bytes();
    ELOGF("Remote storage: %s", chars);
    if(chars[0] == 'X')
    {
        s8 err;
        tointeger(chars + 1, err);
        return (InsertResult) err;
    }

    return InsertResult::Inserted;
}

FetchResult RemoteStoragePool::getChunk(const ContentId& pieceHash, SharedBuffer** out)
{
    if(!out)
        return FetchResult::NoTargetSpecified;

    FixedStringBuilder<> uri;
    uri << "/disk/" << ContentIdStr(pieceHash).str;

    HttpClientRequest req(m_StorageSession);
    *out = req.rpcPAYLOAD(uri.c_str(), NULL, HTTP_VERB_GET);

    const char* chars = (const char*) (*out)->bytes();
    if(!*out)
        return FetchResult::NotFound;

    if(chars[0] == 'X')
    {
        s8 err;
        tointeger(chars + 1, err);
        return (FetchResult) err;
    }

    return FetchResult::Found;
}

FetchResult RemoteStoragePool::fetchChunk(const ContentId& digest, void* buf, u32* size)
{
    return FetchResult::NotFound;
    // FixedStringBuilder<> uri;
    // uri << "/piece/";
    // uri << ContentIdStr(pieceId).str;

    // HttpClientRequest req(m_StorageSession);
    // return req.get(uri.c_str());
}

bool RemoteStoragePool::hasChunk(const ContentId& piece)
{
    return false;
    // FixedStringBuilder<> uri;
    // uri << "/piece/";
    // uri << ContentIdStr(pieceId).str;

    // HttpClientRequest req(m_StorageSession);
    // auto x = req.request(uri.c_str(), HTTP_VERB_GET);
    // x->
}

bool RemoteStoragePool::dropChunks(const ContentId* pieces, u32 count)
{
    FixedStringBuilder<> uri;
    uri << "/diskOps/drop";

    AllocatingBuffer buffer;
    for(u32 i = 0; i < count; i++)
    {
        buffer.write(pieces + i, sizeof(ContentId));
    }

    HttpClientRequest req(m_StorageSession);
    req.post(uri.c_str(), buffer.bytes(), buffer.length()) != nullptr;
}

void RemoteStoragePool::iterateChunks(ChunkCallback_t&& cb)
{

}

bool RemoteStoragePool::fullTrim()
{
    return true;
}
}