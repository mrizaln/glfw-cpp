#ifndef INSTANCE_HPP_AO39EW8FOEW
#define INSTANCE_HPP_AO39EW8FOEW

#include "glfw_cpp/detail/helper.hpp"

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

    namespace api
    {
        namespace gl
        {
            using Proc      = void (*)();
            using GetProc   = Proc(const char*);
            using Context   = ::GLFWwindow*;
            using LoaderFun = std::function<void(Context handle, GetProc proc)>;

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
        }

        /**
         * @struct OpenGLES
         * @brief Describe the OpenGLES version to use.
         *
         * On emscripten, the loader is optional, so you can just set the value to `nullptr` and use the
         * `<GL/gl.h>` header directly. But if you want to use a loader anyway like `glbinding` then you can
         * set the loader like usual.
         */
        struct OpenGLES
        {
            int           major = 2;
            int           minor = 0;
            gl::LoaderFun loader;
        };

        /**
         * @struct OpenGL
         * @brief Describe the OpenGL version to use.
         *
         * On emscripten, the loader is optional, so you can just set the value to `nullptr` and use the
         * `<GL/gl.h>` header directly. But if you want to use a loader anyway like `glbinding` then you can
         * set the loader like usual.
         */
        struct OpenGL
        {
            int           major          = 1;
            int           minor          = 0;
            gl::Profile   profile        = gl::Profile::Core;    // only makes sense for OpenGL 3.2 and above
            bool          forward_compat = true;                 // only makes sense for OpenGL 3.0 and above
            gl::LoaderFun loader;
        };

        /**
         * @struct NoApi
         * @brief No API is used (for Vulkan applications).
         */
        struct NoApi
        {
        };

        using Variant = std::variant<OpenGL, OpenGLES, NoApi>;

        template <typename T>
        concept Api = detail::traits::VarTrait<Variant>::template is_elem<T>();
    }

    /**
     * @struct Api
     * @brief Describe the underlying graphics API to use with GLFW (variant of OpenGL, OpenGLES, or no API).
     */
    class Api : public detail::variants::VariantBase<api::Variant>
    {
    public:
        Api() = default;

        template <typename T>
        Api(T&& t)
            : VariantBase{ std::forward<T>(t) }
        {
        }
    };

    /**
     * @enum LogLevel
     * @brief Log levels for the logger function.
     */
    enum class LogLevel
    {
        None,
        Debug,
        Info,
        Warning,
        Error,
        Critical,
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

        // LogFun should be noexcept
        using LogFun = std::function<void(LogLevel level, std::string msg)>;
        using Unique = std::unique_ptr<Instance, void (*)(Instance*)>;

        /**
         * @brief Initialize GLFW and returns a RAII handle that will terminate GLFW on destruction.
         */
        friend Unique init(Api&&, Instance::LogFun&&);

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

        bool   m_initialized = false;
        Api    m_api;
        LogFun m_loggger;

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
    Instance::Unique init(Api&& api, Instance::LogFun&& logger = nullptr);

    /**
     * @brief Convert a `LogLevel` enum to a string.
     */
    inline std::string_view to_string(LogLevel level)
    {
        switch (level) {
        case LogLevel::None: return "NONE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        default: [[unlikely]] return "UNKNOWN";
        }
    }
}

#endif /* end of include guard: INSTANCE_HPP_AO39EW8FOEW */
