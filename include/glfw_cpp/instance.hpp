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

    /**
     * @struct Api
     * @brief Describe the underlying graphics API to use with GLFW (variant of OpenGL, OpenGLES, or no API).
     */
    struct Api
    {
        using GlProc      = void (*)();
        using GlGetProc   = GlProc(const char*);
        using GlContext   = ::GLFWwindow*;
        using GlLoaderFun = std::function<void(GlContext handle, GlGetProc proc)>;

        /**
         * @struct OpenGLES
         * @brief Describe the OpenGLES version to use.
         */
        struct OpenGLES
        {
            int         m_major = 2;
            int         m_minor = 0;
            GlLoaderFun m_loader;
        };

        /**
         * @struct OpenGL
         * @brief Describe the OpenGL version to use.
         */
        struct OpenGL
        {
            /**
             * @enum Profile
             * @brief OpenGL profile to use.
             */
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

        /**
         * @struct NoApi
         * @brief No API is used (for Vulkan applications).
         */
        struct NoApi
        {
        };

        using Variant = std::variant<OpenGL, OpenGLES, NoApi>;
    };

    /**
     * @class Instance
     * @brief Singleton that manages the global state required to interface with GLFW.
     */
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
        using Unique = std::unique_ptr<Instance, void (*)(Instance*)>;

        /**
         * @brief Initialize GLFW and returns a RAII handle that will terminate GLFW on destruction.
         *
         * @param api The underlying graphics API to use with GLFW.
         * @param logger The logger function to use.
         * @return A RAII handle that will terminate GLFW on destruction.
         *
         * @throw std::runtime_error If the initialization failed.
         */
        friend Unique init(Api::Variant&&, Instance::LogFun&&) noexcept(false);

        /**
         * @brief Create a WindowManager instance.
         * @return A shared pointer to the WindowManager instance.
         */
        std::shared_ptr<WindowManager> createWindowManager() noexcept;

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

        void setLogger(LogFun&& logger) noexcept;
        void reset() noexcept;

        static Instance& get() noexcept;

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

    /**
     * @brief Initialize GLFW and returns a RAII handle that will terminate GLFW on destruction.
     * @param api The underlying graphics API to use with GLFW.
     * @param logger The logger function to use.
     * @return A RAII handle that will terminate GLFW on destruction.
     */
    Instance::Unique init(Api::Variant&& api, Instance::LogFun&& logger = nullptr) noexcept(false);
}

#endif /* end of include guard: INSTANCE_HPP_AO39EW8FOEW */
