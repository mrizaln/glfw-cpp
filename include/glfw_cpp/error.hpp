#ifndef ERROR_HPP_32QWETF9D8
#define ERROR_HPP_32QWETF9D8

#include <format>
#include <stdexcept>

namespace glfw_cpp
{
    enum class ErrorCode : int
    {
        // glfw error codes (these ones are from the GLFW library itself)
        // docs: https://www.glfw.org/docs/latest/group__errors.html
        NotInitialized       = 0x00010001,
        NoCurrentContext     = 0x00010002,
        InvalidEnum          = 0x00010003,
        InvalidValue         = 0x00010004,
        OutOfMemory          = 0x00010005,
        ApiUnavailable       = 0x00010006,
        VersionUnavailable   = 0x00010007,
        PlatformError        = 0x00010008,
        FormatUnavailable    = 0x00010009,
        NoWindowContext      = 0x0001000A,
        CursorUnavailable    = 0x0001000B,
        FeatureUnavailable   = 0x0001000C,
        FeatureUnimplemented = 0x0001000D,
        PlatformUnavailable  = 0x0001000E,

        // glfw-cpp error codes (these ones I made up since they are not from the GLFW library)
        AlreadyInitialized = 0x00020001,
        AlreadyBound       = 0x00020002,
        EmptyLoader        = 0x00020003,
        WrongThreadAccess  = 0x00020004,
        UnknownError       = 0x000FFFFF,
    };

    constexpr std::string_view to_string(ErrorCode code) noexcept
    {
        // clang-format off
        switch (code) {
        case ErrorCode::NotInitialized:       return "NotInitialized";
        case ErrorCode::NoCurrentContext:     return "NoCurrentContext";
        case ErrorCode::InvalidEnum:          return "InvalidEnum";
        case ErrorCode::InvalidValue:         return "InvalidValue";
        case ErrorCode::OutOfMemory:          return "OutOfMemory";
        case ErrorCode::ApiUnavailable:       return "ApiUnavailable";
        case ErrorCode::VersionUnavailable:   return "VersionUnavailable";
        case ErrorCode::PlatformError:        return "PlatformError";
        case ErrorCode::FormatUnavailable:    return "FormatUnavailable";
        case ErrorCode::NoWindowContext:      return "NoWindowContext";
        case ErrorCode::CursorUnavailable:    return "CursorUnavailable";
        case ErrorCode::FeatureUnavailable:   return "FeatureUnavailable";
        case ErrorCode::FeatureUnimplemented: return "FeatureUnimplemented";
        case ErrorCode::PlatformUnavailable:  return "PlatformUnavailable";
        case ErrorCode::AlreadyInitialized:   return "AlreadyInitialized";
        case ErrorCode::AlreadyBound:         return "AlreadyBound";
        case ErrorCode::WrongThreadAccess:    return "WrongThreadAccess";
        case ErrorCode::EmptyLoader:          return "EmptyLoader";
        case ErrorCode::UnknownError:         [[fallthrough]];
        default:                              return "UnknownError";
        }
        // clang-format on
    }

    class Error : public std::runtime_error
    {
    public:
        template <typename... Args>
        Error(ErrorCode code, std::format_string<Args...> fmt, Args&&... args)
            : std::runtime_error{ std::format(
                  "[{:#010x}] {}",
                  static_cast<int>(code),
                  std::format(fmt, std::forward<Args>(args)...)
              ) }
        {
        }

        ErrorCode code() const noexcept { return m_code; }

    private:
        ErrorCode m_code;
    };

    // GLFW error codes
    // ----------------

    class NotInitialized : public Error
    {
    public:
        NotInitialized(const char* description)
            : Error{ ErrorCode::NotInitialized, " GLFW is not initialized | {}", description }
        {
        }
    };

    class NoCurrentContext : public Error
    {
    public:
        NoCurrentContext(const char* description)
            : Error{ ErrorCode::NoCurrentContext, "No current OpenGL or OpenGL ES context | {}", description }
        {
        }
    };

