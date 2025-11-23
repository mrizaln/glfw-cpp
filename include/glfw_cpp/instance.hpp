#ifndef INSTANCE_HPP_AO39EW8FOEW
#define INSTANCE_HPP_AO39EW8FOEW

#include "glfw_cpp/detail/helper.hpp"

#include <chrono>
#include <format>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <variant>

struct GLFWwindow;
struct GLFWmonitor;

namespace glfw_cpp
{
    class Window;
    class IEventInterceptor;
    class Event;

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

    /**
     * @enum Flag
     * @brief Window flags used in its creation.
     */
    enum class Flag : std::uint32_t
    {
        None                   = 0,
        Resizable              = 1 << 0,
        Visible                = 1 << 1,
        Decorated              = 1 << 2,
        Focused                = 1 << 3,
        AutoIconify            = 1 << 4,
        Floating               = 1 << 5,
        Maximized              = 1 << 6,
        CenterCursor           = 1 << 7,
        TransparentFramebuffer = 1 << 8,
        FocusOnShow            = 1 << 9,
        ScaleToMonitor         = 1 << 10,

        Default = Resizable | Visible | Decorated | Focused | AutoIconify | FocusOnShow,
    };

    template <>
    struct detail::enums::EnableOperators<Flag> : std::true_type
    {
    };

    using detail::enums::operators::operator~;
    using detail::enums::operators::operator|;
    using detail::enums::operators::operator&;
    using detail::enums::operators::operator^;
    using detail::enums::operators::operator|=;
    using detail::enums::operators::operator&=;
    using detail::enums::operators::operator^=;

    /**
     * @struct Hint
     * @brief Window creation hints.
     *
     * The window hints included here are only the relevant ones. Graphics API is omitted since the API is not
     * allowed to change at runtime (my design choice).
     */
    struct Hint
    {
        GLFWmonitor* monitor = nullptr;
        GLFWwindow*  share   = nullptr;

        Flag flags = Flag::Default;

        int red_bits     = 8;
        int green_bits   = 8;
        int blue_bits    = 8;
        int alpha_bits   = 8;
        int depth_bits   = 24;
        int stencil_bits = 8;

        int samples      = 0;
        int refresh_rate = -1;    // -1 means don't care
    };

    /**
     * @class Instance
     * @brief Singleton that manages the global state required to interface with GLFW.
     */
    class Instance
    {
    public:
        friend Window;

        // LogFun should be noexcept
        using LogFun = std::function<void(LogLevel level, std::string msg)>;

        friend std::unique_ptr<Instance> init(Api&&, Instance::LogFun&&);

        ~Instance();
        Instance& operator=(Instance&&)      = delete;
        Instance(Instance&&)                 = delete;
        Instance(const Instance&)            = delete;
        Instance& operator=(const Instance&) = delete;

        /**
         * @brief Create a window.
         *
         * @param hint The window hint.
         * @param title The window title.
         * @param width The window width.
         * @param height The window height.
         * @param make_current Immediately make the OpenGL/OpenGL ES context current to this thread.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw glfw_cpp::WrongThreadAccess The function is called not from the main thread.
         * @throw glfw_cpp::ApiUnavailable The requested client API is unavailable.
         * @throw glfw_cpp::VersionUnavailable The requested client API version is unavailable.
         * @throw glfw_cpp::FormatUnavailable The requested format is unavailable.
         * @throw glfw_cpp::NoWindowContext The specified window does not have an OpenGL or OpenGL ES context.
         * @throw glfw_cpp::PlatformError A platform-specific error occurred.
         */
        Window create_window(
            const Hint&      hint,
            std::string_view title,
            int              width,
            int              height,
            bool             make_current = true
        );

        /**
         * @brief Set an event interceptor.
         *
         * @param event_interceptor The event interceptor (nullptr to remove).
         *
         * @return The old event interceptor.
         *
         * The interceptor should have a longer lifetime than this `Instance` and the memory management
         * of it is the responsibility of the caller.
         */
        IEventInterceptor* set_event_interceptor(IEventInterceptor* event_interceptor) noexcept
        {
            return std::exchange(m_event_interceptor, event_interceptor);
        }

