#pragma once
#include "DirectoryRecord.h"
#include <Engine.Network/Netcrypt.h>
#include <Engine.Base/Database/DbObjectBuilder.h>

namespace Deus
{
interface ekvm_Provider
{
    nodefine IANumber           thisIAN() pure;

    nodefine DbObject           readOBJECT(IANumber num) pure;
    nodefine IANumber           applyOBJECT(const DbObject& record) pure;
    nodefine bool               updateOBJECT(IANumber num, const DbObject& record) pure;

    nodefine SharedBuffer*      readDATA(IANumber num) pure;
    nodefine IANumber           applyDATA(const SharedBuffer* data) pure;
    nodefine bool               updateDATA(IANumber num, const SharedBuffer* data) pure;
};
}