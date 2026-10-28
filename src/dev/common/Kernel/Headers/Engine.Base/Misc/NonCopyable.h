#pragma once

namespace rw
{
class NonCopyable
{
private:
    NonCopyable() = delete;
    NonCopyable(const NonCopyable&) = delete;
};
}