#pragma once
#include "Directory.h"

namespace Deus
{
class ekvm_Provider_object : public ekvm_Provider
{
public:
    SATURN_API ekvm_Provider_object();
    ~ekvm_Provider_object() = default;

    SATURN_API IANumber thisIAN() override;

    SATURN_API DbObject readOBJECT(IANumber num) override;
    SATURN_API IANumber applyOBJECT(const DbObject& record) override;
    SATURN_API bool updateOBJECT(IANumber num, const DbObject& record) override { return NO; }

    SATURN_API SharedBuffer* readDATA(IANumber num) { return NO; }
    SATURN_API IANumber applyDATA(const SharedBuffer* data) { return {}; }
    SATURN_API bool updateDATA(IANumber num, const SharedBuffer* data) { return NO; }
};
}