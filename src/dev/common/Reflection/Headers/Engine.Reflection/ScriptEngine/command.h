#pragma once
#include "instructions.h"

namespace rw
{
struct scrThread;
struct scrCommandInfo
{
    scrThread* Thread;
};

typedef void(*scrCommand)(const scrCommandInfo&);
}