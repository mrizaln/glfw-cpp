#ifndef INSTANCE_HPP_AO39EW8FOEW
#define INSTANCE_HPP_AO39EW8FOEW

#if defined(_WIN32)
#    undef ERROR
#endif

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

            int         m_major         = 1;
            int         m_minor         = 0;
            Profile     m_profile       = Profile::CORE;    // only makes sense for OpenGL 3.2 above
            bool        m_forwardCompat = true;             // only makes sense for OpenGL 3.0 above
            GlLoaderFun m_loader;
        };

        struct NoApi
        {
        };

        using Variant = std::variant<OpenGL, OpenGLES, NoApi>;
    };

    // Instance is a singleton that manages the global state required to interface with GLFW
    class Instance
    {
    public:
        friend WindowManager;
        friend Window;

        enum class LogLevel
        {
            NONE,
            DEBUG,
            INFO,
            WARNING,
            ERROR,
            CRITICAL,
        };

        using LogFun = std::function<void(LogLevel level, std::string msg)>;
        using Handle = std::unique_ptr<Instance, void (*)(Instance*)>;

        friend Handle init(Api::Variant&&, Instance::LogFun&&);

        WindowManager createWindowManager();

        ~Instance();
        Instance& operator=(Instance&&)      = delete;
        Instance(Instance&&)                 = delete;
        Instance(const Instance&)            = delete;
        Instance& operator=(const Instance&) = delete;

    private:
        static Instance s_instance;

        bool         m_initialized = false;
        Api::Variant m_api;
        LogFun       m_loggger;

        Instance() = default;

        void setLogger(LogFun&& logger);
        void reset();

        static Instance& get();

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
    inline Instance Instance::s_instance = {};

    // Initialize GLFW and returns a RAII handle that will terminate GLFW on destruction
    Instance::Handle init(Api::Variant&& api, Instance::LogFun&& logger = nullptr);
}

#endif /* end of include guard: INSTANCE_HPP_AO39EW8FOEW */
