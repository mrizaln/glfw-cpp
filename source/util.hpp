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
        case GLFW_NOT_INITIALIZED:       throw glfw_cpp::NotInitialized      { err };
        case GLFW_NO_CURRENT_CONTEXT:    throw glfw_cpp::NoCurrentContext    { err };
        case GLFW_INVALID_ENUM:          throw glfw_cpp::InvalidEnum         { err };
        case GLFW_INVALID_VALUE:         throw glfw_cpp::InvalidValue        { err };
        case GLFW_OUT_OF_MEMORY:         throw glfw_cpp::OutOfMemory         { err };
        case GLFW_API_UNAVAILABLE:       throw glfw_cpp::ApiUnavailable      { err };
        case GLFW_VERSION_UNAVAILABLE:   throw glfw_cpp::VersionUnavailable  { err };
        case GLFW_PLATFORM_ERROR:        throw glfw_cpp::PlatformError       { err };
        case GLFW_FORMAT_UNAVAILABLE:    throw glfw_cpp::FormatUnavailable   { err };
        case GLFW_NO_WINDOW_CONTEXT:     throw glfw_cpp::NoWindowContext     { err };
        case GLFW_CURSOR_UNAVAILABLE:    throw glfw_cpp::CursorUnavailable   { err };
        case GLFW_FEATURE_UNAVAILABLE:   throw glfw_cpp::FeatureUnavailable  { err };
        case GLFW_FEATURE_UNIMPLEMENTED: throw glfw_cpp::FeatureUnimplemented{ err };
        case GLFW_PLATFORM_UNAVAILABLE:  throw glfw_cpp::PlatformUnavailable { err };
        default:                         throw glfw_cpp::UnknownError        { "(Unhandled GLFW error code={:#010x}) {}", err_code, err};
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
        case GLFW_NOT_INITIALIZED:       throw glfw_cpp::NotInitialized      { err };
        case GLFW_NO_CURRENT_CONTEXT:    throw glfw_cpp::NoCurrentContext    { err };
        case GLFW_INVALID_ENUM:          throw glfw_cpp::InvalidEnum         { err };
        case GLFW_INVALID_VALUE:         throw glfw_cpp::InvalidValue        { err };
        case GLFW_OUT_OF_MEMORY:         throw glfw_cpp::OutOfMemory         { err };
        case GLFW_API_UNAVAILABLE:       throw glfw_cpp::ApiUnavailable      { err };
        case GLFW_VERSION_UNAVAILABLE:   throw glfw_cpp::VersionUnavailable  { err };
        case GLFW_PLATFORM_ERROR:        throw glfw_cpp::PlatformError       { err };
        case GLFW_FORMAT_UNAVAILABLE:    throw glfw_cpp::FormatUnavailable   { err };
        case GLFW_NO_WINDOW_CONTEXT:     throw glfw_cpp::NoWindowContext     { err };
        case GLFW_CURSOR_UNAVAILABLE:    throw glfw_cpp::CursorUnavailable   { err };
        case GLFW_FEATURE_UNAVAILABLE:   throw glfw_cpp::FeatureUnavailable  { err };
        case GLFW_FEATURE_UNIMPLEMENTED: throw glfw_cpp::FeatureUnimplemented{ err };
        case GLFW_PLATFORM_UNAVAILABLE:  throw glfw_cpp::PlatformUnavailable { err };
        default:                         throw glfw_cpp::UnknownError        { "(Unhandled GLFW error code={:#010x}) {}", err_code, err};
        }
        // clang-format on

        assert(false && "Programmer mandated that error must occur, but no error occurred");
        std::terminate();
    }
}

#endif /* end of include guard: UTIL_HPP_SE5RTFRYHW6U */
