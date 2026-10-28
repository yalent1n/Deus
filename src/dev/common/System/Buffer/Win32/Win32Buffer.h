#pragma once
#include <Engine.Base/Buffer/Buffer.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace rw
{
class Win32Buffer : public Buffer
{
public:
    Win32Buffer();
    Win32Buffer(BIOACCESS, const char*);
    ~Win32Buffer();

    bool IsOpened() const;

    bool Open(const char*);
    bool Close();

    void flush() override;
    void seek(size_t, BIOSEEKDIR) override;

    s64 getAvailableBytes() const override;
    s64 getPosition() const override;
    void setPosition(s64 pos) override;

    BIORETURN write(const void*, size_t) override;
    BIORETURN writeAsync(const void*, size_t) override;
    BIORETURN read(void*, size_t) override;
    BIORETURN readAsync(void*, size_t) override;

private:
    HANDLE mHandle;
};
}