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
    inline std::size_t get_thread_num(const std::thread::id& thread_id)
    {
        auto hash = std::hash<std::thread::id>{};
        return hash(thread_id);
    }

    template <typename... Fs>
    struct VisitOverloaded : Fs...
    {
        using Fs::operator()...;
    };

    template <typename T, typename M>
        requires requires (M m) {
            m.lock();
            m.unlock();
        }
    T lock_exchange(M& mutex, T& value, T&& new_value)
    {
        auto lock = std::scoped_lock{ mutex };

        T old_value = std::move(value);
        value       = std::forward<T>(new_value);

        return old_value;
    }

    inline void check_glfw_error()
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

    [[noreturn]] inline void throw_glfw_error()
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
