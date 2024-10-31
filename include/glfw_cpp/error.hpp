#ifndef ERROR_HPP_32QWETF9D8
#define ERROR_HPP_32QWETF9D8

#include <stdexcept>
#include <format>

namespace glfw_cpp
{
    // GLFW error codes
    // -------------------------------------------------------------------------

    class NotInitialized : public std::runtime_error
    {
    public:
        NotInitialized()
            : std::runtime_error{ "GLFW is not initialized" }
        {
        }
    };

    class NoCurrentContext : public std::runtime_error
    {
    public:
        NoCurrentContext()
            : std::runtime_error{ "No current OpenGL or OpenGL ES context" }
        {
        }
    };

    class OutOfMemory : public std::runtime_error
    {
    public:
        OutOfMemory()
            : std::runtime_error{ "A memory allocation failed" }
        {
        }
    };

    class ApiUnavailable : public std::runtime_error
    {
    public:
        ApiUnavailable()
            : std::runtime_error{ "The requested client API is unavailable" }
        {
        }
    };

    class VersionUnavailable : public std::runtime_error
    {
    public:
        VersionUnavailable()
            : std::runtime_error{ "The requested client API version is unavailable" }
        {
        }
    };

    class PlatformError : public std::runtime_error
    {
    public:
        PlatformError()
            : std::runtime_error{ "A platform-specific error occurred" }
        {
        }
    };

    class FormatUnavailable : public std::runtime_error
    {
    public:
        FormatUnavailable()
            : std::runtime_error{ "The requested format is unavailable" }
        {
        }
    };

    class NoWindowContext : public std::runtime_error
    {
    public:
        NoWindowContext()
            : std::runtime_error{ "The specified window does not have an OpenGL or OpenGL ES context" }
        {
        }
    };

    // glfw-cpp errors
    // -------------------------------------------------------------------------

    class AlreadyInitialized : public std::runtime_error
    {
    public:
        AlreadyInitialized()
            : std::runtime_error{ "Instance already initialized" }
        {
        }
    };

    class AlreadyBound : public std::runtime_error
    {
    public:
        // std::formatter<std::thread::id> only available in C++23, damn it
        AlreadyBound(std::size_t current, std::size_t other)
            : std::runtime_error{ std::format(
                  "The current context is already bound to another thread: current={}, other={}",
                  current,
                  other
              ) }
        {
        }
    };

    class EmptyLoader : public std::runtime_error
    {
    public:
        EmptyLoader()
            : std::runtime_error{ "The OpenGL/OpenGL ES loader can't be empty" }
        {
        }
    };

    class WrongThreadAccess : public std::runtime_error
    {
    public:
        WrongThreadAccess(std::size_t init, std::size_t current)
            : std::runtime_error{
                std::format(
                    "(WindowManager) Instance accessed from different thread from initialization! "
                    "[at init: {} | current: {}]",
                    init,
                    current
                ),
            }
        {
        }
    };
}

#endif /* end of include guard: ERROR_HPP_32QWETF9D8 */
