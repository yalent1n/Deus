#include <Engine.Core/Backends/ZipFsBackend.h>

namespace rw
{
Buffer* ZipIoBackend::touch(const char* path, BIOACCESS)
{
    
}

Buffer* ZipIoBackend::openFile(const char* path, bool)
{

}

Buffer* ZipIoBackend::createFile(const char* path, bool append, bool read)
{

}

SharedBuffer* ZipIoBackend::shareFile(const char* path, bool)
{

}

bool ZipIoBackend::deleteFile(const char* path) const
{

}

bool ZipIoBackend::fileExists(const char* path) const
{

}

bool ZipIoBackend::createDirectory(const char* dirPath) const
{

}

bool ZipIoBackend::deleteDirectory(const char* dirPath) const
{

}

FileIterator* ZipIoBackend::getFileIterator(const char* dirPath) const
{

}

}