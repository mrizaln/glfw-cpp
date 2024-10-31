#ifndef UTIL_HPP_SE5RTFRYHW6U
#define UTIL_HPP_SE5RTFRYHW6U

#include "glfw_cpp/error.hpp"
#include <cassert>
#include <stdexcept>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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

    inline void checkGlfwError()
    {
        int error = glfwGetError(nullptr);

        // clang-format off
        switch (error) {
        case GLFW_NO_ERROR:             break;
        case GLFW_NOT_INITIALIZED:      throw glfw_cpp::NotInitialized{};
        case GLFW_NO_CURRENT_CONTEXT:   throw glfw_cpp::NoCurrentContext{};
        case GLFW_OUT_OF_MEMORY:        throw glfw_cpp::OutOfMemory{};
        case GLFW_API_UNAVAILABLE:      throw glfw_cpp::ApiUnavailable{};
        case GLFW_VERSION_UNAVAILABLE:  throw glfw_cpp::VersionUnavailable{};
        case GLFW_PLATFORM_ERROR:       throw glfw_cpp::PlatformError{};
        case GLFW_FORMAT_UNAVAILABLE:   throw glfw_cpp::FormatUnavailable{};
        case GLFW_NO_WINDOW_CONTEXT:    throw glfw_cpp::NoWindowContext{};

        default: throw std::runtime_error{ std::format(
                "Unknown error: {}. Might be a bug. Report to developer.",
                error
            ) };
        }
        // clang-format on
    }

    [[noreturn]] inline void throwGlfwError()
    {
        int error = glfwGetError(nullptr);

        // clang-format off
        switch (error) {
        case GLFW_NO_ERROR:             break;
        case GLFW_NOT_INITIALIZED:      throw glfw_cpp::NotInitialized{};
        case GLFW_NO_CURRENT_CONTEXT:   throw glfw_cpp::NoCurrentContext{};
        case GLFW_OUT_OF_MEMORY:        throw glfw_cpp::OutOfMemory{};
        case GLFW_API_UNAVAILABLE:      throw glfw_cpp::ApiUnavailable{};
        case GLFW_VERSION_UNAVAILABLE:  throw glfw_cpp::VersionUnavailable{};
        case GLFW_PLATFORM_ERROR:       throw glfw_cpp::PlatformError{};
        case GLFW_FORMAT_UNAVAILABLE:   throw glfw_cpp::FormatUnavailable{};
        case GLFW_NO_WINDOW_CONTEXT:    throw glfw_cpp::NoWindowContext{};

        default: throw std::runtime_error{ std::format(
                "Unknown error: {}. Might be a bug. Report to developer.",
                error
            ) };
        }
        // clang-format on

        assert(false && "Programmer mandated that error must occur, but no error occurred");
        std::terminate();
    }
}

#endif /* end of include guard: UTIL_HPP_SE5RTFRYHW6U */
