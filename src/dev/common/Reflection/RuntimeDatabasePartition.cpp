#include <Engine.Reflection/RuntimeDatabasePartition.h>

namespace rw
{
struct InstanceComparer
{
    inline bool operator()(const Symbol* lhs, const Symbol* rhs)
    {
        return lhs == rhs;
    }
};

RuntimeDatabasePartition::RuntimeDatabasePartition(const PartitionInitData& initData)
    : m_name(initData.name)
    , m_guid(initData.guid)
{
    // if(!initData.instances.empty())
    // {
    //     m_instances.insert_range(m_instances.end(), initData.instances.begin(), initData.instances.end());
    // }

    ELOGF("Created partition %s", initData.name);
}

RuntimeDatabasePartition::~RuntimeDatabasePartition()
{
    ELOGF("Removed partition %s", m_name);

    for(Symbol* symbol : m_instances)
    {
        removePartitionInstance(symbol);
    }
}

const char* RuntimeDatabasePartition::getName() const
{
    return m_name;
}

const Guid& RuntimeDatabasePartition::getPartitionGuid() const
{
    return m_guid;
}

ExportMode RuntimeDatabasePartition::getExportMode() const
{
    return m_exportMode;
}

void RuntimeDatabasePartition::addPartitionInstance(Symbol* symbol)
{
    EASSERT(symbol, "Cannot add null instance");

    // symbol is already in a partition
    if(symbol->isAggregated())
        return;

    if(symbol->isExported())
    {
        if(m_exportedInstances.find(*symbol->getInstanceGuid()) != m_exportedInstances.end())
            return;
    }

    // weak comparison
    if(m_instances.find(symbol, InstanceComparer()) != m_instances.end())
        return;

    m_instances.push_back(symbol);
    
    if(symbol->isExported())
        m_exportedInstances.insert(*symbol->getInstanceGuid(), symbol);

    symbol->retain();
    symbol->setAggregated(true);
    symbol->setPartition(this);
}

void RuntimeDatabasePartition::removePartitionInstance(Symbol* symbol)
{
    EASSERT(symbol, "Cannot remove null instance");

    // symbol is already in a partition
    if(!symbol->isAggregated())
        return;

    PartitionInstances_t::iterator instIt = m_instances.find(symbol, InstanceComparer());
    // weak comparison
    if(instIt != m_instances.end())
        return;
        
    if(symbol->isExported())
    {
        m_exportedInstances.erase(*symbol->getInstanceGuid());
    }

    m_instances.erase(instIt);

    symbol->setPartition(nullptr);
    symbol->setAggregated(false);
    symbol->detach();
}

const Symbol* RuntimeDatabasePartition::getPrimaryInstance() const
{
    if(!m_instances.empty())
    {
        return m_instances[0];
    }

    return nullptr;
}

Symbol* RuntimeDatabasePartition::getPrimaryInstance()
{
    if(!m_instances.empty())
    {
        return m_instances[0];
    }

    return nullptr;
}

Symbol* RuntimeDatabasePartition::findInstanceByGuid(const Guid& instanceGuid) const
{
    if(const PartitionInstanceMap_t::iterator it = m_exportedInstances.find(instanceGuid); it != m_exportedInstances.end())
    {
        return it->second;
    }

    return nullptr;
}

void RuntimeDatabasePartition::getPartitionInstances(PartitionInstances_t& result)
{
    result.insert_range(result.end(), m_instances.begin(), m_instances.end());
}
}