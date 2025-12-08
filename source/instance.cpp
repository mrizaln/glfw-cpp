#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/window.hpp"

#include "util.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <thread>
#include <utility>

namespace
{
    template <bool Opt, typename A>
    void apply_hints_impl(const glfw_cpp::Hints<Opt>& hints, A adapter)
    {
        const auto& [api, win, fb, mon, win32, cocoa, wl, x11] = hints;

        auto window_hint = adapter;

        // context
        api.visit(util::VisitOverloaded{
            [&](const glfw_cpp::api::OpenGL<Opt>& api) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                window_hint(GLFW_CONTEXT_VERSION_MAJOR, api.version_major);
                window_hint(GLFW_CONTEXT_VERSION_MINOR, api.version_minor);
                window_hint(GLFW_CONTEXT_CREATION_API, api.creation_api);
                window_hint(GLFW_CONTEXT_ROBUSTNESS, api.robustness);
                window_hint(GLFW_CONTEXT_RELEASE_BEHAVIOR, api.release_behavior);
                window_hint(GLFW_CONTEXT_DEBUG, api.debug);
                window_hint(GLFW_CONTEXT_NO_ERROR, api.no_error);

                if (api.version_major >= 3 and api.version_minor >= 0) {
                    window_hint(GLFW_OPENGL_FORWARD_COMPAT, api.forward_compat);
                }
                if (api.version_major >= 3 and api.version_minor >= 3) {
                    window_hint(GLFW_OPENGL_PROFILE, api.profile);
                }
            },
            [&](const glfw_cpp::api::OpenGLES<Opt>& api) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
                window_hint(GLFW_CONTEXT_VERSION_MAJOR, api.version_major);
                window_hint(GLFW_CONTEXT_VERSION_MINOR, api.version_minor);
                window_hint(GLFW_CONTEXT_CREATION_API, api.creation_api);
                window_hint(GLFW_CONTEXT_ROBUSTNESS, api.robustness);
                window_hint(GLFW_CONTEXT_RELEASE_BEHAVIOR, api.release_behavior);
                window_hint(GLFW_CONTEXT_DEBUG, api.debug);
                window_hint(GLFW_CONTEXT_NO_ERROR, api.no_error);
            },
            [](const glfw_cpp::api::NoApi&) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);    //
            },
        });

        // window
        window_hint(GLFW_RESIZABLE, win.resizable);
        window_hint(GLFW_VISIBLE, win.visible);
        window_hint(GLFW_DECORATED, win.decorated);
        window_hint(GLFW_FOCUSED, win.focused);
        window_hint(GLFW_AUTO_ICONIFY, win.auto_iconify);
        window_hint(GLFW_FLOATING, win.floating);
        window_hint(GLFW_MAXIMIZED, win.maximized);
        window_hint(GLFW_CENTER_CURSOR, win.center_cursor);
        window_hint(GLFW_TRANSPARENT_FRAMEBUFFER, win.transparent_framebuffer);
        window_hint(GLFW_FOCUS_ON_SHOW, win.focus_on_show);
        window_hint(GLFW_SCALE_TO_MONITOR, win.scale_to_monitor);
        window_hint(GLFW_SCALE_FRAMEBUFFER, win.scale_framebuffer);
        window_hint(GLFW_MOUSE_PASSTHROUGH, win.mouse_passthrough);
        window_hint(GLFW_POSITION_X, win.position_x);
        window_hint(GLFW_POSITION_Y, win.position_y);

        // framebuffer
        window_hint(GLFW_RED_BITS, fb.red_bits);
        window_hint(GLFW_GREEN_BITS, fb.green_bits);
        window_hint(GLFW_BLUE_BITS, fb.blue_bits);
        window_hint(GLFW_ALPHA_BITS, fb.alpha_bits);
        window_hint(GLFW_DEPTH_BITS, fb.depth_bits);
        window_hint(GLFW_STENCIL_BITS, fb.stencil_bits);
        window_hint(GLFW_SAMPLES, fb.samples);
        window_hint(GLFW_STEREO, fb.stereo);
        window_hint(GLFW_SRGB_CAPABLE, fb.srgb_capable);
        window_hint(GLFW_DOUBLEBUFFER, fb.doublebuffer);

        // monitor
        window_hint(GLFW_REFRESH_RATE, mon.refresh_rate);

        // win32
        window_hint(GLFW_WIN32_KEYBOARD_MENU, win32.keyboard_menu);
        window_hint(GLFW_WIN32_SHOWDEFAULT, win32.showdefault);

        // cocoa
        window_hint(GLFW_COCOA_FRAME_NAME, cocoa.frame_name);
        window_hint(GLFW_COCOA_GRAPHICS_SWITCHING, cocoa.graphics_switching);

        // wayland
        window_hint(GLFW_WAYLAND_APP_ID, wl.app_id);

        // x11
        window_hint(GLFW_X11_CLASS_NAME, x11.class_name);
        window_hint(GLFW_X11_INSTANCE_NAME, x11.instance_name);
    }
}

