#include <Engine.Network/aidl/frame.h>
#include <Engine.Base/Buffer/Adapter.h>

namespace rw
{
aidl_frame* aidl_frame_new(const aidl_tags_t &tags)
{
    AllocatingBuffer result;
    aidl_frame* frame = (aidl_frame*) result.allocateWrite(sizeof(aidl_frame));

    for(aidl_tags_t::const_iterator i = tags.begin(), e = tags.end(); i < e; ++i)
    {
        result.write(*i, (*i)->length);
    }

    frame->magic = AIDL_FRAME_MAGIC;
    frame->tag_count = tags.size();
    frame->length = (u16)(result.length() - sizeof(aidl_frame));

    return (aidl_frame*) result.detach();
}

aidl_frame* aidl_frame_free(aidl_frame* frame)
{
    if(!frame)
        return nullptr;

    if(!HeapFreeGlobal(frame))
        return nullptr;

    return frame;
}
}