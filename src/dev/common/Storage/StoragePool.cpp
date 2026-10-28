#include <Engine.Storage/StoragePool.h>

namespace rw
{
StoragePool::StoragePool()
    : m_config()
{}

StoragePool::~StoragePool()
{}

StorageFormat toStorageFormat(const char* format)
{
    if(equalsIgnoreCase("file", format))
        return StorageFormat::File;
    else if(equalsIgnoreCase("remote", format))
        return StorageFormat::Remote;
    else if(equalsIgnoreCase("flexible", format))
        return StorageFormat::Flexible;
    else if(equalsIgnoreCase("fixed", format))
        return StorageFormat::Fixed;

    return StorageFormat::Flexible;
}
}