// callbacks
namespace glfw_cpp
{
    void Instance::window_pos_callback(GLFWwindow* window, int x, int y)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            auto  prev   = window.properties().position;
            glfw_cpp::Instance::get().push_event(
                window,
                glfw_cpp::event::WindowMoved{
                    .x  = x,
                    .y  = y,
                    .dx = x - prev.x,
                    .dy = y - prev.y,
                }
            );
        }
    }
    void Instance::window_size_callback(GLFWwindow* window, int width, int height)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            auto  prev   = window.properties().dimensions;
            glfw_cpp::Instance::get().push_event(
                window,
                event::WindowResized{
                    .width         = width,
                    .height        = height,
                    .width_change  = width - prev.width,
                    .height_change = height - prev.height,
                }
            );
        }
    }

    void Instance::window_close_callback(GLFWwindow* window)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(window, event::WindowClosed{});
        }
    }

    void Instance::window_refresh_callback(GLFWwindow* window)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(window, event::WindowRefreshed{});
        }
    }

    void Instance::window_focus_callback(GLFWwindow* window, int focused)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(
                window,
                event::WindowFocused{
                    .focused = focused == GLFW_TRUE,
                }
            );
        }
    }

    void Instance::window_iconify_callback(GLFWwindow* window, int iconified)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(
                window,
                event::WindowIconified{
                    .iconified = iconified == GLFW_TRUE,
                }
            );
        }
    }

    void Instance::framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            auto  prev   = window.properties().framebuffer_size;
            glfw_cpp::Instance::get().push_event(
                window,
                event::FramebufferResized{
                    .width         = width,
                    .height        = height,
                    .width_change  = width - prev.width,
                    .height_change = height - prev.height,
                }
            );
        }
    }

    void Instance::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(
                window,
                event::ButtonPressed{
                    .button = static_cast<MouseButton>(button),
                    .state  = static_cast<MouseButtonState>(action),
                    .mods   = ModifierKey{ mods },
                }
            );
        }
    }

    void Instance::cursor_pos_callback(GLFWwindow* window, double x, double y)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            auto  prev   = window.properties().cursor_position;
            glfw_cpp::Instance::get().push_event(
                window,
                event::CursorMoved{
                    .x  = x,
                    .y  = y,
                    .dx = x - prev.x,
                    .dy = y - prev.y,
                }
            );
        }
    }

    void Instance::cursor_enter_callback(GLFWwindow* window, int entered)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(
                window,
                event::CursorEntered{
                    .entered = entered == GLFW_TRUE,
                }
            );
        }
    }

    void Instance::scroll_callback(GLFWwindow* window, double x, double y)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(
                window,
                event::Scrolled{
                    .dx = x,
                    .dy = y,
                }
            );
        }
    }

    void Instance::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(
                window,
                event::KeyPressed{
                    .key      = static_cast<KeyCode>(key),
                    .scancode = scancode,
                    .state    = static_cast<KeyState>(action),
                    .mods     = ModifierKey{ mods },
                }
            );
        }
    }

    void Instance::char_callback(GLFWwindow* window, unsigned int codepoint)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(
                window,
                event::CharInput{
                    .codepoint = codepoint,
                }
            );
        }
    }

    void Instance::file_drop_callback(GLFWwindow* window, int count, const char** paths)
    {
        auto paths_vec = std::vector<std::filesystem::path>(static_cast<std::size_t>(count));
        for (std::size_t i = 0; i < paths_vec.size(); ++i) {
            paths_vec[i] = std::filesystem::path{ paths[i] };
        }
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(
                window,
                event::FileDropped{
                    .files = std::move(paths_vec),
                }
            );
        }
    }

    void Instance::window_maximize_callback(GLFWwindow* window, int maximized)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(
                window,
                event::WindowMaximized{
                    .maximized = maximized == GLFW_TRUE,
                }
            );
        }
    }

    void Instance::window_content_scale_callback(GLFWwindow* window, float xscale, float yscale)
    {
        if (auto* ptr = glfwGetWindowUserPointer(window); ptr != nullptr) {
            auto& window = *static_cast<glfw_cpp::Window*>(ptr);
            glfw_cpp::Instance::get().push_event(
                window,
                event::WindowScaleChanged{
                    .x = xscale,
                    .y = yscale,
                }
            );
        }
    }
}

