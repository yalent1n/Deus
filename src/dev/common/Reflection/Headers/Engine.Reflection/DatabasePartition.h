#pragma once
#include "Symbol.h"

namespace rw
{
typedef Vec<Symbol*> PartitionInstances_t;
typedef Dict<Guid, Symbol*> PartitionInstanceMap_t;

enum ExportMode
{
    /* Only the primary instance exported */
    ExportMode_Limited,

    /* Every instance is exported */
    ExportMode_All
};

// TODO: Create Array<> (only a pointer to elements) and RefArray<> for correct symbol management
interface DatabasePartition
{
    virtual ~DatabasePartition() = default;

    virtual const char* getName() const = 0;
    virtual const Guid& getPartitionGuid() const = 0;
    virtual ExportMode getExportMode() const = 0;

    virtual void addPartitionInstance(Symbol *symbol) = 0;
    virtual void removePartitionInstance(Symbol *symbol) = 0;

    virtual const Symbol* getPrimaryInstance() const = 0;
    virtual Symbol* getPrimaryInstance() = 0;

    virtual Symbol* findInstanceByGuid(const Guid& instanceGuid) const = 0;
    virtual void getPartitionInstances(PartitionInstances_t& result) = 0;
};

struct PartitionInitData
{
    const char* name;
    Guid guid;
    //PartitionInstances_t instances;
};
}