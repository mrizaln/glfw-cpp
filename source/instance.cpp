#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/window.hpp"

#include "util.hpp"
#include <thread>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <format>
#include <utility>

namespace
{
    void configure_hints(const glfw_cpp::Hint& hint) noexcept
    {
        using F = glfw_cpp::Flag;

        const auto set_flag = [&](int flag, F bit) {
            glfwWindowHint(flag, (hint.flags & bit) == bit ? GLFW_TRUE : GLFW_FALSE);
        };

        set_flag(GLFW_RESIZABLE, F::Resizable);
        set_flag(GLFW_VISIBLE, F::Visible);
        set_flag(GLFW_DECORATED, F::Decorated);
        set_flag(GLFW_FOCUSED, F::Focused);
        set_flag(GLFW_AUTO_ICONIFY, F::AutoIconify);
        set_flag(GLFW_FLOATING, F::Floating);
        set_flag(GLFW_MAXIMIZED, F::Maximized);
        set_flag(GLFW_CENTER_CURSOR, F::CenterCursor);
        set_flag(GLFW_TRANSPARENT_FRAMEBUFFER, F::TransparentFramebuffer);
        set_flag(GLFW_FOCUS_ON_SHOW, F::FocusOnShow);
        set_flag(GLFW_SCALE_TO_MONITOR, F::ScaleToMonitor);

        glfwWindowHint(GLFW_RED_BITS, hint.red_bits);
        glfwWindowHint(GLFW_GREEN_BITS, hint.green_bits);
        glfwWindowHint(GLFW_BLUE_BITS, hint.blue_bits);
        glfwWindowHint(GLFW_ALPHA_BITS, hint.alpha_bits);
        glfwWindowHint(GLFW_DEPTH_BITS, hint.depth_bits);
        glfwWindowHint(GLFW_STENCIL_BITS, hint.stencil_bits);

        glfwWindowHint(GLFW_SAMPLES, hint.samples);
        glfwWindowHint(GLFW_REFRESH_RATE, hint.refresh_rate);
    }

    std::string_view error_to_string(int glfw_errc)
    {
#define CASE_ENTRY(Val)                                                                                      \
    case Val: return #Val

        switch (glfw_errc) {
            CASE_ENTRY(GLFW_NO_ERROR);
            CASE_ENTRY(GLFW_NOT_INITIALIZED);
            CASE_ENTRY(GLFW_NO_CURRENT_CONTEXT);
            CASE_ENTRY(GLFW_INVALID_ENUM);
            CASE_ENTRY(GLFW_INVALID_VALUE);
            CASE_ENTRY(GLFW_OUT_OF_MEMORY);
            CASE_ENTRY(GLFW_API_UNAVAILABLE);
            CASE_ENTRY(GLFW_VERSION_UNAVAILABLE);
            CASE_ENTRY(GLFW_PLATFORM_ERROR);
            CASE_ENTRY(GLFW_FORMAT_UNAVAILABLE);
            CASE_ENTRY(GLFW_NO_WINDOW_CONTEXT);
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 4
            CASE_ENTRY(GLFW_CURSOR_UNAVAILABLE);
            CASE_ENTRY(GLFW_FEATURE_UNAVAILABLE);
            CASE_ENTRY(GLFW_FEATURE_UNIMPLEMENTED);
            CASE_ENTRY(GLFW_PLATFORM_UNAVAILABLE);
#endif
        default: return "UNKNOWN_ERROR";
#undef CASE_ENTRY
        }
    }
}

namespace glfw_cpp
{
    Instance::~Instance()
    {
        run_tasks();    // flush task queue first

        for (auto handle : m_windows) {
            glfwDestroyWindow(handle);    // delete all windows
            Instance::log_i("(Instance) Window ({:#x}) deleted", (std::size_t)handle);
        }

        glfwTerminate();    // this might fail, how should I report the failure?
        Instance::log_i("(Instance) Successfully terminated glfw-cpp");

        // unregister the gloal instance pointer
        Instance::s_instance = nullptr;
    }

    Instance& Instance::get()
    {
        assert(s_instance != nullptr && "Instance not initialized!");
        return *s_instance;
    }

    void Instance::log(LogLevel level, std::string msg) noexcept
    {
        if (auto& instance = get(); instance.m_loggger) {
            instance.m_loggger(level, std::move(msg));
        }
    }