namespace glfw_cpp
{
    Instance::~Instance()
    {
        // flush task queue first (there might be window deletion request)
        run_tasks();

        for (auto handle : m_windows) {
            glfwDestroyWindow(handle);
        }

        // this might fail, how should I report the failure?
        glfwTerminate();

        // unregister the gloal instance pointer
        Instance::s_instance = nullptr;
    }

    Instance& Instance::get()
    {
        assert(s_instance != nullptr && "Instance not initialized!");
        return *s_instance;
    }

    void Instance::push_event(Window& window, Event event) noexcept
    {
        auto forward = true;

        if (m_event_interceptor) {
            auto& intr = *m_event_interceptor;

            forward = event.visit(event::Overload{
                // clang-format off
                [&](event::WindowMoved&        event) { return intr.on_window_moved        (window, event); },
                [&](event::WindowResized&      event) { return intr.on_window_resized      (window, event); },
                [&](event::WindowClosed&       event) { return intr.on_window_closed       (window, event); },
                [&](event::WindowRefreshed&    event) { return intr.on_window_refreshed    (window, event); },
                [&](event::WindowFocused&      event) { return intr.on_window_focused      (window, event); },
                [&](event::WindowIconified&    event) { return intr.on_window_iconified    (window, event); },
                [&](event::WindowMaximized&    event) { return intr.on_window_maximized    (window, event); },
                [&](event::WindowScaleChanged& event) { return intr.on_window_scale_changed(window, event); },
                [&](event::FramebufferResized& event) { return intr.on_framebuffer_resized (window, event); },
                [&](event::ButtonPressed&      event) { return intr.on_button_pressed      (window, event); },
                [&](event::CursorMoved&        event) { return intr.on_cursor_moved        (window, event); },
                [&](event::CursorEntered&      event) { return intr.on_cursor_entered      (window, event); },
                [&](event::Scrolled&           event) { return intr.on_scrolled            (window, event); },
                [&](event::KeyPressed&         event) { return intr.on_key_pressed         (window, event); },
                [&](event::CharInput&          event) { return intr.on_char_input          (window, event); },
                [&](event::FileDropped&        event) { return intr.on_file_dropped        (window, event); },
                [&](event::Empty&                   ) { return true; /* always true                      */ },
                // clang-format on
            });
        }

        if (forward) {
            window.push_event(std::move(event));
        }
    }

    void Instance::validate_access() const
    {
        if (m_attached_thread_id != std::this_thread::get_id()) {
            throw error::WrongThreadAccess{
                util::get_thread_num(m_attached_thread_id),
                util::get_thread_num(std::this_thread::get_id()),
            };
        }
    }

    void Instance::run_tasks()
    {
        auto [deletion, tasks] = [&] {
            auto lock = std::scoped_lock{ m_mutex };
            return std::pair{ std::exchange(m_window_delete_queue, {}), std::exchange(m_task_queue, {}) };
        }();

        for (auto&& task : tasks) {
            task();
        }

        // window deletion
        for (auto handle : deletion) {
            if (std::erase(m_windows, handle) != 0) {
                glfwDestroyWindow(handle);
                util::check_glfw_error();
            }
        }
    }

    void Instance::apply_hints(const PartialHints& hints)
    {
        auto adapter = util::VisitOverloaded{
            [](int enumm, const std::optional<bool>& value) {
                value ? glfwWindowHint(enumm, *value ? GLFW_TRUE : GLFW_FALSE) : void();
            },
            [](int enumm, const std::optional<int>& value) {
                value ? glfwWindowHint(enumm, *value) : void();
            },
            [](int enumm, const std::optional<const char*>& value) {
                value ? glfwWindowHintString(enumm, *value) : void();
            },
            []<typename T>(int enumm, const std::optional<T>& value) {
                value ? glfwWindowHint(enumm, static_cast<int>(*value)) : void();
            },
        };
        apply_hints_impl(hints, adapter);
    }

    void Instance::apply_hints_full(const FullHints& hints)
    {
        auto adapter = util::VisitOverloaded{
            [](int enumm, bool value) { glfwWindowHint(enumm, value ? GLFW_TRUE : GLFW_FALSE); },
            [](int enumm, int value) { glfwWindowHint(enumm, value); },
            [](int enumm, const char* value) { glfwWindowHintString(enumm, value); },
            []<typename T>(int enumm, const T& value) { glfwWindowHint(enumm, static_cast<int>(value)); },
        };
        apply_hints_impl(hints, adapter);
    }

