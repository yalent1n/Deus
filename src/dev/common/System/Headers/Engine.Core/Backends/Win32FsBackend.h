#pragma once
#include "../FsBackend.h"

namespace rw
{
CORE_API EXTERN_C bool NtCreateDirectoryA(const char*);

static inline const u8 s_win32SealingKey[32] = {
    0xA7, 0x3C, 0x91, 0xE4, 0x5F, 0x2B, 0xD8, 0x6A,
    0x19, 0xC0, 0x77, 0x8E, 0xF3, 0x42, 0xAD, 0x5C,
    0xE9, 0x10, 0x6F, 0xB2, 0x88, 0x34, 0xCA, 0xD1,
    0x7B, 0xF6, 0x25, 0x9A, 0x04, 0xEE, 0x63, 0x18
};

class WindowsFS : public FsBackend
{
public:
    CORE_API WindowsFS();
    CORE_API WindowsFS(StringFn_t&&);
    CORE_API WindowsFS(const char*);
    
    CORE_API bool authenticate(const char* domain, const char* name, const char* password) override;
    CORE_API Buffer* touch(const char*, BIOACCESS = E_BIO_ALWAYS_CREATE) override;
    CORE_API Buffer* openFile(const char*, bool check = true) override;
    CORE_API Buffer* createFile(const char*, bool append = true, bool read = true) override;
    CORE_API SharedBuffer* shareFile(const char*, bool check = false) override;
    CORE_API bool deleteFile(const char*) const override;    
    CORE_API bool copyFile(const char*, const char*) override;
    CORE_API bool moveFile(const char*, const char*) override;
    CORE_API bool sealFile(const char*) override;
    CORE_API bool unsealFile(const char*) override;
    CORE_API bool fileExists(const char*) const override;    
    CORE_API bool createDirectory(const char*) const override;
    CORE_API bool deleteDirectory(const char*) const override;
    CORE_API FileIterator* getFileIterator(const char*) const override;
};
}