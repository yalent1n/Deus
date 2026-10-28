#include <Engine.Reflection/RuntimeIpr.h>
#include <Engine.Reflection/RuntimeDatabasePartition.h>
#include <Engine.Base/HashSet.h>

namespace rw
{
struct RuntimeInterpartitionCodec
{
    enum : InterpartitionCodec { CompartmentBits = 8, PartitionBits = 28, InstanceBits = 27, FlagBit = 1 };

    u8 referringCompartment;
    u32 partitionId;
    u32 instanceId;

    InterpartitionReference pack() const
	{
		InterpartitionReference inter = { (static_cast<u64>(referringCompartment) << (64-CompartmentBits)) 
											| (static_cast<u64>(partitionId) << (InstanceBits+FlagBit))
											| (instanceId << FlagBit) 
											|  IsInterpartitionReferenceFlag };
		return inter;
	}

	void unpack(InterpartitionReference ref)
	{
		EASSERT(ref.id & IsInterpartitionReferenceFlag, "Inter-partition reference is invalid");

		referringCompartment = ref.id >> (64-CompartmentBits);
		partitionId = (ref.id << CompartmentBits) >> (CompartmentBits + InstanceBits+FlagBit);
		instanceId = (ref.id << (CompartmentBits+PartitionBits)) >> (CompartmentBits+PartitionBits + FlagBit);
	}
};

class GuidNamespace
{
public:
    GuidNamespace() = default;
    ~GuidNamespace() = default;

    u32 makeId(const Guid& guid)
    {
        u32& id = m_guids[guid];
        if(!id)
        {
            id = m_guids.size();
            m_reverseGuids[id] = guid;
        }
        
        return id;
    }

    const Guid& getGuid(u32 id) const
    {
        if(const auto it = m_reverseGuids.find(id); it != m_reverseGuids.end())
        {
            return it->second;
        }

        return c_emptyGuid;
    }

private:
    eastl::hash_map<Guid, u32> m_guids;
    eastl::hash_map<u32, Guid> m_reverseGuids;
};

struct InstanceGroupEntry
{
    const RuntimeDatabasePartition* referringPartition;
    GuidNamespace* instances;
};

class CompartmentNamespace
{
public:
    CompartmentNamespace(Compartment compartment)
        : m_compartment(compartment)
    {}

    ~CompartmentNamespace() = default;

    Compartment getCompartment() const
    {
        return m_compartment;
    }

    u32 makePartitionId(const RuntimeDatabasePartition* partition)
    {
        u32 partitionId = m_partitionNamespace.makeId(partition->getPartitionGuid());
        InstanceGroupEntry& entry = m_instancesNamespaces[partitionId];
        if(!entry.instances)
        {
            entry.referringPartition = partition;
            entry.instances = new GuidNamespace();
        }

        return partitionId;
    }

    const GuidNamespace& getPartitionNamespace() const
        { return m_partitionNamespace; }
    
    GuidNamespace* getNamespace(u32 partitionId)
        { return m_instancesNamespaces[partitionId].instances; }
    
    const RuntimeDatabasePartition* getPartition(u32 partitionId)
        { return m_instancesNamespaces[partitionId].referringPartition; }

private:
    Compartment m_compartment;
    GuidNamespace m_partitionNamespace;
    eastl::hash_map<u32, InstanceGroupEntry> m_instancesNamespaces;
};

static CompartmentNamespace* g_compartmentNamespaces[MAXCOMPARTMENTS];

void RuntimeIpr::staticInit()
{
    InterpartitionReference::make = make;
    InterpartitionReference::resolve = resolve;
    InterpartitionReference::resolveAndCache = resolveAndCache;
    InterpartitionReference::unresolve = unresolve;
}

Symbol* RuntimeIpr::resolve(InterpartitionReference inter)
{
    RuntimeInterpartitionCodec codec;
    codec.unpack(inter);
    
    CompartmentNamespace* partitions = g_compartmentNamespaces[codec.referringCompartment];
    if(!partitions)
        return nullptr;

    GuidNamespace* instances = partitions->getNamespace(codec.partitionId);
    if(!instances)
        return nullptr;

    const RuntimeDatabasePartition* partition = partitions->getPartition(codec.partitionId);
    if(!partition)
        return nullptr;

    const Guid& instanceGuid = instances->getGuid(codec.instanceId);
    return partition->findInstanceByGuid(instanceGuid);
}

Symbol* RuntimeIpr::resolveAndCache(InterpartitionReference inter)
{
    return nullptr;
}

void RuntimeIpr::unresolve(Symbol* symbol)
{
    
}

InterpartitionReference RuntimeIpr::make(Symbol* data)
{
    if(!data->isExported())
    {
        return {0};
    }

    RuntimeDatabasePartition* partition = nullptr;
    if(data->isAggregated())
    {
        partition = dynamic_cast<RuntimeDatabasePartition*>(data->getPartition());
    }

    Compartment namespaceId = data->getCompartment();

    CompartmentNamespace* partitions = nullptr;
    if(!g_compartmentNamespaces[namespaceId])
    {
        partitions = new CompartmentNamespace(namespaceId);
        g_compartmentNamespaces[namespaceId] = partitions;
    }

    RuntimeInterpartitionCodec codec;
    codec.referringCompartment = namespaceId;
    codec.partitionId = partitions->makePartitionId(partition);
    codec.instanceId = partitions->getNamespace(codec.partitionId)->makeId(*data->getInstanceGuid());
    return codec.pack();
}
}