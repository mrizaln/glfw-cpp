#ifndef CONTEXT_HPP_AO39EW8FOEW
#define CONTEXT_HPP_AO39EW8FOEW

#include <format>
#include <functional>
#include <memory>
#include <string>
#include <variant>

struct GLFWwindow;

namespace glfw_cpp
{
    class Window;
    class WindowManager;

    struct Api
    {
        using GlProc      = void (*)();
        using GlGetProc   = GlProc(const char*);
        using GlContext   = ::GLFWwindow*;
        using GlLoaderFun = std::function<void(GlContext handle, GlGetProc proc)>;

        struct OpenGLES
        {
            int         m_major = 2;
            int         m_minor = 0;
            GlLoaderFun m_loader;
        };

        struct OpenGL
        {
            enum class Profile
            {
                CORE,
                COMPAT,
                ANY,
            };

            int         m_major   = 3;
            int         m_minor   = 3;
            Profile     m_profile = Profile::CORE;
            GlLoaderFun m_loader;
        };

        struct NoApi
        {
        };

        using Variant = std::variant<OpenGL, OpenGLES, NoApi>;
    };

    // Context is a singleton that manages the global state required to interface with GLFW
    class Context
    {
    public:
        friend WindowManager;
        friend Window;

        enum class LogLevel
        {
            NONE,
            CRITICAL,
            ERROR,
            WARNING,
            INFO,
            DEBUG,
        };

        using LogFun = std::function<void(LogLevel level, std::string msg)>;
        using Handle = std::unique_ptr<Context, void (*)(Context*)>;

        friend Handle init(Api::Variant&&, Context::LogFun&&);

        WindowManager createWindowManager();

        ~Context();
        Context& operator=(Context&&)      = delete;
        Context(Context&&)                 = delete;
        Context(const Context&)            = delete;
        Context& operator=(const Context&) = delete;

    private:
        static Context s_instance;

        bool         m_initialized = false;
        Api::Variant m_api;
        LogFun       m_loggger;

        Context() = default;

        void setLogger(LogFun&& logger);
        void reset();

        static Context& get();

        // can be called from any thread
        static void log(LogLevel level, std::string msg);

        template <typename... Args>
        static void logD(std::format_string<Args...> fmt, Args&&... args)
        {
            log(LogLevel::DEBUG, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void logI(std::format_string<Args...> fmt, Args&&... args)
        {
            log(LogLevel::INFO, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void logW(std::format_string<Args...> fmt, Args&&... args)
        {
            log(LogLevel::WARNING, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void logE(std::format_string<Args...> fmt, Args&&... args)
        {
            log(LogLevel::ERROR, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void logC(std::format_string<Args...> fmt, Args&&... args)
        {
            log(LogLevel::CRITICAL, std::format(fmt, std::forward<Args>(args)...));
        }
    };

    // Global state required to interface with GLFW
    inline Context Context::s_instance = {};

    // Initialize GLFW and returns a RAII handle that will terminate GLFW on destruction
    Context::Handle init(Api::Variant&& api, Context::LogFun&& logger = nullptr);

    WindowManager createWindowManager();
}

#endif /* end of include guard: CONTEXT_HPP_AO39EW8FOEW */
