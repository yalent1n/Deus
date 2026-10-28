#pragma once
#include "../Bundle.h"
#include <Engine.Base/Database/DbOid.h>
#include <Engine.Base/Database/DbObject.h>
#include <Engine.ToolCore/App/ServiceBase.h>
#include <Engine.Network/TcpServerConnection.h>

namespace rw
{
/*
    Handles sessions with RPC and
    and holds an ephemeral whiteboard (key-value store).

    Main goal is to hold local/remote processes together.
    This way multiple applications can have access to eachother.

    RPC body:
        rpc <array> {
            channel <string>
            event <table>
        }

        event <table> {
            eve <string>
            msg <string>            
            callerId <oid>
            targetId <oid>
        }
*/
class EnsembleService : public ServiceBase
{
public:    
    FB_STORAGE_API  EnsembleService();
    FB_STORAGE_API ~EnsembleService();

    const DbOid& ensembleId() const;
    const DbOid& sessionId() const;

    // ----
    // RPC handling
    void handleRpc(const DbObject& rpc);

private:
    DbOid m_ensembleId;
    DbOid m_sessionId;

};
}