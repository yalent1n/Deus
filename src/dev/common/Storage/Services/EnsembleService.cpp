#include <Engine.Storage/Services/EnsembleService.h>

namespace rw
{
EnsembleService::EnsembleService()
{
    m_ensembleId.generate();
    m_sessionId.generate();
}

EnsembleService::~EnsembleService()
{
    m_ensembleId = DbOid::c_zero;
    m_sessionId = DbOid::c_zero;
}

const DbOid& EnsembleService::ensembleId() const
{
    return m_ensembleId;
}

const DbOid& EnsembleService::sessionId() const
{
    return m_sessionId;
}

void EnsembleService::handleRpc(const DbObject& rpc)
{
    DbOid callerId = rpc["callerId"].valueOid();    
    DbOid targetId = rpc["targetId"].valueOid();
    DbObjectArray msgs = rpc["msg"].valueArray();
    const char* eve = msgs[0].valueString();
    const char* msg = msgs[1].valueString();

    // // broadcasting
    // if(targetId == m_ensembleId)
    // {

    // }
}
}