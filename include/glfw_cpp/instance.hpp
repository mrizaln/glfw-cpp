#ifndef GLFW_CPP_INSTANCE_HPP
#define GLFW_CPP_INSTANCE_HPP

#include "glfw_cpp/constants.hpp"
#include "glfw_cpp/error.hpp"
#include "glfw_cpp/helper.hpp"

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
    class EventInterceptor;
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

    /**
     * The following note is only relevant if you are using glfw-cpp from emscripten.
     *
     * @note emscripten-glfw treats the `GLFW_CLIENT_API` value `GLFW_OPENGL_API` and `GLFW_OPENGLES_API` as
     * the same. If the hint is set to either, the library will create a WebGL context via emscripten html5
     * API: `emscripten_webgl_create_context()`. To use the hint for OpenGL and OpenGLES then is misleading,
     * so I decided to add WebGL struct just for this while disabling OpenGL and OpenGLES on Emscripten. Read
     * the documentation from emscripten-glfw for more detail:
     * - https://github.com/pongasoft/emscripten-glfw/blob/master/docs/Usage.md#webglopengl-support
     */
    namespace api
    {
        using helper::meta::may_opt;
        using helper::meta::MayOpt;

#if __EMSCRIPTEN__
        /**
         * @struct WebGL
         * @brief Describe the WebGL version to use.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         */
        template <bool Opt = true>
        struct WebGL
        {
            MayOpt<Opt, int> version_major = may_opt<Opt>(1);
            MayOpt<Opt, int> version_minor = may_opt<Opt>(0);
        };
#else
        /**
         * @struct OpenGL
         * @brief Describe the OpenGL version to use.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         *
         * The field `forward_compat` only makes sense for OpenGL 3.0+.
         * The field `profile` only makes sense for OpenGL 3.2+.
         */
        template <bool Opt = true>
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
        template <bool Opt = true>
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
#endif
        /**
         * @struct NoApi
         * @brief No API is used (for Vulkan applications).
         */
        struct NoApi
        {
        };

#if __EMSCRIPTEN__
        template <bool Opt = true>
        using Variant = std::variant<WebGL<Opt>, NoApi>;
#else
        template <bool Opt = true>
        using Variant = std::variant<OpenGL<Opt>, OpenGLES<Opt>, NoApi>;
#endif

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
        template <bool Opt = true>
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
        template <bool Opt = true>
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
            MayOpt<Opt, int>  position_x              = may_opt<Opt>(constants::any_position);
            MayOpt<Opt, int>  position_y              = may_opt<Opt>(constants::any_position);
        };

        /**
         * @struct Framebuffer
         * @brief Framebuffer related hints for window creation.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         */
        template <bool Opt = true>
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
        template <bool Opt = true>
        struct Monitor
        {
            MayOpt<Opt, int> refresh_rate = may_opt<Opt>(constants::dont_care);
        };

        /**
         * @struct Win32
         * @brief Win32 (Windows) plaform related hints for window creation.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         *
         * The hints will be ignored if the platform used is not Win32.
         */
        template <bool Opt = true>
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
        template <bool Opt = true>
        struct Cocoa
        {
            MayOpt<Opt, const char*> frame_name         = may_opt<Opt>("");    // must not be nullptr
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
        template <bool Opt = true>
        struct Wayland
        {
            MayOpt<Opt, const char*> app_id = may_opt<Opt>("");    // must not be nullptr
        };

        /**
         * @struct X11
         * @brief X11 (Linux) plaform related hints for window creation.
         *
         * @tparam Opt Indicates whether to use optional fields or not.
         *
         * The hints will be ignored if the platform used is not Win32.
         */
        template <bool Opt = true>
        struct X11
        {
            MayOpt<Opt, const char*> class_name    = may_opt<Opt>("");    // must not be nullptr
            MayOpt<Opt, const char*> instance_name = may_opt<Opt>("");    // must not be nullptr
        };

        /**
         * @struct Emscripten
         * @brief Emscripten-specific option for window creation logic.
         *
         * Emscripten port of GLFW (emscripten-glfw) associates the concept of a window to am HTML canvas
         * element. The framebuffer size of the window is the size of the canvas, the size of the window is
         * the CSS style size of the canvas, etc.
         *
         * Window creation hint:
         * - `Emscripten::canvas_selector`: canvas in which the window will be rendered.
         *
         * Canvas (dynamic) resize behavior:
         * - `Emscripten::resize_selector`: selector to html element that dictates the canvas size.
         * - `Emscripten::handle_selector`: selector to the "handle" element for resizing.
         *
         * You always want to set these fields to new values for each window (one canvas for each window).
         *
         * Set `resize_selector` to `nullptr` to disable the dynamic resize behavior (`handle_selector` is
         * ignored in this case). You don't need to have "handle" if you don't intend to make the user able to
         * resize the canvas dynamically (only let the canvas resize whenever the element resize).
         *
         * @note For more information regarding the behavior of the canvas and the resize behavior, read the
         * extension function defined in `<GLFW/emscripten_glfw3.h>` source file in the original
         * [emscripten-glfw](https://github.com/pongasoft/emscripten-glfw) repostory.
         *
         * There are three typical use case for the resize behavior:
         *
         * 1. Fill entire browser window:
         *
         *    > - set `resize_selector` to `"window"`, and
         *    > - set `handle_selector` to `nullptr` (initial value).
         *
         *    ```html
         *    <canvas id="canvas1"></canvas>
         *    ```
         *
         *    ```cpp
         *    glfw->apply_hints({
         *       .emscripten = {
         *           .canvas_selector = "#canvas1",
         *           .resize_selector = "window",
         *           .handle_selector = nullptr,        // if you left the field, it will use previous value
         *       },
         *       // ...
         *    });
         *    ```
         *
         * 2. The canvas is inside a `div`, in which case the `div` acts as a "container" and the `div` size
         * is defined by CSS rules, like for example: `width: 85%` so that when the page/browser gets resized,
         * the `div` is resized automatically, which then triggers the canvas to be resized.
         *
         *    > - set `resize_selector` to the css path to the `div`, and
         *    > - set `handle_selector` to `nullptr` (initial value).
         *
         *    ```html
         *    <style>
         *      #canvas1-container { width: 85%; height: 85% }
         *    </style>
         *
         *    <div id="canvas1-container">
         *      <canvas id="canvas1"></canvas>
         *    </div>
         *    ```
         *
         *    ```cpp
         *    glfw->apply_hints({
         *       .emscripten = {
         *           .canvas_selector = "#canvas1",
         *           .resize_selector = "#canvas1-container"
         *           .handle_selector = nullptr,        // if you left the field, it will use previous value
         *        },
         *       // ...
         *    });
         *    ```
         *
         * 3. Like use-case 2 but the `div` is made resizable dynamically via a little "handle".
         *
         *    > - set `resize_selector` to the css path to the `div` and
         *    > - set `handle_selector` to the css path the "handle".
         *
         *    ```html
         *    <style>
         *      #canvas1-container {
         *        position: relative;
         *        <!-- ... -->
         *      }
         *
         *      #canvas1-handle {
         *        position: absolute;
         *        bottom: 0;
         *        right: 0;
         *        background-color: #444444;
         *        width: 10px;
         *        height: 10px;
         *        cursor:
         *        nwse-resize;
         *      }
         *    </style>
         *
         *    <div id="canvas1-container">
         *      <div id="canvas1-handle" class="handle"></div>
         *      <canvas id="canvas1"></canvas>
         *    </div>
         *    ```
         *
         *    ```cpp
         *    glfw->apply_hints({
         *       .emscripten = {
         *           .canvas_selector = "#canvas1",
         *           .resize_selector = "#canvas1-container",
         *           .handle_selector = "#canvas1-handle",
         *       },
         *       // ...
         *    });
         *    ```
         */
        template <bool Opt = true>
        struct Emscripten
        {
            // must not be nullptr
            MayOpt<Opt, const char*> canvas_selector = may_opt<Opt>(default_canvas_selector);
            MayOpt<Opt, const char*> resize_selector = may_opt<Opt>(nullptr);
            MayOpt<Opt, const char*> handle_selector = may_opt<Opt>(nullptr);
        };

        /**
         * @enum Platform
         * @brief Specifies the platform to use for windowing and input.
         */
        enum class Platform : int
        {
            Any        = 0x00060000,
            Win32      = 0x00060001,
            Cocoa      = 0x00060002,
            Wayland    = 0x00060003,
            X11        = 0x00060004,
            Null       = 0x00060005,
            Emscripten = 0x00060006,    // not officially supported GLFW platform (only on emscripten-glfw)
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
    }

    /**
     * @struct Hints
     * @brief Window creation hints.
     *
     * @tparam Opt Indicates whether to use optional fields or not.
     */
    template <bool Opt = true>
    struct Hints
    {
#if __EMSCRIPTEN__
        hint::Api<Opt> api = api::WebGL<Opt>{};
#else
        hint::Api<Opt> api = api::OpenGL<Opt>{};
#endif
        hint::Window<Opt>      window      = {};
        hint::Framebuffer<Opt> framebuffer = {};
        hint::Monitor<Opt>     monitor     = {};
        hint::Win32<Opt>       win32       = {};
        hint::Cocoa<Opt>       cocoa       = {};
        hint::Wayland<Opt>     wayland     = {};
        hint::X11<Opt>         x11         = {};
        hint::Emscripten<Opt>  emscripten  = {};    // You want to set this to new values for each window
    };

    using FullHints    = Hints<false>;
    using PartialHints = Hints<true>;

    /**
     * @class InitHint
     * @brief Initialization hints are set before `glfwInit` and affect how the library behaves until
     * termination.
     */
    struct InitHints
    {
        hint::Platform        platform               = hint::Platform::Any;
        bool                  joystick_hat_buttons   = true;
        hint::AnglePlatform   angle_platform_type    = hint::AnglePlatform::None;
        bool                  cocoa_chdir_resource   = true;
        bool                  cocoa_menubar          = true;
        hint::WaylandLibdecor wayland_libdecor       = hint::WaylandLibdecor::Prefer;
        bool                  x11_xcb_vulkan_surface = true;
    };

    /**
     * @class Instance
     * @brief Singleton that manages the global state required to interface with GLFW.
     */
    class Instance
    {
    public:
        friend Window;

        using ErrorCallback = std::function<void(ErrorCode, std::string_view)>;
        using Unique        = std::unique_ptr<Instance>;

        friend Unique init(const InitHints&);

        ~Instance();
        Instance& operator=(Instance&&)      = delete;
        Instance(Instance&&)                 = delete;
        Instance(const Instance&)            = delete;
        Instance& operator=(const Instance&) = delete;

        /**
         * @brief Apply window creation hints (partially).
         *
         * @param hints The hints to be applied.
         *
         * The hints sticks to the instance. The values set won't be changed unless a new hint is applied.
         *
         * This function does not check whether the specified hint values are valid. If you set hints to
         * invalid values this will instead be reported by the next call to `create_window`.
         */
        void apply_hints(const PartialHints& hints);

        /**
         * @brief Apply window creation hints (fully).
         *
         * @param hints The hints to be applied.
         *
         * The hints sticks to the instance. The value set won't be changed unless a new hint is applied.
         *
         * This function does not check whether the specified hint values are valid. If you set hints to
         * invalid values this will instead be reported by the next call to `create_window`.
         *
         * Calling this function with default constructed argument should be equal to calling
         * `apply_hints_default()`.
         */
        void apply_hints_full(const FullHints& hints);

        /**
         * @brief Resets the hints to its default values.
         */
        void apply_hints_default();

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
         * @throw error::WrongThreadAccess The function is called not from the main thread.
         * @throw error::InvalidValue if a hint has invalid value.
         * @throw error::ApiUnavailable The requested client API is unavailable.
         * @throw error::VersionUnavailable if the requested client API version is unavailable.
         * @throw error::FormatUnavailable The requested format is unavailable.
         * @throw error::NoWindowContext The specified window does not have an OpenGL or OpenGL ES context.
         * @throw error::PlatformError A platform-specific error occurred.
         */
        Window create_window(
            int              width,
            int              height,
            std::string_view title,
            GLFWmonitor*     monitor = nullptr,
            GLFWwindow*      share   = nullptr
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
        EventInterceptor* set_event_interceptor(EventInterceptor* event_interceptor) noexcept
        {
            return std::exchange(m_event_interceptor, event_interceptor);
        }

        /**
         * @brief Set error callback for glfw-cpp.
         *
         * @param callback Callback to be installed into glfw-cpp.
         *
         * You can set the argument to `nullptr` to effectively turn off the callback. By default the value
         * for the callback is `nullptr`.
         */
        void set_error_callback(ErrorCallback callback) noexcept { m_callback = callback; }

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
         * @throw error::WrongThreadAccess The function is called not from the main thread.
         */
        void poll_events(std::optional<std::chrono::milliseconds> poll_rate = {});

        /**
         * @brief Wait for events for all windows.
         *
         * @param timeout The timeout, or `std::nullopt` if no timeout is needed.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw error::WrongThreadAccess The function is called not from the main thread.
         */
        void wait_events(std::optional<std::chrono::milliseconds> timeout = {});

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
         * @brief Return the platform that was selected during initialization.
         *
         * Enumerator `hint::Platform::Any` won't be produced by this function.
         */
        hint::Platform platform() const noexcept;

        /**
         * @brief Get the thread id this instance is attached to.
         */
        std::thread::id attached_thread_id() const noexcept { return m_attached_thread_id; }

    private:
        struct CallbackHandler;
        friend CallbackHandler;

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
         * @brief Check whether caller thread is the same as attached thread.
         *
         * @throw error::WrongThreadAccess If this function is not called from attached_thread_id.
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
         * @throw error::PlatformError If the underlying platform produces an error.
         */
        void run_tasks();

        /**
         * @brief Request to delete a window.
         *
         * @param handle The window handle.
         *
         * @thread_safety This function can be called from any thread.
         */
        void request_delete_window(GLFWwindow* handle) noexcept;

        std::thread::id   m_attached_thread_id = std::this_thread::get_id();
        EventInterceptor* m_event_interceptor  = nullptr;
        ErrorCallback     m_callback           = nullptr;

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
     * @brief Check whether GLFW was compiled with support for specified platform.
     *
     * @param platform The platform to query.
     * @return True if platform is supported, false otherwise.
     *
     * @throw error::InvalidValue if `hint::Platform::Any` or invalid enum value is passed as argument.
     */
    bool platform_supported(hint::Platform platform);

    /**
     * @brief Return the address of the specified OpenGL or OpenGLES core or extension function.
     *
     * @param procname The name of the function in ASCII.
     * @return The address of the function, or `nullptr` if an error occurred.
     *
     * @throw error::NotInitialized If GLFW is not initialized.
     * @throw error::NoCurrentContext If there is no context current at the time this function called.
     * @throw error::PlatformError If a platform-specific error occurred.
     *
     * If you need to call this function from C (which can't handle exception) you may want to use `noexcept`
     * version of this function: `get_proc_address_noexcept()`. This is not uncommon if you for example use
     * glad for the OpenGL loader.
     */
    gl::Proc get_proc_address(const char* procname);

    /**
     * @brief Return the address of the specified OpenGL or OpenGLES core or extension function.
     *
     * @param procname The name of the function in ASCII.
     * @return The address of the function, or `nullptr` if an error occurred.
     *
     * The error will be logged but this function won't throw to preserve the behavior from the underlying
     * function. The possible errors are:
     * - `ErrorCode::NotInitialized`,
     * - `ErrorCode::NoCurrentContext`, and
     * - `ErrorCode::PlatformError`.
     *
     * If you don't mind the function throws, use `get_proc_address()` instead.
     */
    gl::Proc get_proc_address_noexcept(const char* procname) noexcept;

    /**
     * @brief Check whether the specified extension is supported by current OpenGL or OpenGL ES context.
     *
     * @param extension The name of the extensio in ASCII.
     * @return True if extension is available or false otherwise.
     *
     * @throw error::NotInitialized If GLFW is not initialized.
     * @throw error::NoCurrentContext If there is no context current at the time this function called.
     * @throw error::InvalidValue The specified extension is invalid.
     * @throw error::PlatformError If a platform-specific error occurred.
     *
     * If you need to call this function from C (which can't handle exception) you may want to use `noexcept`
     * version of this function: `extension_supported_noexcept()`.
     */
    bool extension_supported(const char* extension);

    /**
     * @brief Check whether the specified extension is supported by current OpenGL or OpenGL ES context.
     *
     * @param extension The name of the extensio in ASCII.
     * @return True if extension is available or false otherwise.
     *
     * This function is a wrapper of `glfwExtensionSupported` function. The error will be logged but this
     * function won't throw to preserve the behavior from the underlying function. The possible errors are:
     * - `ErrorCode::NotInitialized`,
     * - `ErrorCode::NoCurrentContext`,
     * - `ErrorCode::InvalidValue`, and
     * - `ErrorCode::PlatformError`.
     *
     * If you don't mind the function throws, use `extension_supported()` instead.
     */
    bool extension_supported_noexcept(const char* extension) noexcept;

    /**
     * @brief Initialize GLFW and returns a RAII handle that will terminate GLFW on destruction.
     *
     * @param hints Initialization hints.
     * @return A RAII handle that will terminate GLFW on destruction.
     *
     * @throw error::AlreadyInitialized if GLFW is already initialized.
     * @throw error::PlatformError if a platform-specific error occurred.
     * @throw error::PlatformUnavailable if platform can't be detected; if `Platform::Any` was set, GLFW
     * can only detect null platform.
     *
     * Pass a default constructed hint to use default init hints.
     */
    Instance::Unique init(const InitHints& hints);

    /**
     * @brief Make the OpenGL or OpenGL ES context of the specified window current on calling thread.
     *
     * @param context The window whose context to make current (pass null to detach).
     *
     * @throw error::NotInitialized if GLFW is not initialized.
     * @throw error::NoWindowContext if the window doesn't have OpenGL or OpenGL ES context.
     * @throw error::PlatformError if a platform-specific error occurred.
     */
    void make_current(GLFWwindow* window);

    /**
     * @brief Get window handle whose context is current.
     *
     * @throw error::NotInitialized if GLFW is not initialized.
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