    void Instance::apply_hints_default()
    {
        glfwDefaultWindowHints();
    }

    Window Instance::create_window(
        int              width,
        int              height,
        std::string_view title,
        GLFWmonitor*     monitor,
        GLFWwindow*      share
    )
    {
        validate_access();

        const auto handle = glfwCreateWindow(width, height, title.data(), monitor, share);
        if (handle == nullptr) {
            util::throw_glfw_error();
        }
        m_windows.emplace_back(handle);

        glfwSetWindowPosCallback(handle, window_pos_callback);
        glfwSetWindowSizeCallback(handle, window_size_callback);
        glfwSetWindowCloseCallback(handle, window_close_callback);
        glfwSetWindowRefreshCallback(handle, window_refresh_callback);
        glfwSetWindowFocusCallback(handle, window_focus_callback);
        glfwSetWindowIconifyCallback(handle, window_iconify_callback);
        glfwSetFramebufferSizeCallback(handle, framebuffer_size_callback);
        glfwSetMouseButtonCallback(handle, mouse_button_callback);
        glfwSetCursorPosCallback(handle, cursor_pos_callback);
        glfwSetCursorEnterCallback(handle, cursor_enter_callback);
        glfwSetScrollCallback(handle, scroll_callback);
        glfwSetKeyCallback(handle, key_callback);
        glfwSetCharCallback(handle, char_callback);
        glfwSetDropCallback(handle, file_drop_callback);
        glfwSetWindowMaximizeCallback(handle, window_maximize_callback);
        glfwSetWindowContentScaleCallback(handle, window_content_scale_callback);

        int    real_width, real_height, fb_width, fb_height;
        double x_cursor, y_cursor;
        glfwGetWindowSize(handle, &real_width, &real_height);
        glfwGetCursorPos(handle, &x_cursor, &y_cursor);
        glfwGetFramebufferSize(handle, &fb_width, &fb_height);

        int x_pos = 0, y_pos = 0;
        if (platform() != hint::Platform::Wayland) {    // emits GLFW_FEATURE_UNAVAILABLE on wayland
            glfwGetWindowPos(handle, &x_pos, &y_pos);
        }

        util::check_glfw_error();

        auto properties = Properties{
            .title              = { title.begin(), title.end() },
            .position           = { x_pos, y_pos },
            .dimensions         = { real_width, real_height },
            .framebuffer_size   = { fb_width, fb_height },
            .cursor_position    = { x_cursor, y_cursor },
            .mouse_button_state = {},
            .key_state          = {},
            .monitor            = monitor,
        };

        auto attributes = Attributes{
            .focused                 = glfwGetWindowAttrib(handle, GLFW_FOCUSED) == GLFW_TRUE,
            .iconified               = glfwGetWindowAttrib(handle, GLFW_ICONIFIED) == GLFW_TRUE,
            .maximized               = glfwGetWindowAttrib(handle, GLFW_MAXIMIZED) == GLFW_TRUE,
            .hovered                 = glfwGetWindowAttrib(handle, GLFW_HOVERED) == GLFW_TRUE,
            .visible                 = glfwGetWindowAttrib(handle, GLFW_VISIBLE) == GLFW_TRUE,
            .resizable               = glfwGetWindowAttrib(handle, GLFW_RESIZABLE) == GLFW_TRUE,
            .decorated               = glfwGetWindowAttrib(handle, GLFW_DECORATED) == GLFW_TRUE,
            .auto_iconify            = glfwGetWindowAttrib(handle, GLFW_AUTO_ICONIFY) == GLFW_TRUE,
            .floating                = glfwGetWindowAttrib(handle, GLFW_FLOATING) == GLFW_TRUE,
            .transparent_framebuffer = glfwGetWindowAttrib(handle, GLFW_TRANSPARENT_FRAMEBUFFER) == GLFW_TRUE,
            .focus_on_show           = glfwGetWindowAttrib(handle, GLFW_FOCUS_ON_SHOW) == GLFW_TRUE,
            .mouse_passthrough       = glfwGetWindowAttrib(handle, GLFW_MOUSE_PASSTHROUGH) == GLFW_TRUE,
        };

        util::check_glfw_error();

        return Window{ handle, std::move(properties), std::move(attributes) };
    }

    bool Instance::has_window_opened() const noexcept
    {
        return std::ranges::any_of(m_windows, [](auto* h) { return glfwWindowShouldClose(h) != GLFW_TRUE; });
    }

