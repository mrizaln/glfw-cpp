#include "glfw_cpp/window.hpp"
#include "glfw_cpp/error.hpp"
#include "glfw_cpp/event.hpp"
#include "glfw_cpp/instance.hpp"

#include "util.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <functional>
#include <mutex>
#include <utility>

namespace glfw_cpp
{
    Window::Window(Handle handle, Properties&& properties, Attributes&& attributes)
        : m_handle{ handle }
        , m_properties{ std::move(properties) }
        , m_attributes{ std::move(attributes) }
        , m_has_context{ glfwGetWindowAttrib(handle, GLFW_CLIENT_API) != GLFW_NO_API }
    {
        glfwSetWindowUserPointer(m_handle, this);
    }

    // clang-format off
    Window::Window(Window&& other) noexcept
        : m_handle            { std::exchange(other.m_handle, nullptr) }
        , m_properties        { std::move(other.m_properties) }
        , m_attributes        { std::move(other.m_attributes) }
        , m_last_frame_time   { other.m_last_frame_time }
        , m_delta_time        { other.m_delta_time }
        , m_vsync             { other.m_vsync }
        , m_capture_mouse     { other.m_capture_mouse }
        , m_has_context       { other.m_has_context }
        , m_event_queue_front { std::move(other.m_event_queue_front) }
        , m_event_queue_back  { std::move(other.m_event_queue_back) }
    // clang-format on
    {
        glfwSetWindowUserPointer(m_handle, this);
    }

    Window& Window::operator=(Window&& other) noexcept
    {
        if (this == &other) {
            return *this;
        }

        if (m_handle != nullptr) {
            if (get_current() == m_handle) {
                make_current(nullptr);
            }

            glfwSetWindowUserPointer(m_handle, nullptr);    // remove user pointer
            Instance::get().request_delete_window(m_handle);
        }

        m_handle            = std::exchange(other.m_handle, nullptr);
        m_properties        = std::move(other.m_properties);
        m_attributes        = std::move(other.m_attributes);
        m_last_frame_time   = other.m_last_frame_time;
        m_delta_time        = other.m_delta_time;
        m_vsync             = other.m_vsync;
        m_capture_mouse     = other.m_capture_mouse;
        m_has_context       = other.m_has_context;
        m_event_queue_front = std::move(other.m_event_queue_front);
        m_event_queue_back  = std::move(other.m_event_queue_back);

        if (m_handle != nullptr) {
            glfwSetWindowUserPointer(m_handle, this);
        }

        return *this;
    }

    Window::~Window()
    {
        if (m_handle != nullptr) {
            if (get_current() == m_handle) {
                make_current(nullptr);
            }

            glfwSetWindowUserPointer(m_handle, nullptr);    // remove user pointer
            Instance::get().request_delete_window(m_handle);
        } else {
            // window is in invalid state (probably moved)
        }
    }

    void Window::destroy() noexcept
    {
        *this = {};
    }

    void Window::iconify() noexcept
    {
        m_attributes.iconified = true;
        Instance::get().enqueue_task([this] {
            glfwIconifyWindow(m_handle);
            util::check_glfw_error();
        });
    }

    void Window::restore() noexcept
    {
        m_attributes.iconified = false;
        m_attributes.maximized = false;
        Instance::get().enqueue_task([this] {
            glfwRestoreWindow(m_handle);
            util::check_glfw_error();
        });
    }

    void Window::maximize() noexcept
    {
        m_attributes.maximized = true;
        Instance::get().enqueue_task([this] {
            glfwMaximizeWindow(m_handle);
            util::check_glfw_error();
        });
    }

    void Window::show() noexcept
    {
        m_attributes.visible = true;
        Instance::get().enqueue_task([this] {
            glfwShowWindow(m_handle);
            util::check_glfw_error();
        });
    }

    void Window::hide() noexcept
    {
        m_attributes.visible = false;
        Instance::get().enqueue_task([this] {
            glfwHideWindow(m_handle);
            util::check_glfw_error();
        });
    }

