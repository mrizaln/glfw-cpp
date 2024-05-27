#ifndef UTIL_HPP_SE5RTFRYHW6U
#define UTIL_HPP_SE5RTFRYHW6U

#include <cstddef>
#include <mutex>
#include <thread>

namespace util
{
    inline std::size_t getThreadNum(const std::thread::id& threadId)
    {
        auto hash{ std::hash<std::thread::id>{} };
        return hash(threadId);
    }

    template <typename... Fs>
    struct VisitOverloaded : Fs...
    {
        using Fs::operator()...;
    };

    template <typename T, typename M>
        requires requires(M m) {
            m.lock();
            m.unlock();
        }
    T lockExchange(M& mutex, T& value, T&& newValue)
    {
        std::scoped_lock lock{ mutex };

        T oldValue = std::move(value);
        value      = std::forward<T>(newValue);
        return oldValue;
    }
}

#endif /* end of include guard: UTIL_HPP_SE5RTFRYHW6U */
