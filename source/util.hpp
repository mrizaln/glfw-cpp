#ifndef UTIL_HPP_SE5RTFRYHW6U
#define UTIL_HPP_SE5RTFRYHW6U

#include <cstddef>
#include <thread>

namespace util
{
    inline std::size_t getThreadNum(const std::thread::id& threadId)
    {
        auto hash{ std::hash<std::thread::id>{} };
        return hash(threadId);
    }
}

#endif /* end of include guard: UTIL_HPP_SE5RTFRYHW6U */
