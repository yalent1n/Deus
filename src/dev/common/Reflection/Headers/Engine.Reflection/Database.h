#pragma once
#include "Bundle.h"

namespace rw
{
class DatabaseManager
{
public:
    virtual ~DatabaseManager() = default;

    const char* getDatabaseId() const
        { return m_databaseId.c_str(); }

    void setDatabaseId(const char* databaseId)
        { m_databaseId = databaseId; }

private:
    eastl::string m_databaseId;
};
}