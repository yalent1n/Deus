#include <Engine.Reflection/EbxSharedTypeDescriptorTable.h>

namespace rw
{
EbxSharedTypeDescriptorTable::EbxSharedTypeDescriptorTable()
{
    EDEBUGF("Shared types initialized");
}

EbxSharedTypeDescriptorTable::~EbxSharedTypeDescriptorTable()
{
    EDEBUGF("Shared types deinitialized");
}
}