        /**
         * @brief Set logger for glfw-cpp.
         *
         * @param logger The new logger function.
         *
         * This function replaces the logger used by glfw-cpp used to log its debug information and the
         * underlying GLFW errors. You can set the argument to nullptr to effectively turn off the logger.
         */
        void set_logger(LogFun&& logger) noexcept { m_loggger = logger; }

        /**
         * @brief Check if any window is still open.
         *
         * @thread_safety This function can be called from any thread.
         */
        bool has_window_opened() const noexcept;

        /**
         * @brief Poll events for all windows.
         *
         * @param poll_rate The poll rate, or `std::nullopt` if no sleep is needed.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw glfw_cpp::WrongThreadAccess The function is called not from the main thread.
         */
        void poll_events(std::optional<std::chrono::milliseconds> poll_rate = {});

        /**
         * @brief Wait for events for all windows.
         *
         * @param timeout The timeout, or `std::nullopt` if no timeout is needed.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw glfw_cpp::WrongThreadAccess The function is called not from the main thread.
         */
        void wait_events(std::optional<std::chrono::milliseconds> timeout = {});

        /**
         * @brief Request to delete a window.
         *
         * @param handle The window handle.
         *
         * @thread_safety This function can be called from any thread.
         */
        void request_delete_window(GLFWwindow* handle) noexcept;

        /**
         * @brief Enqueue a task to be processed in the main thread.
         *
         * @param task The task.
         *
         * @thread_safety This function can be called from any thread.
         *
         * This function can be used for any task that needs to be executed in the main thread.
         */
        void enqueue_task(std::function<void()>&& task) noexcept;

        /**
         * @brief Get the thread id this instance is attached to.
         */
        std::thread::id attached_thread_id() const noexcept { return m_attached_thread_id; }

    private:
        inline static Instance* s_instance = nullptr;

        /**
         * @brief Constructs a default instance (same as uninitialized)
         */
        Instance() = default;

        /**
         * @brief Safe (in debug) wrapper to get the instance from the global pointer
         */
        static Instance& get();

        /**
         * @brief Log a message.
         *
         * @param level Severity of the message.
         * @param msg The message.
         */
        static void log(LogLevel level, std::string msg) noexcept;

#define GLFW_CPP_LOG_FN(Name, Level)                                                                         \
    template <typename... Args>                                                                              \
    static void Name(std::format_string<Args...> fmt, Args&&... args) noexcept                               \
    {                                                                                                        \
        log(LogLevel::Level, std::format(fmt, std::forward<Args>(args)...));                                 \
    }

        GLFW_CPP_LOG_FN(log_d, Debug)
        GLFW_CPP_LOG_FN(log_i, Info)
        GLFW_CPP_LOG_FN(log_w, Warning)
        GLFW_CPP_LOG_FN(log_e, Error)
        GLFW_CPP_LOG_FN(log_c, Critical)

#undef GLFW_CPP_LOG_FN

        static void window_pos_callback(GLFWwindow* window, int x, int y);
        static void window_size_callback(GLFWwindow* window, int width, int height);
        static void window_close_callback(GLFWwindow* window);
        static void window_refresh_callback(GLFWwindow* window);
        static void window_focus_callback(GLFWwindow* window, int focused);
        static void window_iconify_callback(GLFWwindow* window, int iconified);
        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void cursor_pos_callback(GLFWwindow* window, double x, double y);
        static void cursor_enter_callback(GLFWwindow* window, int entered);
        static void scroll_callback(GLFWwindow* window, double x, double y);
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void char_callback(GLFWwindow* window, unsigned int codepoint);
        static void file_drop_callback(GLFWwindow* window, int count, const char** paths);
        static void window_maximize_callback(GLFWwindow* window, int maximized);
        static void window_content_scale_callback(GLFWwindow* window, float xscale, float yscale);

        // TODO: Implement these two callbacks
        /*
            static void monitor_callback(GLFWmonitor* monitor, int action);
            static void joystick_callback(int jid, int action);
        */

