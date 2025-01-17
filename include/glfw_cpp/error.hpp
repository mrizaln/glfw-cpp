#ifndef ERROR_HPP_32QWETF9D8
#define ERROR_HPP_32QWETF9D8

#include <stdexcept>
#include <format>

namespace glfw_cpp
{
    class Error : public std::runtime_error
    {
    public:
        template <typename... Args>
        Error(std::format_string<Args...> fmt, Args&&... args)
            : std::runtime_error{ std::format(fmt, std::forward<Args>(args)...) }
        {
        }
    };

    // GLFW error codes
    // ----------------

    class NotInitialized : public Error
    {
    public:
        NotInitialized()
            : Error{ "GLFW is not initialized" }
        {
        }
    };

    class NoCurrentContext : public Error
    {
    public:
        NoCurrentContext()
            : Error{ "No current OpenGL or OpenGL ES context" }
        {
        }
    };

    class OutOfMemory : public Error
    {
    public:
        OutOfMemory()
            : Error{ "A memory allocation failed" }
        {
        }
    };

    class ApiUnavailable : public Error
    {
    public:
        ApiUnavailable()
            : Error{ "The requested client API is unavailable" }
        {
        }
    };

    class VersionUnavailable : public Error
    {
    public:
        VersionUnavailable()
            : Error{ "The requested client API version is unavailable" }
        {
        }
    };

    class PlatformError : public Error
    {
    public:
        PlatformError()
            : Error{ "A platform-specific error occurred" }
        {
        }
    };

    class FormatUnavailable : public Error
    {
    public:
        FormatUnavailable()
            : Error{ "The requested format is unavailable" }
        {
        }
    };

    class NoWindowContext : public Error
    {
    public:
        NoWindowContext()
            : Error{ "The specified window does not have an OpenGL or OpenGL ES context" }
        {
        }
    };

    // ----------------

    // glfw-cpp errors
    // ---------------

    class AlreadyInitialized : public Error
    {
    public:
        AlreadyInitialized()
            : Error{ "Instance already initialized" }
        {
        }
    };

    class AlreadyBound : public Error
    {
    public:
        // std::formatter<std::thread::id> only available in C++23, damn it
        AlreadyBound(std::size_t current, std::size_t other)
            : Error{
                "The current context is already bound to another thread: current={}, other={}",
                current,
                other,
            }
        {
        }
    };

    class EmptyLoader : public Error
    {
    public:
        EmptyLoader()
            : Error{ "The OpenGL/OpenGL ES loader can't be empty" }
        {
        }
    };

    class WrongThreadAccess : public Error
    {
    public:
        WrongThreadAccess(std::size_t init, std::size_t current)
            : Error{

                "(WindowManager) Instance accessed from different thread from initialization! "
                "[init: {} | current: {}]",
                init,
                current,
            }
        {
        }
    };

    // ---------------
}

#endif /* end of include guard: ERROR_HPP_32QWETF9D8 */
