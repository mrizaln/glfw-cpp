#include "glfw_cpp/error.hpp"
#include "glfw_cpp/event.hpp"
#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/window.hpp"
#include "glfw_cpp/window_manager.hpp"

#include "util.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <filesystem>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace glfw_cpp
{
    void Window::window_pos_callback(GLFWwindow* window, int x, int y)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        window_callback_helper(
            window,
            Event::WindowMoved{
                .m_x  = x,
                .m_y  = y,
                .m_dx = x - w->properties().m_pos.m_x,
                .m_dy = y - w->properties().m_pos.m_y,
            }
        );
    }
    void Window::window_size_callback(GLFWwindow* window, int width, int height)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        window_callback_helper(
            window,
            Event::WindowResized{
                .m_width         = width,
                .m_height        = height,
                .m_width_change  = width - w->properties().m_dimension.m_width,
                .m_height_change = height - w->properties().m_dimension.m_height,
            }
        );
    }

    void Window::window_close_callback(GLFWwindow* window)
    {
        window_callback_helper(window, Event::WindowClosed{});
    }

    void Window::window_refresh_callback(GLFWwindow* window)
    {
        window_callback_helper(window, Event::WindowRefreshed{});
    }

    void Window::window_focus_callback(GLFWwindow* window, int focused)
    {
        window_callback_helper(
            window,
            Event::WindowFocused{
                .m_focused = focused == GLFW_TRUE,
            }
        );
    }

    void Window::window_iconify_callback(GLFWwindow* window, int iconified)
    {
        window_callback_helper(
            window,
            Event::WindowIconified{
                .m_iconified = iconified == GLFW_TRUE,
            }
        );
    }

    void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        window_callback_helper(
            window,
            Event::FramebufferResized{
                .m_width         = width,
                .m_height        = height,
                .m_width_change  = width - w->properties().m_framebuffer_size.m_width,
                .m_height_change = height - w->properties().m_framebuffer_size.m_height,
            }
        );
    }

    void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        window_callback_helper(
            window,
            Event::ButtonPressed{
                .m_button = static_cast<MouseButton>(button),
                .m_state  = static_cast<MouseButtonState>(action),
                .m_mods   = ModifierKey{ mods },
            }
        );
    }

    void Window::cursor_pos_callback(GLFWwindow* window, double x, double y)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        window_callback_helper(
            window,
            Event::CursorMoved{
                .m_x  = x,
                .m_y  = y,
                .m_dx = x - w->properties().m_cursor.m_x,
                .m_dy = y - w->properties().m_cursor.m_y,
            }
        );
    }

    void Window::cursor_enter_callback(GLFWwindow* window, int entered)
    {
        window_callback_helper(
            window,
            Event::CursorEntered{
                .m_entered = entered == GLFW_TRUE,
            }
        );
    }

    void Window::scroll_callback(GLFWwindow* window, double x, double y)
    {
        window_callback_helper(
            window,
            Event::Scrolled{
                .m_dx = x,
                .m_dy = y,
            }
        );
    }

    void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        window_callback_helper(
            window,
            Event::KeyPressed{
                .m_key      = static_cast<KeyCode>(key),
                .m_scancode = scancode,
                .m_state    = static_cast<KeyState>(action),
                .m_mods     = ModifierKey{ mods },
            }
        );
    }

    void Window::char_callback(GLFWwindow* window, unsigned int codepoint)
    {
        window_callback_helper(
            window,
            Event::CharInput{
                .m_codepoint = codepoint,
            }
        );
    }

    void Window::file_drop_callback(GLFWwindow* window, int count, const char** paths)
    {
        auto paths_vec = std::vector<std::filesystem::path>(static_cast<std::size_t>(count));
        for (std::size_t i = 0; i < paths_vec.size(); ++i) {
            paths_vec[i] = std::filesystem::path{ paths[i] };
        }

        window_callback_helper(
            window,
            Event::FileDropped{
                .m_files = std::move(paths_vec),
            }
        );
    }

    void Window::window_maximize_callback(GLFWwindow* window, int maximized)
    {
        window_callback_helper(
            window,
            Event::WindowMaximized{
                .m_maximized = maximized == GLFW_TRUE,
            }
        );
    }

    void Window::window_content_scale_callback(GLFWwindow* window, float xscale, float yscale)
    {
        window_callback_helper(
            window,
            Event::WindowScaleChanged{
                .m_x = xscale,
                .m_y = yscale,
            }
        );
    }

    // this constructor must be called only from main thread (WindowManager run in main thread)
    Window::Window(
        WindowManager::Shared manager,
        Handle                handle,
        Properties&&          properties,
        bool                  bind_immediately
    )
        : m_manager{ std::move(manager) }
        , m_handle{ handle }
        , m_attached_thread_id{}
        , m_properties{ std::move(properties) }
    {
        auto init = [&](auto* api) {
            bind();
            set_vsync(m_vsync);
            glfwSetWindowUserPointer(m_handle, this);
            api->m_loader(handle, glfwGetProcAddress);
            if (!bind_immediately) {
                unbind();
            }
        };

        if (auto* api = std::get_if<Api::OpenGL>(&Instance::get().m_api)) {
            init(api);
        } else if (auto* api = std::get_if<Api::OpenGLES>(&Instance::get().m_api)) {
            init(api);
        } else {
            glfwSetWindowUserPointer(m_handle, this);
            // don't need to do anything for NoApi
        }
    }

    // clang-format off
    Window::Window(Window&& other) noexcept
        : m_manager           { std::exchange(other.m_manager, nullptr) }
        , m_handle            { std::exchange(other.m_handle, nullptr) }
        , m_attached_thread_id{ std::exchange(other.m_attached_thread_id, {}) }
        , m_properties        { std::move(other.m_properties) }
        , m_last_frame_time   { other.m_last_frame_time }
        , m_delta_time        { other.m_delta_time }
        , m_vsync             { other.m_vsync }
        , m_capture_mouse     { other.m_capture_mouse }
        , m_event_queue_front { std::move(other.m_event_queue_front) }
        , m_event_queue_back  { std::move(other.m_event_queue_back) }
        , m_task_queue        { std::move(other.m_task_queue) }
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
            unbind();
            glfwSetWindowUserPointer(m_handle, nullptr);    // remove user pointer
            m_manager->request_delete_window(m_handle);
        }

        m_manager            = std::exchange(other.m_manager, nullptr);
        m_handle             = std::exchange(other.m_handle, nullptr);
        m_attached_thread_id = other.m_attached_thread_id;
        m_properties         = std::move(other.m_properties);
        m_last_frame_time    = other.m_last_frame_time;
        m_delta_time         = other.m_delta_time;
        m_vsync              = other.m_vsync;
        m_capture_mouse      = other.m_capture_mouse;
        m_event_queue_front  = std::move(other.m_event_queue_front);
        m_event_queue_back   = std::move(other.m_event_queue_back);
        m_task_queue         = std::move(other.m_task_queue);

        if (m_handle != nullptr) {
            glfwSetWindowUserPointer(m_handle, this);
        }

        return *this;
    }

    Window::~Window()
    {
        if (m_handle != nullptr) {
            if (!std::holds_alternative<Api::NoApi>(Instance::get().m_api)) {
                unbind();
            }
            glfwSetWindowUserPointer(m_handle, nullptr);    // remove user pointer
            m_manager->request_delete_window(m_handle);
        } else {
            // window is in invalid state (probably moved)
        }
    }

    void Window::bind()
    {
        if (m_attached_thread_id == std::thread::id{}) {
            // no thread attached, attach to this thread

            m_attached_thread_id = std::this_thread::get_id();
            Instance::log_d("(Window) Context ({:#x}) attached (+)", (std::size_t)m_handle);

            if (!std::holds_alternative<Api::NoApi>(Instance::get().m_api)) {
                glfwMakeContextCurrent(m_handle);
            } else {
                throw NoWindowContext{ "glfw-cpp is initialized to be NoApi" };
            }

        } else if (m_attached_thread_id == std::this_thread::get_id()) {

            // same thread, do nothing

        } else {
            // different thread, cannot attach

            Instance::log_c(
                "(Window) Context ({:#x}) already attached to another thread [{:#x}], cannot attach to this "
                "thread [{:#x}].",
                (std::size_t)m_handle,
                util::get_thread_num(m_attached_thread_id),
                util::get_thread_num(std::this_thread::get_id())
            );

            throw AlreadyBound{
                util::get_thread_num(std::this_thread::get_id()),
                util::get_thread_num(m_attached_thread_id),
            };
        }
    }

    void Window::unbind()
    {
        if (!std::holds_alternative<Api::NoApi>(Instance::get().m_api)) {
            glfwMakeContextCurrent(nullptr);
        } else {
            throw NoWindowContext{ "glfw-cpp is initialized to be NoApi" };
        }

        if (m_attached_thread_id != std::thread::id{}) {
            Instance::log_d("(Window) Context ({:#x}) detached (-)", (std::size_t)m_handle);
            m_attached_thread_id = std::thread::id{};
        }
    }

    void Window::destroy() noexcept
    {
        *this = {};
    }

    void Window::iconify() noexcept
    {
        m_manager->enqueue_window_task(m_handle, [this] { glfwIconifyWindow(m_handle); });
    }

    void Window::restore() noexcept
    {
        m_manager->enqueue_window_task(m_handle, [this] { glfwRestoreWindow(m_handle); });
    }

    void Window::maximize() noexcept
    {
        m_manager->enqueue_window_task(m_handle, [this] { glfwMaximizeWindow(m_handle); });
    }

    void Window::show() noexcept
    {
        m_manager->enqueue_window_task(m_handle, [this] { glfwShowWindow(m_handle); });
    }

    void Window::hide() noexcept
    {
        m_manager->enqueue_window_task(m_handle, [this] { glfwHideWindow(m_handle); });
    }

    void Window::focus() noexcept
    {
        m_manager->enqueue_window_task(m_handle, [this] { glfwFocusWindow(m_handle); });
    }

    void Window::set_vsync(bool value)
    {
        m_vsync = value;

        if (!std::holds_alternative<Api::NoApi>(Instance::get().m_api)) {
            // 0 = immediate updates, 1 = update synchronized with vertical retrace
            glfwSwapInterval(value ? 1 : 0);
        } else {
            throw NoWindowContext{ "glfw-cpp is initialized to be NoApi" };
        }
    }

    void Window::set_window_size(int width, int height) noexcept
    {
        m_properties.m_dimension = {
            .m_width  = width,
            .m_height = height,
        };

        m_manager->enqueue_window_task(m_handle, [this, width, height] {
            glfwSetWindowSize(m_handle, width, height);
        });
    }

    void Window::set_window_pos(int x, int y) noexcept
    {
        m_properties.m_pos = {
            .m_x = x,
            .m_y = y,
        };

        m_manager->enqueue_window_task(m_handle, [this, x, y] { glfwSetWindowPos(m_handle, x, y); });
    }

    float Window::aspect_ratio() const noexcept
    {
        auto [width, height] = m_properties.m_dimension;
        return (float)width / (float)height;
    }

    void Window::lock_aspect_ratio(float ratio) noexcept
    {
        if (ratio <= 0.0f) {
            Instance::log_w("(Window) Invalid aspect ratio: {}", ratio);
            return;
        }

        m_manager->enqueue_window_task(m_handle, [=, this] {
            auto width  = m_properties.m_dimension.m_width;
            auto height = int((float)width / ratio);
            glfwSetWindowAspectRatio(m_handle, width, height);
        });
    }

    void Window::lock_current_aspect_ratio() noexcept
    {
        m_manager->enqueue_window_task(m_handle, [this] {
            auto [width, height] = m_properties.m_dimension;
            glfwSetWindowAspectRatio(m_handle, width, height);
        });
    }

    void Window::unlock_aspect_ratio() noexcept
    {
        m_manager->enqueue_window_task(m_handle, [this] {
            glfwSetWindowAspectRatio(m_handle, GLFW_DONT_CARE, GLFW_DONT_CARE);
        });
    }

    void Window::update_title(std::string_view title) noexcept
    {
        m_properties.m_title = title;
        m_manager->enqueue_window_task(m_handle, [this] {
            glfwSetWindowTitle(m_handle, m_properties.m_title.c_str());
        });
    }

    bool Window::should_close() const noexcept
    {
        return glfwWindowShouldClose(m_handle) == GLFW_TRUE;
    }

    // TODO: confirm if there is not data race on m_event_queue_front since it returned freely here despite
    // the m_event_queue_back locked on push_event() function
    const EventQueue& Window::poll() noexcept
    {
        process_queued_tasks();
        std::scoped_lock lock{ m_queue_mutex };
        m_event_queue_front.swap(m_event_queue_back);
        m_event_queue_back.reset();
        return m_event_queue_front;
    }

    double Window::display()
    {
        if (!std::holds_alternative<Api::NoApi>(Instance::get().m_api)) {
            glfwSwapBuffers(m_handle);
        } else {
            util::check_glfw_error();
        }
        update_delta_time();
        return m_delta_time;
    }

    void Window::enqueue_task(Fun<void(Window&)>&& func) noexcept
    {
        std::scoped_lock lock{ m_queue_mutex };
        m_task_queue.push_back(std::move(func));
    }

    void Window::request_close() noexcept
    {
        glfwSetWindowShouldClose(m_handle, 1);
        Instance::log_i("(Window) Window ({:#x}) requested to close", (std::size_t)m_handle);
    }

    void Window::set_capture_mouse(bool value) noexcept
    {
        m_capture_mouse = value;
        m_manager->enqueue_task([this] {
            if (m_capture_mouse) {
                auto& [x, y] = m_properties.m_cursor;
                glfwGetCursorPos(m_handle, &x, &y);    // prevent sudden jump on first capture
                glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
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
        using EV = Event;

        auto& [_, pos, dim, frame, cursor, attr, btns, keys, __] = m_properties;
        event.visit(util::VisitOverloaded{
            // clang-format off
            [&](EV::WindowMoved&        e) { pos    = { .m_x     = e.m_x,     .m_y      = e.m_y   }; },
            [&](EV::WindowResized&      e) { dim    = { .m_width = e.m_width, .m_height = e.m_height }; },
            [&](EV::FramebufferResized& e) { frame  = { .m_width = e.m_width, .m_height = e.m_height }; },
            [&](EV::CursorMoved&        e) { cursor = { .m_x     = e.m_x,     .m_y      = e.m_y      }; },
            [&](EV::CursorEntered&      e) { attr.m_hovered   = e.m_entered;   },
            [&](EV::WindowFocused&      e) { attr.m_focused   = e.m_focused;   },
            [&](EV::WindowIconified&    e) { attr.m_iconified = e.m_iconified; },
            [&](EV::WindowMaximized&    e) { attr.m_maximized = e.m_maximized; },
            [&](EV::KeyPressed&         e) { keys.set_value(e.m_key,    e.m_state != KS::Release); },
            [&](EV::ButtonPressed&      e) { btns.set_value(e.m_button, e.m_state != MS::Release); },
            [&] /* else */ (auto&)         { /* do nothing */ }
            // clang-format on
        });

        m_event_queue_back.push(std::move(event));
    }

    void Window::process_queued_tasks() noexcept
    {
        auto queue = [&] {
            std::scoped_lock lock{ m_queue_mutex };
            return std::exchange(m_task_queue, {});
        }();

        for (auto& func : queue) {
            func(*this);
        }
    }

    void Window::update_delta_time() noexcept
    {
        auto current_time = glfwGetTime();
        m_delta_time      = current_time - m_last_frame_time;
        m_last_frame_time = current_time;
    }

    void Window::window_callback_helper(GLFWwindow* window, Event&& event) noexcept
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        auto forward = w->m_manager->send_intercept_event(*w, event);
        if (forward) {
            w->push_event(std::move(event));
        }
    }
}
