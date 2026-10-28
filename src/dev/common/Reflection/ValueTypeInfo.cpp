#include <Engine.Reflection/ValueTypeInfo.h>
#include <Engine.Reflection/SmartHandle.h>

namespace rw
{
SkeletonNode* ValueTypeInfo::toSkel(const void* data, const SkelConversionContext* context) const
{
    SkeletonNode* root = nullptr;
    SkeletonNode* node = nullptr;
    
    if(!data)
        return nullptr;

    root = context->skel->createElement("complex");
    root = context->root->appendChild(root);

    const FieldInfoData* fieldInfos = getFieldInfos();
    u32 fieldInfoCount = getFieldCount();

    for(u32 i = 0; i < fieldInfoCount; i++)
    {
        const FieldInfoData* fid = fieldInfos + i;
        const void* fieldValue = ((const u8*)data) + fid->fieldOffset;

        SkelConversionContext subcontext;
        subcontext.skel = context->skel;
        subcontext.root = root;
        subcontext.asMember = true;

        node = fid->fieldType->toSkel(fieldValue, &subcontext);
        if(!node)
            continue;

        if(isnil(node->getAttribute("name")))
            node->setAttribute("name", fid->member.name, AddTo::Front);
    }

    return root;
}

void ValueTypeInfo::toString(const void* data, const StringConversionParams* params, StringEditor& result) const
{
    if(!data)
        return;

    const FieldInfoData* fieldInfos = getFieldInfos();
    u32 fieldInfoCount = getFieldCount();

    result.appendChars("{ ");

    for(u32 i = 0; i < fieldInfoCount; i++)
    {
        const FieldInfoData* fid = fieldInfos + i;
        const void* fieldValue = ((const u8*)data) + fid->fieldOffset;
        
        StringConversionParams subparams;
        subparams.asMember = true;
        fid->fieldType->toString(fieldValue, &subparams, result);

        if(i != fieldInfoCount - 1)
            result.appendChar(',');

        result.appendChar(' ');
    }

    result.appendChar('}');
}
}