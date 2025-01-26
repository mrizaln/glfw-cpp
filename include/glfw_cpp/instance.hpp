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
    class IEventInterceptor;

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

            int         m_major          = 1;
            int         m_minor          = 0;
            Profile     m_profile        = Profile::Core;    // only makes sense for OpenGL 3.2 above
            bool        m_forward_compat = true;             // only makes sense for OpenGL 3.0 above
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
         * @brief Create a `WindowManager` instance.
         *
         * @param event_interceptor The event interceptor (can be nullptr).
         *
         * @return A shared pointer to the `WindowManager` instance.
         *
         * The interceptor should have a longer lifetime than this `WindowManager` and the memory management
         * of it is the responsibility of the caller.
         */
        std::shared_ptr<WindowManager> create_window_manager(
            IEventInterceptor* event_interceptor = nullptr
        ) noexcept;

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

        void set_logger(LogFun&& logger) noexcept;
        void reset() noexcept;

        static Instance& get() noexcept;

        // can be called from any thread
        static void log(LogLevel level, std::string msg) noexcept;

        template <typename... Args>
        static void log_d(std::format_string<Args...> fmt, Args&&... args) noexcept
        {
            log(LogLevel::Debug, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void log_i(std::format_string<Args...> fmt, Args&&... args) noexcept
        {
            log(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void log_w(std::format_string<Args...> fmt, Args&&... args) noexcept
        {
            log(LogLevel::Warning, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void log_e(std::format_string<Args...> fmt, Args&&... args) noexcept
        {
            log(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void log_c(std::format_string<Args...> fmt, Args&&... args) noexcept
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

    inline std::string_view to_string(Instance::LogLevel level)
    {
        using L = glfw_cpp::Instance::LogLevel;
        switch (level) {
        case L::None: return "NONE";
        case L::Debug: return "DEBUG";
        case L::Info: return "INFO";
        case L::Warning: return "WARNING";
        case L::Error: return "ERROR";
        case L::Critical: return "CRITICAL";
        default: [[unlikely]] return "UNKNOWN";
        }
    }
}

#endif /* end of include guard: INSTANCE_HPP_AO39EW8FOEW */
