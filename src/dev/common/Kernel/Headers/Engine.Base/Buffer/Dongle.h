#pragma once
#include "Buffer.h"

namespace rw
{
// An interface for disk IO.
class Dongle : public Buffer
{
public:
    FB_CORE_API Dongle();
    FB_CORE_API ~Dongle();

private:
    void* mHandle;
};
}