    void Window::focus() noexcept
    {
        m_attributes.focused = true;
        Instance::get().enqueue_task([this] {
            glfwFocusWindow(m_handle);
            util::check_glfw_error();
        });
    }

    void Window::set_vsync(bool value)
    {
        m_vsync = value;

        if (not m_has_context) {
            throw error::NoWindowContext{ "Window has no associated context" };
        }

        // 0 = immediate update, 1 = update synchronized with vertical retrace
        auto vsync = value ? 1 : 0;

        if (auto current = get_current(); current == m_handle) {
            glfwSwapInterval(vsync);
        } else {
            make_current(m_handle);
            glfwSwapInterval(vsync);
            make_current(current);
        }

        util::check_glfw_error();
    }

    void Window::set_resizable(bool value)
    {
        m_attributes.resizable = value;
        Instance::get().enqueue_task([this, value] {
            glfwSetWindowAttrib(m_handle, GLFW_RESIZABLE, value ? GLFW_TRUE : GLFW_FALSE);
            util::check_glfw_error();
        });
    }

    void Window::set_decorated(bool value)
    {
        m_attributes.decorated = value;
        Instance::get().enqueue_task([this, value] {
            glfwSetWindowAttrib(m_handle, GLFW_RESIZABLE, value ? GLFW_TRUE : GLFW_FALSE);
            util::check_glfw_error();
        });
    }

    void Window::set_auto_iconify(bool value)
    {
        m_attributes.auto_iconify = value;
        Instance::get().enqueue_task([this, value] {
            glfwSetWindowAttrib(m_handle, GLFW_AUTO_ICONIFY, value ? GLFW_TRUE : GLFW_FALSE);
            util::check_glfw_error();
        });
    }

    void Window::set_floating(bool value)
    {
        m_attributes.floating = value;
        Instance::get().enqueue_task([this, value] {
            glfwSetWindowAttrib(m_handle, GLFW_FLOATING, value ? GLFW_TRUE : GLFW_FALSE);
            util::check_glfw_error();
        });
    }

    void Window::set_focus_on_show(bool value)
    {
        m_attributes.focus_on_show = value;
        Instance::get().enqueue_task([this, value] {
            glfwSetWindowAttrib(m_handle, GLFW_FOCUS_ON_SHOW, value ? GLFW_TRUE : GLFW_FALSE);
            util::check_glfw_error();
        });
    }

    void Window::set_mouse_passthrough(bool value)
    {
        m_attributes.mouse_passthrough = value;
        Instance::get().enqueue_task([this, value] {
            glfwSetWindowAttrib(m_handle, GLFW_MOUSE_PASSTHROUGH, value ? GLFW_TRUE : GLFW_FALSE);
            util::check_glfw_error();
        });
    }

    void Window::set_window_size(int width, int height) noexcept
    {
        m_properties.dimensions = { .width = width, .height = height };
        Instance::get().enqueue_task([this, width, height] {
            glfwSetWindowSize(m_handle, width, height);
            util::check_glfw_error();
        });
    }

    void Window::set_window_pos(int x, int y) noexcept
    {
        m_properties.position = { .x = x, .y = y };
        Instance::get().enqueue_task([this, x, y] {
            glfwSetWindowPos(m_handle, x, y);
            util::check_glfw_error();
        });
    }

    float Window::aspect_ratio() const noexcept
    {
        auto [width, height] = m_properties.dimensions;
        return (float)width / (float)height;
    }

    void Window::lock_aspect_ratio(float ratio) noexcept
    {
        assert(ratio > 0.0f);
        Instance::get().enqueue_task([this, ratio] {
            auto width  = m_properties.dimensions.width;
            auto height = int((float)width / ratio);
            glfwSetWindowAspectRatio(m_handle, width, height);
            util::check_glfw_error();
        });
    }

