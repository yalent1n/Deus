#include "DiskManagementService.h"
#include <Engine.Core/diag/ExecutionContext.h>
#include <Engine.Storage/Delta.h>

namespace rw
{
constexpr s32 CONTENTID_MAX_LEN = sizeof(ContentId) * 2 + 1;

DiskManagementService::DiskManagementService(u16 port, DiskType diskType, const char* root)
    : m_Threads("diskmgmt")
    , m_Http(port, &m_Threads, YES)
{
    switch(diskType)
    {
        default:
            break;

        case DiskType::File:
            m_Disk = new FileStoragePool(ExecutionContext::getVirtualFileSystem(), root, "Virtual Disk Mamagement");
            break;
        case DiskType::Flexible:
            m_Disk = new CasStoragePool(ExecutionContext::getVirtualFileSystem(), root, "Virtual Disk Mamagement");
            break;
    }

    initializeServiceRouters();
}

DiskManagementService::~DiskManagementService()
{

}

void DiskManagementService::initializeServiceRouters()
{
    HttpRouter* diskOps = m_Http.addRouter("/diskOps/(op)");
    diskOps->setAction(HTTP_VERB_POST, makeDelegate(this, &DiskManagementService::opsDISK));
    
    HttpRouter* diskUploader = m_Http.addRouter("/disk");
    diskUploader->setAction(HTTP_VERB_POST, makeDelegate(this, &DiskManagementService::postDISK));

    HttpRouter* diskPatcher = m_Http.addRouter("/disk/(base)/patch");
    diskPatcher->setAction(HTTP_VERB_POST, makeDelegate(this, &DiskManagementService::patchDISK));

    HttpRouter* diskDownloader = m_Http.addRouter("/disk/(piece)");
    diskDownloader->setAction(HTTP_VERB_GET, makeDelegate(this, &DiskManagementService::fetchDISK));

    diskOps->setFormType            (Mime_Json);
    diskOps->setFormCodec           (CodecMethod_None);

    diskUploader->setFormType       (Mime_OctetStream);
    diskUploader->setFormCodec      (CodecMethod_None);

    diskDownloader->setFormType     (Mime_OctetStream);
    diskDownloader->setFormCodec    (CodecMethod_None);

    diskPatcher->setFormType     (Mime_OctetStream);
    diskPatcher->setFormCodec    (CodecMethod_None);
}

HttpStatus DiskManagementService::postDISK(const HttpRequest& req, HttpResponse& res)
{
    if(!req.content_length)
    {
        return res.status(HTTP_NOT_ACCEPTABLE);
    }

    StringBuilder<CONTENTID_MAX_LEN + 1> payload;
    InsertResult err = m_Disk->putChunk(new MemorySharedBuffer(req.content, req.content_length));
    if(err >= InsertResult::Error)
    {
        payload << 'X';
        tostring((s32)err, payload);
        return res.content(payload.detach()).status(HTTP_BAD_REQUEST);
    }

    PieceHash piece;
    piece.calculate(req.content, req.content_length);
    tohexstr(&piece, sizeof(piece), payload);

    ELOGF("Disk: uploaded piece %s", payload.c_str());
    return res.content(payload.detach()).status(HTTP_OK);
}

HttpStatus DiskManagementService::opsDISK(const HttpRequest& req, HttpResponse& res)
{
    FixedStringBuilder<16> op;
    if(!req.params["op"].getValueSafe(op))
        return res.body(TDBOBJ("status" << "error" << "hint" << op.c_str())).status(HTTP_BAD_REQUEST);

    ELOGF("Disk: completing %s operation", op.c_str());

    bool completed = true;
    DbObject hint = c_NullObject;
    if(equalsIgnoreCase("save", op.c_str()))
    {
        bool clean = false;
        req.body["clean"].getValueSafe(clean);
        m_Disk->flush(clean);
    }
    else if(equalsIgnoreCase("shrink", op.c_str()))
    {
        completed = m_Disk->getFullTrim()->fullTrim();
    }
    else if(equalsIgnoreCase("info", op.c_str()))
    {
        hint = std::move(m_Disk->describe());
    }
    else if(equalsIgnoreCase("patchadd", op.c_str()))
    {
        FixedStringBuilder<CONTENTID_MAX_LEN + 1> sBase;
        if(!req.body["base"].getValueSafe(sBase)) {
            hint = TDBOBJ("hint" << "missing base");
            completed = false;
        }
            
        FixedStringBuilder<CONTENTID_MAX_LEN + 1> sDelta;
        if(!req.body["delta"].getValueSafe(sDelta)) {
            hint = TDBOBJ("hint" << "missing delta");
            completed = false;
        }

        if(completed)
        {
            ContentId base;
            fromhexstr(sBase.c_str(), sBase.size(), &base, sizeof(base));
    
            ContentId delta;
            fromhexstr(sDelta.c_str(), sDelta.size(), &delta, sizeof(delta));
    
            ContentId patch;
            InsertResult err = m_Disk->putPatch(&patch, base, delta);
            if(err < InsertResult::Error)
            {
                hint = TDBOBJ("patchContentID" << ContentIdStr(patch).str);
            }
            else
            {
                hint = TDBOBJ("err" << (s32) err);
                completed = false;
            }
        }
        
    }
    else if(equalsIgnoreCase("patchinfo", op.c_str()))
    {
        FixedStringBuilder<CONTENTID_MAX_LEN + 1> sPatchContentID;
        if(!req.body["patchContentID"].getValueSafe(sPatchContentID)) {
            hint = TDBOBJ("hint" << "missing patchContentID");
            completed = false;
        }

        if(completed)
        {
            ContentId patchContentID;
            ContentId base;
            ContentId delta;
    
            fromhexstr(sPatchContentID.c_str(), sPatchContentID.size(), &patchContentID, sizeof(patchContentID));
            FetchResult err = m_Disk->getPatch(patchContentID, base, delta);
            if(err < FetchResult::Found)
            {
                hint = TDBOBJ("error" << (s32) err);
                completed = false;
            }
            else
            {
                hint = TDBOBJ("base" << ContentIdStr(base).str << "delta" << ContentIdStr(delta).str);
            }
        }
    }
    else if(equalsIgnoreCase("cataloginfo", op.c_str()))
    {
        DbObjectArrayBuilder pieces;
        m_Disk->iterateChunks([&](const ContentId& contentId)
        {
            pieces << ContentIdStr(contentId).str;
        });

        DbObjectBuilder catalogInfo;
        catalogInfo << "pieces" << pieces.done();

        hint = catalogInfo.done();
    }

    return res.body(DBOBJ("status" << (completed ? "completed" : "error") << "hint" << hint)).status(completed ? HTTP_OK : HTTP_BAD_REQUEST);
}

HttpStatus DiskManagementService::fetchDISK(const HttpRequest& req, HttpResponse& res)
{
    FetchResult err;

    FixedStringBuilder<> sPiece;
    if(!req.params["piece"].getValueSafe(sPiece))
    {
        return res.status(HTTP_BAD_REQUEST);
    }

    PieceHash piece;
    fromhexstr(sPiece.c_str(), sPiece.size(), &piece, sizeof(piece));

    StringBuilder<> tmp;
    tohexstr(&piece, sizeof(piece), tmp);

    SharedBuffer* pieceBuffer;
    err = m_Disk->getChunk(piece, &pieceBuffer);
    if(err < FetchResult::Found)
    {
        StringBuilder<> errString;
        errString << 'X';
        tostring((s32) err, errString);

        return res.header("X-Canopy-Exception", errString.detach()).status(HTTP_BAD_REQUEST);
    }

    ELOGF("Disk: fetched piece %s", tmp.c_str());
    return res.content((const char*) pieceBuffer->bytes(), pieceBuffer->size64()).status(HTTP_OK);
}

HttpStatus DiskManagementService::patchDISK(const HttpRequest& req, HttpResponse& res)
{
    InsertResult err;

    FixedStringBuilder<> sBase;
    if(!req.params["base"].getValueSafe(sBase))
    {
        return res.status(HTTP_BAD_REQUEST);
    }

    StringBuilder<CONTENTID_MAX_LEN + 1> payload;
    
    PieceHash base;
    fromhexstr(sBase.c_str(), sBase.size(), &base, sizeof(base));
    
    ContentId patch;
    err = m_Disk->putPatch(&patch, base, new MemorySharedBuffer(req.content, req.content_length));
    if(err >= InsertResult::Error)
    {
        payload << 'X';
        tostring((s32)err, payload);
        return res.content(payload.detach()).status(HTTP_INTERNAL_SERVER_ERROR);
    }
    
    ELOGF("Disk: new patch %s", ContentIdStr(patch).str);
    tohexstr(&patch, sizeof(patch), payload);

    return res.content(payload.detach()).status(HTTP_OK);
}
}