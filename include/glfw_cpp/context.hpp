#ifndef CONTEXT_HPP_AO39EW8FOEW
#define CONTEXT_HPP_AO39EW8FOEW

#include <atomic>
#include <functional>
#include <shared_mutex>
#include <string>
#include <variant>

struct GLFWwindow;

namespace glfw_cpp
{
    class Window;
    class WindowManager;

    class Api
    {
    public:
        using GLProc      = void (*)();
        using GLGetProc   = GLProc(const char*);
        using GLContext   = ::GLFWwindow*;
        using GLLoaderFun = std::function<void(GLContext handle, GLGetProc proc)>;

        struct OpenGLES
        {
            int         m_major = 2;
            int         m_minor = 0;
            GLLoaderFun m_loader;
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
            GLLoaderFun m_loader;
        };

        struct Vulkan
        {
        };

        using Variant = std::variant<OpenGL, OpenGLES, Vulkan>;
    };

    // Context is an instance that can only be instantiated once but can be moved around
    class Context
    {
    public:
        friend WindowManager;
        friend Window;

        static inline std::atomic<bool> s_hasInstance = false;

        enum class LogLevel
        {
            NONE,
            CRITICAL,
            ERROR,
            WARNING,
            INFO,
            DEBUG,
        };

        using ErrorCallback = void(int errc, const char* description);
        using LogFun        = std::function<void(LogLevel level, std::string msg)>;

        // loader must be a valid lambda/function pointer to a function that loads OpenGL functions
        Context(Api::Variant api);
        ~Context();
        Context(Context&&) noexcept;
        Context& operator=(Context&&) noexcept;

        Context()                          = default;
        Context(const Context&)            = delete;
        Context& operator=(const Context&) = delete;

        // can be called from any thread
        void setLogCallback(LogFun callback);

        // set internal GLFW error (do this once)
        static void setErrorCallback(ErrorCallback* callback);

    private:
        mutable std::shared_mutex m_mutex;

        bool         m_initialized = false;
        Api::Variant m_api;
        LogFun       m_logCallback;

        // can be called from any thread
        void log(LogLevel level, std::string msg) const;

        void logD(std::string msg) const { log(LogLevel::DEBUG, std::move(msg)); }
        void logI(std::string msg) const { log(LogLevel::INFO, std::move(msg)); }
        void logW(std::string msg) const { log(LogLevel::WARNING, std::move(msg)); }
        void logE(std::string msg) const { log(LogLevel::ERROR, std::move(msg)); }
        void logC(std::string msg) const { log(LogLevel::CRITICAL, std::move(msg)); }
    };
}

#endif /* end of include guard: CONTEXT_HPP_AO39EW8FOEW */
