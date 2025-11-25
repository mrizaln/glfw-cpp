#ifndef GLFW_CPP_INSTANCE_HPP
#define GLFW_CPP_INSTANCE_HPP

#include "glfw_cpp/helper.hpp"

#include "glfw_cpp/constant.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <utility>
#include <variant>

struct GLFWwindow;
struct GLFWmonitor;

namespace glfw_cpp
{
    class Window;
    class IEventInterceptor;
    struct Event;

    namespace gl
    {
        using Proc    = void (*)();
        using GetProc = Proc(const char*);

        /**
         * @enum Profile
         * @brief OpenGL profile to use.
         */
        enum class Profile : int
        {
            Any    = 0,
            Core   = 0x00032001,
            Compat = 0x00032002,
        };

        /**
         * @enum CreationApi
         * @brief Specifies context creation API to use to create the context.
         */
        enum class CreationApi : int
        {
            Native = 0x00036001,
            EGL    = 0x00036002,
            OSMesa = 0x00036003,
        };

        /**
         * @enum Robustness
         * @brief Specifies the robustness strategy to be used by the context.
         */
        enum class Robustness : int
        {
            NoRobustness        = 0,
            NoResetNotification = 0x00031001,
            LoseContextOnReset  = 0x00031002,
        };

        /**
         * @enum Robustness
         * @brief specifies the release behavior to be used by the context.
         */
        enum class ReleaseBehavior : int
        {
            Any   = 0,
            Flush = 0x00035001,
            None  = 0x00035002,
        };
    }

    namespace api
    {
        using helper::meta::may_opt;
        using helper::meta::MayOpt;

        /**
         * @struct OpenGL
         * @brief Describe the OpenGL version to use.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         *
         * The field `forward_compat` only makes sense for OpenGL 3.0+.
         * The field `profile` only makes sense for OpenGL 3.2+.
         */
        template <bool Opt = false>
        struct OpenGL
        {
            MayOpt<Opt, int>  version_major  = may_opt<Opt>(1);
            MayOpt<Opt, int>  version_minor  = may_opt<Opt>(0);
            MayOpt<Opt, bool> forward_compat = may_opt<Opt>(true);

            MayOpt<Opt, gl::Profile>         profile          = may_opt<Opt>(gl::Profile::Core);
            MayOpt<Opt, gl::CreationApi>     creation_api     = may_opt<Opt>(gl::CreationApi::Native);
            MayOpt<Opt, gl::Robustness>      robustness       = may_opt<Opt>(gl::Robustness::NoRobustness);
            MayOpt<Opt, gl::ReleaseBehavior> release_behavior = may_opt<Opt>(gl::ReleaseBehavior::Any);

            MayOpt<Opt, bool> debug    = may_opt<Opt>(false);
            MayOpt<Opt, bool> no_error = may_opt<Opt>(false);
        };

        /**
         * @struct OpenGLES
         * @brief Describe the OpenGLES version to use.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         */
        template <bool Opt = false>
        struct OpenGLES
        {
            MayOpt<Opt, int> version_major = may_opt<Opt>(1);
            MayOpt<Opt, int> version_minor = may_opt<Opt>(0);

            MayOpt<Opt, gl::CreationApi>     creation_api     = may_opt<Opt>(gl::CreationApi::Native);
            MayOpt<Opt, gl::Robustness>      robustness       = may_opt<Opt>(gl::Robustness::NoRobustness);
            MayOpt<Opt, gl::ReleaseBehavior> release_behavior = may_opt<Opt>(gl::ReleaseBehavior::Any);

            MayOpt<Opt, bool> debug    = may_opt<Opt>(false);
            MayOpt<Opt, bool> no_error = may_opt<Opt>(false);
        };

        /**
         * @struct NoApi
         * @brief No API is used (for Vulkan applications).
         */
        struct NoApi
        {
        };

        template <bool Opt = false>
        using Variant = std::variant<OpenGL<Opt>, OpenGLES<Opt>, NoApi>;

