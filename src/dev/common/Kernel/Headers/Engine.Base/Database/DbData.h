#pragma once
#include "../Bundle.h"

namespace rw
{
enum class DatabaseOwnershipPolicy : u8
{
	Assume,
	Shared,
	Borrowed
};
	
class PropData
{
public:
	FB_BASE_API PropData();
	FB_BASE_API PropData(const void* data, u32 size, DatabaseOwnershipPolicy policy);
	FB_BASE_API PropData(const PropData& other);
	FB_BASE_API ~PropData();
	
	u8* data()							const { return m_data; }
	u32 size()							const { return m_size; }
	DatabaseOwnershipPolicy policy()	const { return m_policy; }

	FB_BASE_API PropData				clone() const;
	FB_BASE_API PropData&				operator=(const PropData& rhs);

private:
	u8* 						m_data;
	u32					    	m_size;
	DatabaseOwnershipPolicy		m_policy;
};

extern FB_BASE_API PropData c_emptyDbData;
}