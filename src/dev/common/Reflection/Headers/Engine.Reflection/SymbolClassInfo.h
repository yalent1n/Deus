#pragma once
#include "ClassInfo.h"
#include "Compartment.h"

namespace rw
{
interface DatabasePartition;
class Symbol;

class SymbolClassInfo : public ClassInfo
{
    friend class Symbol;

public:
    struct CreateInfo
    {
        Compartment compartment;
        bool makeRoomForGuid;
        bool makeRoomForPartition;
        bool makeUniqueGuid;
    };

private:
    static inline const int c_partitionEntryIndex = 1;
    struct PartitionEntry
    {
        DatabasePartition* partition;
    };

    static inline const int c_guidEntryIndex = 3;
    struct GuidEntry
    {
        Guid guid;
    };

public:
    SymbolClassInfo(const ClassInfoData* infoData)
        : ClassInfo(infoData)
    {}

    ~SymbolClassInfo() = default;

    FB_REFL_API Object* createInstance() const;
    FB_REFL_API Object* createInstance(const CreateInfo& createInfo) const;

private:
    void initializeSymbol(Symbol* symbol, const CreateInfo* createInfo) const;
    void deinitializeSymbol(Symbol* symbol) const;
};
}