    void Window::lock_current_aspect_ratio() noexcept
    {
        Instance::get().enqueue_task([this] {
            auto [width, height] = m_properties.dimensions;
            glfwSetWindowAspectRatio(m_handle, width, height);
            util::check_glfw_error();
        });
    }

    void Window::unlock_aspect_ratio() noexcept
    {
        Instance::get().enqueue_task([this] {
            glfwSetWindowAspectRatio(m_handle, GLFW_DONT_CARE, GLFW_DONT_CARE);
            util::check_glfw_error();
        });
    }

    void Window::update_title(std::string_view title) noexcept
    {
        m_properties.title = title;
        Instance::get().enqueue_task([this, &title = m_properties.title] {
            glfwSetWindowTitle(m_handle, title.c_str());
            util::check_glfw_error();
        });
    }

    bool Window::should_close() const noexcept
    {
        return glfwWindowShouldClose(m_handle) == GLFW_TRUE;
    }

    const EventQueue& Window::swap_events() noexcept
    {
        std::scoped_lock lock{ m_queue_mutex };
        m_event_queue_front.swap(m_event_queue_back);
        m_event_queue_back.reset();
        return m_event_queue_front;
    }

    double Window::swap_buffers()
    {
        if (m_has_context) {
            glfwSwapBuffers(m_handle);
            util::check_glfw_error();
        }
        update_delta_time();
        return m_delta_time;
    }

    void Window::request_close() noexcept
    {
        glfwSetWindowShouldClose(m_handle, 1);
    }

    void Window::set_capture_mouse(bool value) noexcept
    {
        m_capture_mouse = value;
        Instance::get().enqueue_task([this] {
            if (m_capture_mouse) {
                auto& [x, y] = m_properties.cursor_position;
                glfwGetCursorPos(m_handle, &x, &y);    // prevent sudden jump on first capture
                glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            util::check_glfw_error();
        });
    }

    void Window::resize_event_queue(std::size_t new_size) noexcept
    {
        std::scoped_lock lock{ m_queue_mutex };
        m_event_queue_front.resize(new_size, EventQueue::ResizePolicy::DiscardOld);
        m_event_queue_back.resize(new_size, EventQueue::ResizePolicy::DiscardOld);
    }

    void Window::push_event(Event&& event) noexcept
    {
        std::scoped_lock lock{ m_queue_mutex };

        // intercept some events to update properties before pushing them to the queue
        using KS = KeyState;
        using MS = MouseButtonState;

        auto& [title, pos, dim, frame, cursor, btns, keys, mon] = m_properties;
        event.visit(util::VisitOverloaded{
            // clang-format off
            [&](event::WindowMoved&        e) { pos    = { .x     = e.x,     .y      = e.y   }; },
            [&](event::WindowResized&      e) { dim    = { .width = e.width, .height = e.height }; },
            [&](event::FramebufferResized& e) { frame  = { .width = e.width, .height = e.height }; },
            [&](event::CursorMoved&        e) { cursor = { .x     = e.x,     .y      = e.y      }; },
            [&](event::CursorEntered&      e) { m_attributes.hovered   = e.entered;   },
            [&](event::WindowFocused&      e) { m_attributes.focused   = e.focused;   },
            [&](event::WindowIconified&    e) { m_attributes.iconified = e.iconified; },
            [&](event::WindowMaximized&    e) { m_attributes.maximized = e.maximized; },
            [&](event::KeyPressed&         e) { keys.set_value(e.key,    e.state != KS::Release); },
            [&](event::ButtonPressed&      e) { btns.set_value(e.button, e.state != MS::Release); },
            [&] /* else */ (auto&)         { /* do nothing */ }
            // clang-format on
        });

        m_event_queue_back.push(std::move(event));
    }

    void Window::update_delta_time() noexcept
    {
        auto current_time = glfwGetTime();
        m_delta_time      = current_time - m_last_frame_time;
        m_last_frame_time = current_time;
    }
}
