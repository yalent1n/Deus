#include <Engine.Reflection/SymbolClassInfo.h>
#include <Engine.Reflection/Symbol.h>

namespace rw
{
Object* SymbolClassInfo::createInstance() const
{
    CreateInfo info;
    info.compartment = Compartment_STATIC;
    info.makeRoomForPartition = false;
    info.makeRoomForGuid = false;
    info.makeUniqueGuid = false;
    return createInstance(info);
}

Object* SymbolClassInfo::createInstance(const CreateInfo& createInfo) const
{
    size_t instanceSize = getInstanceSize();
    size_t instanceOffset = 0;

    if(createInfo.makeRoomForGuid)
    {
        instanceSize += sizeof(GuidEntry);
        instanceOffset += sizeof(GuidEntry);
    }

    if(createInfo.makeRoomForPartition | createInfo.makeRoomForGuid)
    {
        instanceSize += sizeof(PartitionEntry);
        instanceOffset += sizeof(PartitionEntry);
    }

    ResourceCompartmentHandle chandle = ResourceCompartment::getHandle(createInfo.compartment);
    if(!chandle)
    {
        return nullptr;
    }

    CtorFun_t ctor = getTypeInfoData()->ctorFun;
    if(!ctor) // probably abstract
    {
        return nullptr;
    }

    ResourceCompartmentAllocator callocator = ResourceCompartment::getAllocator(chandle);
    void* placement = HeapAlloc(callocator, instanceSize, getInstanceAlignment());
    if(!placement)
        return nullptr;

    Symbol* symbol = (Symbol*)(((u8*)placement) + instanceOffset);
    ctor(symbol);

    initializeSymbol(symbol, &createInfo);
    return symbol;
}

void SymbolClassInfo::initializeSymbol(Symbol* symbol, const CreateInfo* createInfo) const
{
    symbol->m_compartment = createInfo->compartment;
    symbol->m_typeInfo = this;
    
    // NOTE: it's ok to have room for guid, but only mark it as exported if we generate it before,
    // otherwise partition will assign a guid.
    if(createInfo->makeUniqueGuid)
    {
        symbol->setExported(true);
        symbol->setInstanceGuid(genFastGuid());
    }
}

void SymbolClassInfo::deinitializeSymbol(Symbol* symbol) const
{
    // TODO: remove from interpool cache for example.

    if(DtorFun_t fun = getTypeInfoData()->dtorFun; fun)
    {
        fun(symbol);
    }

    symbol->setUnresolvable(true);

    u8* basePtr = (u8*) symbol;
    if(symbol->isAggregated() | symbol->isExported())
        basePtr -= sizeof(PartitionEntry);
        
    if(symbol->isExported())
        basePtr -= sizeof(GuidEntry);

    ResourceCompartmentHandle chandle = ResourceCompartment::getHandle(symbol->getCompartment());
    if(!chandle)
    {
        return;
    }

    ResourceCompartmentAllocator callocator = ResourceCompartment::getAllocator(chandle);
    HeapFree(callocator, basePtr);
}
}