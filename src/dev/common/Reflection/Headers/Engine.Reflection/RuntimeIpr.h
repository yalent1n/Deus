#pragma once
#include "SmartHandle.h"

namespace rw
{
/*
NOTE: For runtime we need a fast lookup table with as few allocations as possible.
Runtime won't use AssetIds, it has to be something easily indexed.
*/
struct RuntimeIpr
{
    static FB_REFL_API void     staticInit();

    static FB_REFL_API Symbol*  resolve(InterpartitionReference);
    static FB_REFL_API Symbol*  resolveAndCache(InterpartitionReference);
    static FB_REFL_API void     unresolve(Symbol*);

    static FB_REFL_API InterpartitionReference make(Symbol* data);
};
}