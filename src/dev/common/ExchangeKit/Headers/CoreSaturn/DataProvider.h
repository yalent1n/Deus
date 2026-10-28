#pragma once
#include "Directory.h"

namespace Deus
{
class ekvm_Provider_data : public ekvm_Provider
{
public:
    SATURN_API ekvm_Provider_data();
    ~ekvm_Provider_data() = default;

    SATURN_API IANumber thisIAN() override;

    SATURN_API DbObject readOBJECT(IANumber num) { return rw::DbObject(); }
    SATURN_API IANumber applyOBJECT(const DbObject& record) { return IANumber(); }
    SATURN_API bool updateOBJECT(IANumber num, const DbObject& record) override { return NO; }

    SATURN_API SharedBuffer* readDATA(IANumber num) override;
    SATURN_API IANumber applyDATA(const SharedBuffer* data) override;
    SATURN_API bool updateDATA(IANumber num, const SharedBuffer* data) override { return NO; }
};
}