        template <typename T>
        concept Api = helper::variant::VariantMember<T, Variant<false>>
                   or helper::variant::VariantMember<T, Variant<true>>;
    }

    namespace hint
    {
        using helper::meta::may_opt;
        using helper::meta::MayOpt;

        /**
         * @struct Api
         * @brief Describe the underlying graphics API to use with GLFW (variant of OpenGL, OpenGLES, or no
         * API).
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         */
        template <bool Opt = false>
        struct Api : helper::variant::VariantWrapper<api::Variant<Opt>>
        {
            using helper::variant::VariantWrapper<api::Variant<Opt>>::VariantWrapper;
        };

        /**
         * @struct Window
         * @brief Window related hints for window creation.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         */
        template <bool Opt = false>
        struct Window
        {
            MayOpt<Opt, bool> resizable               = may_opt<Opt>(true);
            MayOpt<Opt, bool> visible                 = may_opt<Opt>(true);
            MayOpt<Opt, bool> decorated               = may_opt<Opt>(true);
            MayOpt<Opt, bool> focused                 = may_opt<Opt>(true);
            MayOpt<Opt, bool> auto_iconify            = may_opt<Opt>(true);
            MayOpt<Opt, bool> floating                = may_opt<Opt>(false);
            MayOpt<Opt, bool> maximized               = may_opt<Opt>(false);
            MayOpt<Opt, bool> center_cursor           = may_opt<Opt>(true);
            MayOpt<Opt, bool> transparent_framebuffer = may_opt<Opt>(false);
            MayOpt<Opt, bool> focus_on_show           = may_opt<Opt>(true);
            MayOpt<Opt, bool> scale_to_monitor        = may_opt<Opt>(false);
            MayOpt<Opt, bool> scale_framebuffer       = may_opt<Opt>(true);
            MayOpt<Opt, bool> mouse_passthrough       = may_opt<Opt>(false);
            MayOpt<Opt, int>  position_x              = may_opt<Opt>(constant::any_position);
            MayOpt<Opt, int>  position_y              = may_opt<Opt>(constant::any_position);
        };

        /**
         * @struct Framebuffer
         * @brief Framebuffer related hints for window creation.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         */
        template <bool Opt = false>
        struct Framebuffer
        {
            MayOpt<Opt, int>  red_bits     = may_opt<Opt>(8);
            MayOpt<Opt, int>  green_bits   = may_opt<Opt>(8);
            MayOpt<Opt, int>  blue_bits    = may_opt<Opt>(8);
            MayOpt<Opt, int>  alpha_bits   = may_opt<Opt>(8);
            MayOpt<Opt, int>  depth_bits   = may_opt<Opt>(24);
            MayOpt<Opt, int>  stencil_bits = may_opt<Opt>(8);
            MayOpt<Opt, int>  samples      = may_opt<Opt>(0);
            MayOpt<Opt, bool> stereo       = may_opt<Opt>(false);
            MayOpt<Opt, bool> srgb_capable = may_opt<Opt>(false);
            MayOpt<Opt, bool> doublebuffer = may_opt<Opt>(true);
        };

        /**
         * @struct Monitor
         * @brief Monitor related hints for window creation.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         */
        template <bool Opt = false>
        struct Monitor
        {
            MayOpt<Opt, int> refresh_rate = may_opt<Opt>(constant::dont_care);
        };

        /**
         * @struct Win32
         * @brief Win32 (Windows) plaform related hints for window creation.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         *
         * The hints will be ignored if the platform used is not Win32.
         */
        template <bool Opt = false>
        struct Win32
        {
            MayOpt<Opt, bool> keyboard_menu = may_opt<Opt>(false);
            MayOpt<Opt, bool> showdefault   = may_opt<Opt>(false);
        };

        /**
         * @struct Cocoa
         * @brief Cocoa (macOS) plaform related hints for window creation.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         *
         * The hints will be ignored if the platform used is not Win32.
         */
        template <bool Opt = false>
        struct Cocoa
        {
            MayOpt<Opt, const char*> frame_name         = may_opt<Opt>("");
            MayOpt<Opt, bool>        graphics_switching = may_opt<Opt>(false);
        };

