#ifndef UTIL_HPP_SE5RTFRYHW6U
#define UTIL_HPP_SE5RTFRYHW6U

#include "glfw_cpp/error.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
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
        const char* err      = nullptr;
        const int   err_code = glfwGetError(&err);

        // clang-format off
        switch (err_code) {
        case GLFW_NO_ERROR:              break;
        case GLFW_NOT_INITIALIZED:       throw glfw_cpp::error::NotInitialized      { err };
        case GLFW_NO_CURRENT_CONTEXT:    throw glfw_cpp::error::NoCurrentContext    { err };
        case GLFW_INVALID_ENUM:          throw glfw_cpp::error::InvalidEnum         { err };
        case GLFW_INVALID_VALUE:         throw glfw_cpp::error::InvalidValue        { err };
        case GLFW_OUT_OF_MEMORY:         throw glfw_cpp::error::OutOfMemory         { err };
        case GLFW_API_UNAVAILABLE:       throw glfw_cpp::error::ApiUnavailable      { err };
        case GLFW_VERSION_UNAVAILABLE:   throw glfw_cpp::error::VersionUnavailable  { err };
        case GLFW_PLATFORM_ERROR:        throw glfw_cpp::error::PlatformError       { err };
        case GLFW_FORMAT_UNAVAILABLE:    throw glfw_cpp::error::FormatUnavailable   { err };
        case GLFW_NO_WINDOW_CONTEXT:     throw glfw_cpp::error::NoWindowContext     { err };
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 4
        case GLFW_CURSOR_UNAVAILABLE:    throw glfw_cpp::error::CursorUnavailable   { err };
        case GLFW_FEATURE_UNAVAILABLE:   throw glfw_cpp::error::FeatureUnavailable  { err };
        case GLFW_FEATURE_UNIMPLEMENTED: throw glfw_cpp::error::FeatureUnimplemented{ err };
        case GLFW_PLATFORM_UNAVAILABLE:  throw glfw_cpp::error::PlatformUnavailable { err };
#endif
        default:                         throw glfw_cpp::error::UnknownError        { "(Unhandled GLFW error code={:#010x}) {}", err_code, err};
        }
        // clang-format on
    }

    [[noreturn]] inline void throw_glfw_error()
    {
        const char* err      = nullptr;
        const int   err_code = glfwGetError(&err);

        // clang-format off
        switch (err_code) {
        case GLFW_NO_ERROR:              break;
        case GLFW_NOT_INITIALIZED:       throw glfw_cpp::error::NotInitialized      { err };
        case GLFW_NO_CURRENT_CONTEXT:    throw glfw_cpp::error::NoCurrentContext    { err };
        case GLFW_INVALID_ENUM:          throw glfw_cpp::error::InvalidEnum         { err };
        case GLFW_INVALID_VALUE:         throw glfw_cpp::error::InvalidValue        { err };
        case GLFW_OUT_OF_MEMORY:         throw glfw_cpp::error::OutOfMemory         { err };
        case GLFW_API_UNAVAILABLE:       throw glfw_cpp::error::ApiUnavailable      { err };
        case GLFW_VERSION_UNAVAILABLE:   throw glfw_cpp::error::VersionUnavailable  { err };
        case GLFW_PLATFORM_ERROR:        throw glfw_cpp::error::PlatformError       { err };
        case GLFW_FORMAT_UNAVAILABLE:    throw glfw_cpp::error::FormatUnavailable   { err };
        case GLFW_NO_WINDOW_CONTEXT:     throw glfw_cpp::error::NoWindowContext     { err };
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 4
        case GLFW_CURSOR_UNAVAILABLE:    throw glfw_cpp::error::CursorUnavailable   { err };
        case GLFW_FEATURE_UNAVAILABLE:   throw glfw_cpp::error::FeatureUnavailable  { err };
        case GLFW_FEATURE_UNIMPLEMENTED: throw glfw_cpp::error::FeatureUnimplemented{ err };
        case GLFW_PLATFORM_UNAVAILABLE:  throw glfw_cpp::error::PlatformUnavailable { err };
#endif
        default:                         throw glfw_cpp::error::UnknownError        { "(Unhandled GLFW error code={:#010x}) {}", err_code, err};
        }
        // clang-format on

        assert(false && "Programmer mandated that error must occur, but no error occurred");
        std::terminate();
    }
}

#endif /* end of include guard: UTIL_HPP_SE5RTFRYHW6U */
