#ifndef INSTANCE_HPP_AO39EW8FOEW
#define INSTANCE_HPP_AO39EW8FOEW

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
                Core,
                Compat,
                Any,
            };

            int         m_major         = 1;
            int         m_minor         = 0;
            Profile     m_profile       = Profile::Core;    // only makes sense for OpenGL 3.2 above
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
            None,
            Debug,
            Info,
            Warning,
            Error,
            Critical,
        };

        // LogFun should be noexcept
        using LogFun = std::function<void(LogLevel level, std::string msg)>;
        using Unique = std::unique_ptr<Instance, void (*)(Instance*)>;

        /**
         * @brief Initialize GLFW and returns a RAII handle that will terminate GLFW on destruction.
         */
        friend Unique init(Api::Variant&&, Instance::LogFun&&);

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
        static void log(LogLevel level, std::string msg) noexcept;

        template <typename... Args>
        static void logD(std::format_string<Args...> fmt, Args&&... args) noexcept
        {
            log(LogLevel::Debug, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void logI(std::format_string<Args...> fmt, Args&&... args) noexcept
        {
            log(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void logW(std::format_string<Args...> fmt, Args&&... args) noexcept
        {
            log(LogLevel::Warning, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void logE(std::format_string<Args...> fmt, Args&&... args) noexcept
        {
            log(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void logC(std::format_string<Args...> fmt, Args&&... args) noexcept
        {
            log(LogLevel::Critical, std::format(fmt, std::forward<Args>(args)...));
        }
    };

    // Global state required to interface with GLFW
    inline Instance Instance::s_instance = {};

    /**
     * @brief Initialize GLFW and returns a RAII handle that will terminate GLFW on destruction.
     *
     * @param api The underlying graphics API to use with GLFW.
     * @param logger The logger function to use.
     * @return A RAII handle that will terminate GLFW on destruction.
     *
     * @throw glfw_cpp::AlreadyInitialized if GLFW is already initialized.
     * @throw glfw_cpp::EmptyLoader if the loader function is empty (OpenGL/OpenGL ES only).
     * @throw glfw_cpp::ApiUnavailable if the requested client API is unavailable.
     * @throw glfw_cpp::VersionUnavailable if the requested client API version is unavailable.
     * @throw glfw_cpp::PlatformError if a platform-specific error occurred.
     */
    Instance::Unique init(Api::Variant&& api, Instance::LogFun&& logger = nullptr);
}

#endif /* end of include guard: INSTANCE_HPP_AO39EW8FOEW */
