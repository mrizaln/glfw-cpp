#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/window.hpp"

#include "util.hpp"
#include <thread>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <utility>

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
            throw WrongThreadAccess{
                util::get_thread_num(m_attached_thread_id),
                util::get_thread_num(std::this_thread::get_id()),
            };
        }
    }

    void Instance::run_tasks()
    {
        for (auto&& task : util::lock_exchange(m_mutex, m_task_queue, {})) {
            task();
        }

        // window deletion
        for (auto handle : util::lock_exchange(m_mutex, m_window_delete_queue, {})) {
            if (std::erase(m_windows, handle) != 0) {
                glfwDestroyWindow(handle);
                util::check_glfw_error();
            }
        }
    }

    void Instance::apply_hint(const Hint& hint)
    {
        const auto& [api, win, fb, mon, win32, cocoa, wl, x11] = hint;

        // context
        api.visit(util::VisitOverloaded{
            [](const api::OpenGL& api) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.version_major);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.version_minor);
                glfwWindowHint(GLFW_CONTEXT_CREATION_API, static_cast<int>(api.creation_api));
                glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, static_cast<int>(api.robustness));
                glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, static_cast<int>(api.release_behavior));
                glfwWindowHint(GLFW_CONTEXT_DEBUG, api.debug ? GLFW_TRUE : GLFW_FALSE);
                glfwWindowHint(GLFW_CONTEXT_NO_ERROR, api.no_error ? GLFW_TRUE : GLFW_FALSE);

                if (api.version_major >= 3 and api.version_minor >= 0) {
                    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, api.forward_compat ? GLFW_TRUE : GLFW_FALSE);
                }
                if (api.version_major >= 3 and api.version_minor >= 3) {
                    glfwWindowHint(GLFW_OPENGL_PROFILE, static_cast<int>(api.profile));
                }
            },
            [](const api::OpenGLES& api) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.version_major);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.version_minor);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
                glfwWindowHint(GLFW_CONTEXT_CREATION_API, static_cast<int>(api.creation_api));
                glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, static_cast<int>(api.robustness));
                glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, static_cast<int>(api.release_behavior));
                glfwWindowHint(GLFW_CONTEXT_DEBUG, api.debug ? GLFW_TRUE : GLFW_FALSE);
                glfwWindowHint(GLFW_CONTEXT_NO_ERROR, api.no_error ? GLFW_TRUE : GLFW_FALSE);
            },
            [](const api::NoApi&) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);    //
            },
        });

        // window
        glfwWindowHint(GLFW_RESIZABLE, win.resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, win.visible ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, win.decorated ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUSED, win.focused ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_AUTO_ICONIFY, win.auto_iconify ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_FLOATING, win.floating ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, win.maximized ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_CENTER_CURSOR, win.center_cursor ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, win.transparent_framebuffer ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, win.focus_on_show ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, win.scale_to_monitor ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, win.scale_framebuffer ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, win.mouse_passthrough ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_POSITION_X, win.position_x);
        glfwWindowHint(GLFW_POSITION_Y, win.position_y);

        // framebuffer
        glfwWindowHint(GLFW_RED_BITS, fb.red_bits);
        glfwWindowHint(GLFW_GREEN_BITS, fb.green_bits);
        glfwWindowHint(GLFW_BLUE_BITS, fb.blue_bits);
        glfwWindowHint(GLFW_ALPHA_BITS, fb.alpha_bits);
        glfwWindowHint(GLFW_DEPTH_BITS, fb.depth_bits);
        glfwWindowHint(GLFW_STENCIL_BITS, fb.stencil_bits);
        glfwWindowHint(GLFW_SAMPLES, fb.samples);
        glfwWindowHint(GLFW_STEREO, fb.stereo ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_SRGB_CAPABLE, fb.srgb_capable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, fb.doublebuffer ? GLFW_TRUE : GLFW_FALSE);

        // monitor
        glfwWindowHint(GLFW_REFRESH_RATE, mon.refresh_rate);

        // win32
        glfwWindowHint(GLFW_WIN32_KEYBOARD_MENU, win32.keyboard_menu ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_WIN32_SHOWDEFAULT, win32.showdefault ? GLFW_TRUE : GLFW_FALSE);

        // cocoa
        glfwWindowHintString(GLFW_COCOA_FRAME_NAME, cocoa.frame_name);
        glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, cocoa.graphics_switching ? GLFW_TRUE : GLFW_FALSE);

        // wayland
        glfwWindowHintString(GLFW_WAYLAND_APP_ID, wl.app_id);

        // x11
        glfwWindowHintString(GLFW_X11_CLASS_NAME, x11.class_name);
        glfwWindowHintString(GLFW_X11_INSTANCE_NAME, x11.instance_name);
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

        int    x_pos, y_pos, real_width, real_height, fb_width, fb_height;
        double x_cursor, y_cursor;
        glfwGetWindowPos(handle, &x_pos, &y_pos);
        glfwGetWindowSize(handle, &real_width, &real_height);
        glfwGetCursorPos(handle, &x_cursor, &y_cursor);
        glfwGetFramebufferSize(handle, &fb_width, &fb_height);

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

    gl::Proc get_proc_address(const char* procname) noexcept
    {
        return glfwGetProcAddress(procname);
    }

    bool extension_supported(const char* extension) noexcept
    {
        return glfwExtensionSupported(extension) == GLFW_TRUE;
    }

    std::unique_ptr<Instance> init(const InitHint& hint)
    {
        if (Instance::s_instance) {
            throw AlreadyInitialized{};
        }

        // bare new since the constructor is private
        auto instance = std::unique_ptr<Instance>{ new Instance{} };

        // register the newly created instance to the global pointer
        Instance::s_instance = instance.get();

        glfwSetErrorCallback([](int err, const char* msg) {
            auto& instance = Instance::get();
            if (instance.m_callback) {
                instance.m_callback(err, msg);
            }
        });

        glfwInitHint(GLFW_PLATFORM, static_cast<int>(hint.platform));
        glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, hint.joystick_hat_buttons ? GLFW_TRUE : GLFW_FALSE);
        glfwInitHint(GLFW_ANGLE_PLATFORM_TYPE, static_cast<int>(hint.angle_platform_type));
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, hint.cocoa_chdir_resource ? GLFW_TRUE : GLFW_FALSE);
        glfwInitHint(GLFW_COCOA_MENUBAR, hint.cocoa_menubar ? GLFW_TRUE : GLFW_FALSE);
        glfwInitHint(GLFW_WAYLAND_LIBDECOR, static_cast<int>(hint.wayland_libdecor));
        glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, hint.x11_xcb_vulkan_surface ? GLFW_TRUE : GLFW_FALSE);

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
