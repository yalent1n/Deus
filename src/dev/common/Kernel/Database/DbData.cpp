#include <Engine.Base/Database/DbData.h>
#include <Engine.Base/Memory/VirtualAlloc.h>
#include <cstring>

namespace rw
{
PropData c_emptyDbData;
	
PropData::PropData()
	: m_data(nullptr)
	, m_size(0)
	, m_policy(DatabaseOwnershipPolicy::Assume)
{}

PropData::PropData(const void* data, u32 size, DatabaseOwnershipPolicy policy)
	: m_data((u8*)const_cast<void*>(data))
	, m_size(size)
	, m_policy(policy)
{}

PropData::PropData(const PropData& other)
	: m_data(other.m_data)
	, m_size(other.m_size)
	, m_policy(DatabaseOwnershipPolicy::Shared)
{}
	
PropData::~PropData()
{
	if (!m_data)
		return;

	if (m_policy == DatabaseOwnershipPolicy::Shared)
		return;

	HeapFreeGlobal(m_data);	
}

PropData PropData::clone() const
{
	u8* newAddr = static_cast<u8*>(HeapAllocGlobal(m_size));
	std::memcpy(newAddr, m_data, m_size);
	return { newAddr, m_size, DatabaseOwnershipPolicy::Borrowed };
}

PropData& PropData::operator=(const PropData& rhs)
{
	m_data = rhs.m_data;
	m_size = rhs.m_size;
	m_policy = DatabaseOwnershipPolicy::Shared;
	return *this;
}
}