        /**
         * @brief Check whether caller thread is the same as attached thread.
         *
         * @throw glfw_cpp::WrongThreadAccess If this function is not called from attached_thread_id.
         */
        void validate_access() const;

        /**
         * @brief Push event to Window but intercept it first using the associated interceptor.
         *
         * @param window The window on which the event belong to.
         * @param event The event to be pushed.
         */
        void push_event(Window& window, Event event) noexcept;

        // Run queued tasks.
        // May throw `PlatformError`.
        /**
         * @brief Run queued tasks.
         *
         * @throw glfw_cpp::PlatformError If the underlying platform produces an error.
         */
        void run_tasks();

        Api    m_api     = api::NoApi{};
        LogFun m_loggger = nullptr;

        std::thread::id    m_attached_thread_id = {};
        IEventInterceptor* m_event_interceptor  = nullptr;

        std::vector<GLFWwindow*>           m_windows;
        std::vector<GLFWwindow*>           m_window_delete_queue;
        std::vector<std::function<void()>> m_task_queue;

        mutable std::mutex m_mutex;    // protects queue
    };

    /**
     * @brief Turns fps to milliseconds.
     */
    constexpr std::chrono::milliseconds operator""_fps(unsigned long long fps)
    {
        namespace chr = std::chrono;
        return chr::duration_cast<chr::milliseconds>(chr::milliseconds{ 1000 } / fps);
    }

    /**
     * @brief Initialize GLFW and returns a RAII handle that will terminate GLFW on destruction.
     *
     * @param api The underlying graphics API to use with GLFW.
     * @param logger The logger function to use.
     * @return A RAII handle that will terminate GLFW on destruction.
     *
     * @throw glfw_cpp::AlreadyInitialized if GLFW is already initialized.
     * @throw glfw_cpp::EmptyLoader if the loader function is empty (non-emscripten OpenGL/OpenGL ES only).
     * @throw glfw_cpp::ApiUnavailable if the requested client API is unavailable.
     * @throw glfw_cpp::VersionUnavailable if the requested client API version is unavailable.
     * @throw glfw_cpp::PlatformError if a platform-specific error occurred.
     */
    std::unique_ptr<Instance> init(Api&& api, Instance::LogFun&& logger = nullptr);

    /**
     * @brief Make the OpenGL or OpenGL ES context of the specified window current on calling thread.
     *
     * @param context The window whose context to make current (pass null to detach).
     *
     * @throw glfw_cpp::NotInitialized if GLFW is not initialized.
     * @throw glfw_cpp::NoWindowContext if the window doesn't have OpenGL or OpenGL ES context.
     * @throw glfw_cpp::PlatformError if a platform-specific error occurred.
     */
    void make_current(GLFWwindow* window);

    /**
     * @brief Get window handle whose context is current.
     *
     * @throw glfw_cpp::NotInitialized if GLFW is not initialized.
     */
    GLFWwindow* get_current();

    /**
     * @brief Set the clipboard string.
     *
     * @param string The string to set (must be null-terminated).
     */
    void set_clipboard_string(const char* string);

    /**
     * @brief Get the clipboard string.
     *
     * @return The clipboard string.
     */
    std::string_view get_clipboard_string();

    /**
     * @brief Get time in seconds since the GLFW library was initialized.
     *
     * @return The time in seconds.
     */
    double get_time();

    /**
     * @brief Set the current GLFW time.
     *
     * @param time The time to set, in seconds.
     *
     * The time value must be a positive finite number less than or equal to 18446744073.0.
     */
    void set_time(double time);

    /**
     * @brief Get the current value of the raw timer, measured in 1 / frequency seconds.
     *
     * @return The timer value in microseconds.
     */
    uint64_t get_timer_value();

    /**
     * @brief Get the frequency, in Hz, of the raw timer.
     *
     * @return The timer frequency.
     */
    uint64_t get_timer_frequency();
}

#endif /* end of include guard: INSTANCE_HPP_AO39EW8FOEW */