        /**
         * @struct Wayland
         * @brief Wayland (Linux) plaform related hints for window creation.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         *
         * The hints will be ignored if the platform used is not Win32.
         */
        template <bool Opt = false>
        struct Wayland
        {
            MayOpt<Opt, const char*> app_id = may_opt<Opt>("");
        };

        /**
         * @struct X11
         * @brief X11 (Linux) plaform related hints for window creation.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         *
         * The hints will be ignored if the platform used is not Win32.
         */
        template <bool Opt = false>
        struct X11
        {
            MayOpt<Opt, const char*> class_name    = may_opt<Opt>("");
            MayOpt<Opt, const char*> instance_name = may_opt<Opt>("");
        };
    }

    /**
     * @struct Hint
     * @brief Window creation hints.
     *
     * @tparam Opt Indicates whether to use optional fields or not.
     */
    template <bool Opt = false>
    struct Hint
    {
        hint::Api<Opt>         api         = api::OpenGL<Opt>{};
        hint::Window<Opt>      window      = {};
        hint::Framebuffer<Opt> framebuffer = {};
        hint::Monitor<Opt>     monitor     = {};
        hint::Win32<Opt>       win32       = {};
        hint::Cocoa<Opt>       cocoa       = {};
        hint::Wayland<Opt>     wayland     = {};
        hint::X11<Opt>         x11         = {};
    };

    using FullHint    = Hint<false>;
    using PartialHint = Hint<true>;

    /**
     * @enum Platform
     * @brief Specifies the platform to use for windowing and input.
     */
    enum class Platform : int
    {
        Any     = 0x00060000,
        Win32   = 0x00060001,
        Cocoa   = 0x00060002,
        Wayland = 0x00060003,
        X11     = 0x00060004,
        Null    = 0x00060005,
    };

    /**
     * @enum AnglePlatform
     * @brief specifies the platform type (rendering backend) to request when using OpenGL ES and EGL via
     * ANGLE.
     *
     * See ANGLE repo: https://chromium.googlesource.com/angle/angle/.
     */
    enum class AnglePlatform : int
    {
        None     = 0x00037001,
        OpenGL   = 0x00037002,
        OpenGLES = 0x00037003,
        D3D9     = 0x00037004,
        D3D11    = 0x00037005,
        Vulkan   = 0x00037006,
        Metal    = 0x00037007,
    };

    /**
     * @enum WaylandLibdecor
     * @brief Specifies whether to use libdecor for window decorations where available.
     */
    enum class WaylandLibdecor : int
    {
        Prefer  = 0x00038001,
        Disable = 0x00038002,
    };

    /**
     * @class InitHint
     * @brief Initialization hints are set before `glfwInit` and affect how the library behaves until
     * termination.
     */
    struct InitHint
    {
        Platform        platform               = Platform::Any;
        bool            joystick_hat_buttons   = true;
        AnglePlatform   angle_platform_type    = AnglePlatform::None;
        bool            cocoa_chdir_resource   = true;
        bool            cocoa_menubar          = true;
        WaylandLibdecor wayland_libdecor       = WaylandLibdecor::Prefer;
        bool            x11_xcb_vulkan_surface = true;
    };

    /**
     * @class Instance
     * @brief Singleton that manages the global state required to interface with GLFW.
     */
    class Instance
    {
    public:
        friend Window;
        friend std::unique_ptr<Instance> init(const InitHint&);

        using ErrorCallback = std::function<void(int, std::string_view)>;

        ~Instance();
        Instance& operator=(Instance&&)      = delete;
        Instance(Instance&&)                 = delete;
        Instance(const Instance&)            = delete;
        Instance& operator=(const Instance&) = delete;