    bool Instance::send_intercept_event(Window& window, Event& event) noexcept
    {
        if (!m_event_interceptor) {
            return true;
        }

        auto& intr = *m_event_interceptor;
        return event.visit(event::Overload{
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
            [&](event::Empty&                   ) { return true; /* always true                   */ },
            // clang-format on
        });
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
                Instance::log_i("(Instance) Window ({:#x}) deleted", (std::size_t)handle);
            }
        }
    }

    Window Instance::create_window(
        const Hint&      hint,
        std::string_view title,
        int              width,
        int              height,
        bool             bind_immediately
    )
    {
        validate_access();
        configure_hints(hint);
        util::check_glfw_error();

        const auto handle = glfwCreateWindow(width, height, title.data(), hint.monitor, hint.share);
        if (handle == nullptr) {
            Instance::log_c("(Instance) Window creation failed");
            util::throw_glfw_error();
        }
        m_windows.emplace_back(handle);

        Instance::log_i("(Instance) Window ({:#x}) created", (std::size_t)handle);

        glfwSetWindowPosCallback(handle, Window::window_pos_callback);
        glfwSetWindowSizeCallback(handle, Window::window_size_callback);
        glfwSetWindowCloseCallback(handle, Window::window_close_callback);
        glfwSetWindowRefreshCallback(handle, Window::window_refresh_callback);
        glfwSetWindowFocusCallback(handle, Window::window_focus_callback);
        glfwSetWindowIconifyCallback(handle, Window::window_iconify_callback);
        glfwSetFramebufferSizeCallback(handle, Window::framebuffer_size_callback);
        glfwSetMouseButtonCallback(handle, Window::mouse_button_callback);
        glfwSetCursorPosCallback(handle, Window::cursor_pos_callback);
        glfwSetCursorEnterCallback(handle, Window::cursor_enter_callback);
        glfwSetScrollCallback(handle, Window::scroll_callback);
        glfwSetKeyCallback(handle, Window::key_callback);
        glfwSetCharCallback(handle, Window::char_callback);
        glfwSetDropCallback(handle, Window::file_drop_callback);
        glfwSetWindowMaximizeCallback(handle, Window::window_maximize_callback);
        glfwSetWindowContentScaleCallback(handle, Window::window_content_scale_callback);

        int    x_pos, y_pos, real_width, real_height, fb_width, fb_height;
        double x_cursor, y_cursor;
        glfwGetWindowPos(handle, &x_pos, &y_pos);
        glfwGetWindowSize(handle, &real_width, &real_height);
        glfwGetCursorPos(handle, &x_cursor, &y_cursor);
        glfwGetFramebufferSize(handle, &fb_width, &fb_height);

        return Window{ handle, Properties{
            .title              = { title.begin(), title.end() },
            .pos                = { x_pos, y_pos },
            .dimensions         = { real_width, real_height },
            .framebuffer_size   = { fb_width, fb_height },
            .cursor             = { x_cursor, y_cursor },
            .attribute          = {
                .iconified      = 0,
                .maximized      = (hint.flags & Flag::Maximized) == Flag::Maximized,
                .focused        = (hint.flags & Flag::Focused) == Flag::Focused,
                .visible        = (hint.flags & Flag::Visible) == Flag::Visible,
                .hovered        = (unsigned int)glfwGetWindowAttrib(handle, GLFW_HOVERED),
                .resizable      = (hint.flags & Flag::Resizable) == Flag::Resizable,
                .floating       = (hint.flags & Flag::Floating) == Flag::Floating,
                .auto_iconify   = (hint.flags & Flag::AutoIconify) == Flag::AutoIconify,
                .focus_on_show  = (hint.flags & Flag::FocusOnShow) == Flag::FocusOnShow,
            },
            .mouse_button_state = {},
            .key_state          = {},
            .monitor            = hint.monitor,
        }, bind_immediately };
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

    std::unique_ptr<Instance> init(Api&& api, Instance::LogFun&& logger)
    {
        if (Instance::s_instance) {
            throw AlreadyInitialized{};
        }

        // bare new since the constructor is private
        auto instance = std::unique_ptr<Instance>{ new Instance{} };

        // register the newly created instance to the global pointer
        Instance::s_instance = instance.get();

        instance->m_api                = std::move(api);
        instance->m_loggger            = std::move(logger);
        instance->m_attached_thread_id = std::this_thread::get_id();

        glfwSetErrorCallback([](int err, const char* msg) {
            Instance::log_c("[{}|{:#010x}] {}", error_to_string(err), err, msg);
        });

        if (glfwInit() != GLFW_TRUE) {
            instance.reset();
            util::throw_glfw_error();
        }

        instance->m_api.visit(util::VisitOverloaded{
            [](api::OpenGL& api) {
                if (api.loader == nullptr) {
#if not __EMSCRIPTEN__
                    throw EmptyLoader{};
#endif
                }

                auto gl_profile = [&] {
                    using P = glfw_cpp::api::gl::Profile;
                    switch (api.profile) {
                    case P::Core: return GLFW_OPENGL_CORE_PROFILE;
                    case P::Compat: return GLFW_OPENGL_COMPAT_PROFILE;
                    case P::Any: return GLFW_OPENGL_ANY_PROFILE;
                    default: [[unlikely]] return GLFW_OPENGL_CORE_PROFILE;
                    }
                }();

                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.major);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.minor);

                if (api.major >= 3 && api.minor >= 0) {
                    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, api.forward_compat);
                }

                if (api.major >= 3 && api.minor >= 2) {
                    glfwWindowHint(GLFW_OPENGL_PROFILE, gl_profile);
                }
            },
            [](api::OpenGLES& api) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.major);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.minor);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
            },
            [](api::NoApi&) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);    //
            },
        });

        Instance::log_i("(Instance) Successfully initialized glfw-cpp");

        return instance;
    }

    void make_current(GLFWwindow* window)
    {
        glfwMakeContextCurrent(window);
        util::check_glfw_error();
    }

    GLFWwindow* get_current()
    {
        auto current = glfwGetCurrentContext();
        util::check_glfw_error();
        return current;
    }
}
