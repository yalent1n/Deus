#pragma once
#include "../FsBackend.h"

namespace rw
{
class ZipIoBackend : public FsBackend
{
public:
    // ----
    // File functions
    Buffer* touch(const char*, BIOACCESS = E_BIO_ALWAYS_CREATE) override;
    Buffer* openFile(const char*, bool check = true) override;
    Buffer* createFile(const char*, bool append = false, bool read = true) override;
    SharedBuffer* shareFile(const char*, bool check = true) override;
    bool deleteFile(const char*) const override;
    bool fileExists(const char*) const override;

    // ----
    // Directory functions
    bool createDirectory(const char*) const override;
    bool deleteDirectory(const char*) const override;
    FileIterator* getFileIterator(const char*) const override;
};
}