#ifndef CONTEXT_HPP_AO39EW8FOEW
#define CONTEXT_HPP_AO39EW8FOEW

#include <functional>
#include <shared_mutex>
#include <string>

struct GLFWwindow;

namespace glfw_cpp
{
    class Window;
    class WindowManager;

    class Context
    {
    public:
        friend WindowManager;
        friend Window;

        enum class Profile
        {
            CORE,
            COMPAT,
            ANY,
        };

        struct Hint
        {
            int     m_major;
            int     m_minor;
            Profile m_profile;
        };

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
        using GLProc        = void (*)();
        using GLGetProc     = GLProc(const char*);
        using GLContext     = ::GLFWwindow*;
        using GLLoaderFun   = std::function<void(GLContext handle, GLGetProc proc)>;

        // loader must be a valid lambda/function pointer to a function that loads OpenGL functions
        Context(Hint hint, GLLoaderFun glLoader);
        ~Context();
        Context(Context&&) noexcept;
        Context& operator=(Context&&) noexcept;

        Context()                          = default;
        Context(const Context&)            = delete;
        Context& operator=(const Context&) = delete;

        // I use C-style function since it needs global state if I use std::function in order for it
        // to be able to be passed to glfwSetErrorCallback
        void setErrorCallback(ErrorCallback* callback);

        // can be called from any thread
        void setLogCallback(LogFun callback);

    private:
        mutable std::shared_mutex m_mutex;

        bool        m_initialized = false;
        Hint        m_hint;
        GLLoaderFun m_loader;
        LogFun      m_logCallback;

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
