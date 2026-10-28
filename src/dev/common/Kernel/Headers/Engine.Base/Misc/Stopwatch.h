#pragma onced
#include "Time.h"

namespace rw
{
class Stopwatch
{
public:
    Stopwatch()
        : m_timestamp(Time::unixNow())
    {}

    s64 getElapsedMs()          const { return Time::unixNow() - m_timestamp; }
    void reset()                { m_timestamp = Time::unixNow(); }
    
private:
    Timestamp m_timestamp;
};
}