    void Instance::poll_events(std::optional<std::chrono::milliseconds> poll_rate)
    {
        validate_access();

        if (poll_rate) {
            auto sleep_until_time = std::chrono::steady_clock::now() + *poll_rate;

            glfwPollEvents();
            util::check_glfw_error();
            run_tasks();

            if (sleep_until_time > std::chrono::steady_clock::now()) {
                std::this_thread::sleep_until(sleep_until_time);
            }
        } else {
            glfwPollEvents();
            util::check_glfw_error();
            run_tasks();
        }
    }

    void Instance::wait_events(std::optional<std::chrono::milliseconds> timeout)
    {
        validate_access();
        if (timeout) {
            using SecondsDouble = std::chrono::duration<double>;
            const auto seconds  = std::chrono::duration_cast<SecondsDouble>(*timeout);
            glfwWaitEventsTimeout(seconds.count());
            util::check_glfw_error();
        } else {
            glfwWaitEvents();
            util::check_glfw_error();
        }
        run_tasks();
    }

    void Instance::request_delete_window(GLFWwindow* handle) noexcept
    {
        auto lock = std::unique_lock{ m_mutex };
        m_window_delete_queue.push_back(handle);
    }

    void Instance::enqueue_task(std::function<void()>&& task) noexcept
    {
        auto lock = std::unique_lock{ m_mutex };
        m_task_queue.emplace_back(std::move(task));
    }

    hint::Platform Instance::platform() const noexcept
    {
        auto platform = glfwGetPlatform();
        assert(platform != 0);
        return static_cast<hint::Platform>(platform);
    }

    bool platform_supported(hint::Platform platform)
    {
        auto res = glfwPlatformSupported(static_cast<int>(platform));
        util::check_glfw_error();
        return res;
    }

    gl::Proc get_proc_address(const char* procname)
    {
        auto addr = glfwGetProcAddress(procname);
        util::check_glfw_error();
        return addr;
    }

    gl::Proc get_proc_address_noexcept(const char* procname) noexcept
    {
        return glfwGetProcAddress(procname);
    }

    bool extension_supported(const char* extension)
    {
        auto res = glfwExtensionSupported(extension) == GLFW_TRUE;
        util::check_glfw_error();
        return res;
    }

    bool extension_supported_noexcept(const char* extension) noexcept
    {
        return glfwExtensionSupported(extension) == GLFW_TRUE;
    }

    Instance::Unique init(const InitHints& hints)
    {
        if (Instance::s_instance) {
            throw error::AlreadyInitialized{};
        }

        // bare new since the constructor is private
        auto instance = std::unique_ptr<Instance>{ new Instance{} };

        // register the newly created instance to the global pointer
        Instance::s_instance = instance.get();

        glfwSetErrorCallback([](int err, const char* msg) {
            auto& instance = Instance::get();
            if (instance.m_callback) {
                instance.m_callback(static_cast<ErrorCode>(err), msg);
            }
        });

        glfwInitHint(GLFW_PLATFORM, static_cast<int>(hints.platform));
        glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, hints.joystick_hat_buttons ? GLFW_TRUE : GLFW_FALSE);
        glfwInitHint(GLFW_ANGLE_PLATFORM_TYPE, static_cast<int>(hints.angle_platform_type));
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, hints.cocoa_chdir_resource ? GLFW_TRUE : GLFW_FALSE);
        glfwInitHint(GLFW_COCOA_MENUBAR, hints.cocoa_menubar ? GLFW_TRUE : GLFW_FALSE);
        glfwInitHint(GLFW_WAYLAND_LIBDECOR, static_cast<int>(hints.wayland_libdecor));
        glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, hints.x11_xcb_vulkan_surface ? GLFW_TRUE : GLFW_FALSE);

        if (glfwInit() != GLFW_TRUE) {
            instance.reset();
            util::throw_glfw_error();
        }

        return instance;
    }

    void make_current(GLFWwindow* window)
    {
#if __EMSCRIPTEN__
        // emscripten-glfw which is the one used for this library emits an error if window is nullptr,
        // see: https://github.com/pongasoft/emscripten-glfw/issues/24

        window ? glfwMakeContextCurrent(window) : void();
        util::check_glfw_error();
#else
        glfwMakeContextCurrent(window);
        util::check_glfw_error();
#endif
    }

    GLFWwindow* get_current()
    {
        auto current = glfwGetCurrentContext();
        util::check_glfw_error();
        return current;
    }
}
