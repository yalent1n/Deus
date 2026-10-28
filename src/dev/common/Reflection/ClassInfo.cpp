#include <Engine.Reflection/ClassInfo.h>
#include <Engine.Reflection/SmartHandle.h>
#include <Engine.Reflection/ValueTypeInfo.h>
#include <Engine.Reflection/DatabasePartition.h>

namespace rw
{
ClassInfo::ClassInfo(const ClassInfoData* infoData)
    : TypeInfo(const_cast<TypeInfoData*>(&infoData->basic))
{
    // TODO: something class classinfo range
}

ClassInfo::~ClassInfo()
{
    // TODO: something class classinfo range
}

bool ClassInfo::isSubclassOf(const ClassInfo* classInfo) const
{
    const ClassInfo* iter = static_cast<const ClassInfo*>(this);
	const ClassInfo* other = static_cast<const ClassInfo*>(classInfo);

	const u32 clsId = iter->getClassId();
	const u32 otherClsId = other->getClassId();
	const u32 diff = clsId - otherClsId;

	// This is effectively a range compare, but only requires a single branch
	// due to the unsigned comparison. (Or in fact, no branch at all if the 
	// compiler optimizes properly -- VC9 does).
    return diff <= u32(other->getLastSubclassId() - otherClsId);
}

void ClassInfo::createRef(void* placement)
{
    if(CreateRefFun_t fn = getClassInfoData()->createRef)
        fn(placement);
}

void ClassInfo::setRef(void* placement, void* oP)
{
    if(SetRefFun_t fn = getClassInfoData()->setRef)
        fn(placement, oP);
}

void ClassInfo::destroyRef(void* placement)
{
    if(DestroyRefFun_t fn = getClassInfoData()->destroyRef)
        fn(placement);
}

void ClassInfo::resolveRef(void* placement)
{
    if(ResolveRefFun_t fn = getClassInfoData()->resolveRef)
        fn(placement);
}

Symbol* resolveClassRef(const void* placement)
{
    const InterpartitionReference* ref = (const InterpartitionReference*)(placement);
    if(ref->id & IsInterpartitionReferenceFlag)
    {
        // at least one lookup will be performed anyway
        return InterpartitionReference::resolveAndCache(*ref);
    }

    return (Symbol*)(const_cast<void*>(*(const void**)placement));
}

SkeletonNode* ClassInfo::toSkel(const void* data, const SkelConversionContext* context) const
{
    SkeletonNode* root = nullptr;
    SkeletonNode* node = nullptr;
    
    if(!data)
        return nullptr;

    StringConversionParams params;
    params.asMember = true;

    if(context->asMember)
    {
        StringBuilder<40> refStr;
        toString(data, &params, refStr);

        if(!refStr.isEmpty())
        {
            Symbol* symbol = resolveClassRef(data);
            if(!symbol)
                return nullptr;

            root = context->skel->createElement(context->asArray ? "member" : "field");
            root = context->root->appendChild(root);

            if(symbol->isExported())
            {
                root->setAttribute("ref", (new FormattedStringBuilder<40>("%s", GuidStr(*symbol->getInstanceGuid()).str))->c_str());
            }

            if(symbol->isAggregated())
            {

                root->setAttribute("uri", symbol->getPartition()->getName());
            }
        }
    }
    else
    {
        root = context->skel->createElement("instance");
        root = context->root->appendChild(root);

        const Symbol* symbol = (const Symbol*)data;
        if(symbol->isExported())
        {
            root->setAttribute("guid", (new FormattedStringBuilder<40>("%s", GuidStr(*symbol->getInstanceGuid()).str))->c_str());
        }

        const SymbolClassInfo* symbolType = symbol->getType();

        StringBuilder<> type;
        type << symbolType->getModule()->getName() << '.' << symbolType->getName();

        root->setAttribute("type", type.detach());
    
        Vec<const FieldInfoData*> fieldInfos;
        {
            const ClassInfo* iter = this;
            while(iter != iter->getBaseType())
            {
                const FieldInfoData* fields = iter->getFieldInfos();
                u32 fieldCount = iter->getFieldCount();
                for(u32 i = 0; i < fieldCount; i++)
                {
                    fieldInfos.push_back(&fields[i]);
                }
                iter = iter->getBaseType();
            }
        }
    
        for(auto i = fieldInfos.end() - 1, e = fieldInfos.begin(); i >= e; --i)
        {
            const FieldInfoData* fid = *i;
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
    }


    return root;
}

void ClassInfo::toString(const void* data, const StringConversionParams* params, StringEditor& result) const
{
    if(!data)
        return;

    if(params->asMember)
    {
        SymbolHandle<>* handle = (SymbolHandle<>*)data;
        if(Symbol* symbol = handle->get())
        {
            if(const Guid* instanceGuid = symbol->getInstanceGuid())
            {
                result << GuidStr(*instanceGuid).str;
            }
            else
            {
                result << "null";
            }
        }
        else
        {
            result << "null";
        }

    }
    else
    {
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
}