        /**
         * @brief Apply window creation hints (fully).
         *
         * @param hint The hints to be applied.
         *
         * The hints sticks to the instance. The value set won't be changed unless a new hint is applied. To
         * get the same effect as setting the hint as default (`glfwDefaultWindowHints`) pass a default
         * constructed `Hint`.
         *
         * This function does not check whether the specified hint values are valid. If you set hints to
         * invalid values this will instead be reported by the next call to `create_window`.
         */
        void apply_hint(const FullHint& hint);

        /**
         * @brief Apply window creation hints (partially).
         *
         * @param hint The hints to be applied.
         *
         * The hints sticks to the instance. The value set won't be changed unless a new hint is applied. To
         * get the same effect as setting the hint as default (`glfwDefaultWindowHints`) pass a default
         * constructed `Hint`.
         *
         * This function does not check whether the specified hint values are valid. If you set hints to
         * invalid values this will instead be reported by the next call to `create_window`.
         */
        void apply_hint_some(const PartialHint& hint);

        /**
         * @brief Create a window.
         *
         * @param width The desired width in screen coordinates (must be positive).
         * @param height The desired height in screen coordinates (must be positive).
         * @param title Initial window title (utf-8).
         * @param monitor Monitor to use for full screen mode, or `nullptr` for windowed mode.
         * @param share The window whose context to share with, or `nullptr` to not share resources.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw glfw_cpp::WrongThreadAccess The function is called not from the main thread.
         * @throw glfw_cpp::InvalidValue if a hint has invalid value.
         * @throw glfw_cpp::ApiUnavailable The requested client API is unavailable.
         * @throw glfw_cpp::VersionUnavailable if the requested client API version is unavailable.
         * @throw glfw_cpp::FormatUnavailable The requested format is unavailable.
         * @throw glfw_cpp::NoWindowContext The specified window does not have an OpenGL or OpenGL ES context.
         * @throw glfw_cpp::PlatformError A platform-specific error occurred.
         */
        Window create_window(
            int              width,
            int              height,
            std::string_view title,
            GLFWmonitor*     monitor,
            GLFWwindow*      share
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
        void set_error_callback(std::function<void(int, std::string_view)> callback) noexcept
        {
            m_callback = callback;
        }

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

        std::thread::id    m_attached_thread_id = std::this_thread::get_id();
        IEventInterceptor* m_event_interceptor  = nullptr;
        ErrorCallback      m_callback           = nullptr;

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
     * @brief Return the address of the specified OpenGL or OpenGLES core or extension function.
     *
     * @param procname The name of the function in ASCII.
     * @return The address of the function, or `nullptr` if an error occurred.
     *
     * This function is a wrapper of `glfwGetProcAddress` function. The error will be logged but this function
     * won't throw to preserve the behavior from the underlying function. The possible error is
     * `GLFW_NO_CURRENT_CONTEXT` so make sure to have context current when calling this function.
     */
    gl::Proc get_proc_address(const char* procname) noexcept;

    /**
     * @brief Check whether the specified extension is supported by current OpenGL or OpenGL ES context.
     *
     * @param extension The name of the extensio in ASCII.
     * @return True if extension is available or false otherwise.
     *
     * This function is a wrapper of `glfwExtensionSupported` function. The error will be logged but this
     * function won't throw to preserve the behavior from the underlying function. The possible error is
     * `GLFW_NO_CURRENT_CONTEXT` so make sure to have context current when calling this function.
     */
    bool extension_supported(const char* extension) noexcept;

    /**
     * @brief Initialize GLFW and returns a RAII handle that will terminate GLFW on destruction.
     *
     * @param hint Initialization hints.
     * @return A RAII handle that will terminate GLFW on destruction.
     *
     * @throw glfw_cpp::AlreadyInitialized if GLFW is already initialized.
     * @throw glfw_cpp::PlatformError if a platform-specific error occurred.
     * @throw glfw_cpp::PlatformUnavailable if platform can't be detected; if `Platform::Any` was set, GLFW
     * can only detect null platform.
     *
     * Pass a default constructed hint to use default init hints.
     */
    std::unique_ptr<Instance> init(const InitHint& hint);

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

#endif /* end of include guard: GLFW_CPP_INSTANCE_HPP */
