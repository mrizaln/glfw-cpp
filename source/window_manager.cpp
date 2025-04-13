#include "glfw_cpp/window_manager.hpp"
#include "glfw_cpp/instance.hpp"

#include "util.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <algorithm>
#include <chrono>
#include <format>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

using LogLevel = glfw_cpp::Instance::LogLevel;

namespace
{
    void configure_hints(const glfw_cpp::WindowHint& hint) noexcept
    {
        using F = glfw_cpp::WindowHint::FlagBit;

        const auto set_flag = [&](int flag, F bit) {
            glfwWindowHint(flag, (hint.flags & bit) != 0 ? GLFW_TRUE : GLFW_FALSE);
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
}

namespace glfw_cpp
{
    void WindowManager::GLFWwindowDeleter::operator()(GLFWwindow* handle) const noexcept
    {
        glfwDestroyWindow(handle);
    };

    WindowManager::WindowManager(std::thread::id thread_id, IEventInterceptor* event_interceptor) noexcept
        : m_attached_thread_id{ thread_id }
        , m_event_interceptor{ event_interceptor }
    {
        // yeah, that's it.
        // attached thread id will not be changed for the lifetime of this class instance.
    }

    Window WindowManager::create_window(
        const WindowHint& hint,
        std::string_view  title,
        int               width,
        int               height,
        bool              bind_immediately
    )
    {
        validate_access();
        configure_hints(hint);

        const auto handle = glfwCreateWindow(
            width,
            height,
            title.data(),
            hint.monitor ? hint.monitor->handle() : nullptr,
            hint.share ? hint.share->handle() : nullptr
        );
        if (handle == nullptr) {
            Instance::log_c("(WindowManager) Window creation failed");
            util::throw_glfw_error();
        }
        m_windows.emplace_back(handle);

        Instance::log_i("(WindowManager) Window ({:#x}) created", (std::size_t)handle);

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

        auto wm_copy = std::enable_shared_from_this<WindowManager>::shared_from_this();

        return Window{ wm_copy, handle, Window::Properties{
            .title             = { title.begin(), title.end() },
            .pos               = { x_pos, y_pos },
            .dimension         = { real_width, real_height },
            .framebuffer_size  = { fb_width, fb_height },
            .cursor            = { x_cursor, y_cursor },
            .attribute         = {
                .iconified     = 0,
                .maximized     = (hint.flags & WindowHint::Maximized) != 0,
                .focused       = (hint.flags & WindowHint::Focused) != 0,
                .visible       = (hint.flags & WindowHint::Visible) != 0,
                .hovered       = (unsigned int)glfwGetWindowAttrib(handle, GLFW_HOVERED),
                .resizable     = (hint.flags & WindowHint::Resizable) != 0,
                .floating      = (hint.flags & WindowHint::Floating) != 0,
                .auto_iconify  = (hint.flags & WindowHint::AutoIconify) != 0,
                .focus_on_show = (hint.flags & WindowHint::FocusOnShow) != 0,
            },
        }, bind_immediately };
    }

    bool WindowManager::has_window_opened()
    {
        using Ptr                   = decltype(m_windows)::value_type;
        const auto should_not_close = [](Window::Handle h) { return glfwWindowShouldClose(h) != GLFW_TRUE; };
        return std::ranges::any_of(m_windows, should_not_close, &Ptr::get);
    }

    void WindowManager::poll_events(std::optional<std::chrono::milliseconds> poll_rate)
    {
        validate_access();

        if (poll_rate) {
            auto sleep_until_time = std::chrono::steady_clock::now() + *poll_rate;

            glfwPollEvents();
            check_tasks();

            if (sleep_until_time > std::chrono::steady_clock::now()) {
                std::this_thread::sleep_until(sleep_until_time);
            }
        } else {
            glfwPollEvents();
            check_tasks();
        }
    }

    void WindowManager::wait_events(std::optional<std::chrono::milliseconds> timeout)
    {
        validate_access();
        if (timeout) {
            using SecondsDouble = std::chrono::duration<double>;
            const auto seconds  = std::chrono::duration_cast<SecondsDouble>(*timeout);
            glfwWaitEventsTimeout(seconds.count());
        } else {
            glfwWaitEvents();
        }
        check_tasks();
    }

    void WindowManager::request_delete_window(Window::Handle handle)
    {
        std::unique_lock lock{ m_mutex };

        // accept request only for available windows
        if (std::ranges::find(m_windows, handle, &UniqueGLFWwindow::get) != m_windows.end()) {
            m_window_delete_queue.push_back(handle);
        }
    }

    void WindowManager::enqueue_window_task(Window::Handle handle, Fun<void()>&& task)
    {
        std::unique_lock lock{ m_mutex };
        m_window_task_queue.emplace_back(handle, std::move(task));
    }

    void WindowManager::enqueue_task(Fun<void()>&& task)
    {
        std::unique_lock lock{ m_mutex };
        m_task_queue.emplace_back(std::move(task));
    }

    void WindowManager::validate_access() const
    {
        if (m_attached_thread_id != std::this_thread::get_id()) {
            throw WrongThreadAccess{
                util::get_thread_num(m_attached_thread_id),
                util::get_thread_num(std::this_thread::get_id()),
            };
        }
    }

    void WindowManager::check_tasks()
    {
        // window deletion
        for (auto handle : util::lock_exchange(m_mutex, m_window_delete_queue, {})) {
            if (std::erase_if(m_windows, [handle](auto& e) { return e.get() == handle; }) != 0) {
                Instance::log_i("(WindowManager) Window ({:#x}) deleted", (std::size_t)handle);
            }
        }

        // window task requests
        for (auto&& [handle, task] : util::lock_exchange(m_mutex, m_window_task_queue, {})) {
            if (std::ranges::find(m_windows, handle, &UniqueGLFWwindow::get) != m_windows.end()) {
                task();
            } else {
                Instance::log_w(
                    "(WindowManager) Task for window ({:#x}) failed: window has destroyed",
                    (std::size_t)handle
                );
            }
        }

        // general task request
        for (auto&& task : std::exchange(m_task_queue, {})) {
            task();
        }
    }

    bool WindowManager::send_intercept_event(Window& window, Event& event) noexcept
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
}
