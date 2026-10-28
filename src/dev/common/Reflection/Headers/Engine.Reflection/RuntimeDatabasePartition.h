#pragma once
#include "DatabasePartition.h"

// TODO: move this into new module ContentDatabase or existing Storage
namespace rw
{
class RuntimeDatabasePartition : public DatabasePartition
{
public:
    FB_REFL_API RuntimeDatabasePartition(const PartitionInitData& initData);
    FB_REFL_API ~RuntimeDatabasePartition() override;

    FB_REFL_API const char* getName() const override;
    FB_REFL_API const Guid& getPartitionGuid() const override;
    FB_REFL_API ExportMode getExportMode() const override;

    FB_REFL_API void addPartitionInstance(Symbol* symbol) override;
    FB_REFL_API void removePartitionInstance(Symbol* symbol) override;

    FB_REFL_API const Symbol* getPrimaryInstance() const override;
    FB_REFL_API Symbol* getPrimaryInstance() override;

    FB_REFL_API Symbol* findInstanceByGuid(const Guid& instanceGuid) const override;
    FB_REFL_API void getPartitionInstances(PartitionInstances_t& result) override;

private:
    const char* m_name;
    Guid m_guid;
    PartitionInstances_t m_instances;
    PartitionInstanceMap_t m_exportedInstances;
    ExportMode m_exportMode;
};
}