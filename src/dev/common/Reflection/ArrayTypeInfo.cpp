#include <Engine.Reflection/ArrayTypeInfo.h>
#include <Engine.Reflection/ArrayBase.h>

namespace rw
{
void ArrayTypeInfo::setElementCount(void* placement, uint newCount)
{
    if(SetElementCountFun_t fun = getArrayTypeInfoData()->setElementCount; fun)
        fun(placement, newCount);
}

void ArrayTypeInfo::releaseElements(void* placement)
{
    if(ReleaseElementsFun_t fun = getArrayTypeInfoData()->releaseElements; fun)
        fun(placement);
}

SkeletonNode* ArrayTypeInfo::toSkel(const void* data, const SkelConversionContext* context) const
{
    SkeletonNode* root;
    SkeletonNode* node;

    const u8* bytes = (const u8*)(*(const void**)data);
    u32 count = ((const u32*)bytes)[array_helper::kEntrySize];
    if(!count)
        return nullptr;

    root = context->skel->createElement("array");
    root = context->root->appendChild(root);

    const TypeInfo* elementType = getElementType();

    const u8* elementPtr = bytes;
    u16 stride = elementType->getMemberSize();
    for(u32 i = 0; i < count; i++)
    {
        SkelConversionContext subcontext;
        subcontext.skel = context->skel;
        subcontext.root = root;
        subcontext.asMember = true;
        subcontext.asArray = true;

        node = elementType->toSkel(elementPtr, &subcontext);
        elementPtr += stride;
    }

    return root;
}

void ArrayTypeInfo::toString(const void* data, const StringConversionParams* params, StringEditor& result) const
{

}
}