    // realistically this will never be thrown so it's not needed, but for completeness sake, here it is
    class InvalidEnum : public Error
    {
    public:
        InvalidEnum(const char* description)
            : Error{ ErrorCode::InvalidEnum,
                     "An invalid enum value was passed to a function | {}",
                     description }
        {
        }
    };

    class InvalidValue : public Error
    {
    public:
        InvalidValue(const char* description)
            : Error{ ErrorCode::InvalidValue, "An invalid value was passed to a function | {}", description }
        {
        }
    };

    class OutOfMemory : public Error
    {
    public:
        OutOfMemory(const char* description)
            : Error{ ErrorCode::OutOfMemory, "A memory allocation failed | {}", description }
        {
        }
    };

    class ApiUnavailable : public Error
    {
    public:
        ApiUnavailable(const char* description)
            : Error{ ErrorCode::ApiUnavailable, "The requested client API is unavailable | {}", description }
        {
        }
    };

    class VersionUnavailable : public Error
    {
    public:
        VersionUnavailable(const char* description)
            : Error{ ErrorCode::VersionUnavailable,
                     "The requested client API version is unavailable | {}",
                     description }
        {
        }
    };

    class PlatformError : public Error
    {
    public:
        PlatformError(const char* description)
            : Error{ ErrorCode::PlatformError, "A platform-specific error occurred | {}", description }
        {
        }
    };

    class FormatUnavailable : public Error
    {
    public:
        FormatUnavailable(const char* description)
            : Error{ ErrorCode::FormatUnavailable, "The requested format is unavailable | {}", description }
        {
        }
    };

    class NoWindowContext : public Error
    {
    public:
        NoWindowContext(const char* description)
            : Error{ ErrorCode::NoWindowContext,
                     "The specified window does not have an OpenGL or OpenGL ES context | {}",
                     description }
        {
        }
    };

    class CursorUnavailable : public Error
    {
    public:
        CursorUnavailable(const char* description)
            : Error{ ErrorCode::CursorUnavailable,
                     "The specified cursor shape is not available | {}",
                     description }
        {
        }
    };

    class FeatureUnavailable : public Error
    {
    public:
        FeatureUnavailable(const char* description)
            : Error{ ErrorCode::FeatureUnavailable,
                     "The requested feature is not provided by the platform | {}",
                     description }
        {
        }
    };

    class FeatureUnimplemented : public Error
    {
    public:
        FeatureUnimplemented(const char* description)
            : Error{ ErrorCode::FeatureUnimplemented,
                     "The requested feature is not implemented for the platform | {}",
                     description }
        {
        }
    };

    class PlatformUnavailable : public Error
    {
    public:
        PlatformUnavailable(const char* description)
            : Error{ ErrorCode::PlatformUnavailable,
                     "Platform unavailable or no matching platform was found | {}",
                     description }
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
            : Error{ ErrorCode::AlreadyInitialized, "Instance already initialized" }
        {
        }
    };

    class AlreadyBound : public Error
    {
    public:
        // std::formatter<std::thread::id> only available in C++23, damn it
        AlreadyBound(std::size_t current, std::size_t other)
            : Error{ ErrorCode::AlreadyBound,
                     "The current context is already bound to another thread: current={}, other={}",
                     current,
                     other }
        {
        }
    };

    class EmptyLoader : public Error
    {
    public:
        EmptyLoader()
            : Error{ ErrorCode::EmptyLoader, "The OpenGL/OpenGL ES loader can't be empty" }
        {
        }
    };

    class WrongThreadAccess : public Error
    {
    public:
        WrongThreadAccess(std::size_t init, std::size_t current)
            : Error{ ErrorCode::WrongThreadAccess,
                     "(WindowManager) Instance accessed from different thread from initialization! "
                     "[init: {} | current: {}]",
                     init,
                     current }
        {
        }
    };

    class UnknownError : public Error
    {
    public:
        template <typename... Args>
        UnknownError(std::format_string<Args...> fmt, Args&&... args)
            : Error{ ErrorCode::UnknownError,
                     "Unknown error, might be a bug | {}",
                     std::format(fmt, std::forward<Args>(args)...) }
        {
        }
    };

    // ---------------
}

#endif /* end of include guard: ERROR_HPP_32QWETF9D8 */
