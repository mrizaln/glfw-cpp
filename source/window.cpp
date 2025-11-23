#include "glfw_cpp/window.hpp"
#include "glfw_cpp/error.hpp"
#include "glfw_cpp/event.hpp"
#include "glfw_cpp/instance.hpp"

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
            event::WindowMoved{
                .x  = x,
                .y  = y,
                .dx = x - w->properties().pos.x,
                .dy = y - w->properties().pos.y,
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
            event::WindowResized{
                .width         = width,
                .height        = height,
                .width_change  = width - w->properties().dimensions.width,
                .height_change = height - w->properties().dimensions.height,
            }
        );
    }

    void Window::window_close_callback(GLFWwindow* window)
    {
        window_callback_helper(window, event::WindowClosed{});
    }

    void Window::window_refresh_callback(GLFWwindow* window)
    {
        window_callback_helper(window, event::WindowRefreshed{});
    }

    void Window::window_focus_callback(GLFWwindow* window, int focused)
    {
        window_callback_helper(
            window,
            event::WindowFocused{
                .focused = focused == GLFW_TRUE,
            }
        );
    }

    void Window::window_iconify_callback(GLFWwindow* window, int iconified)
    {
        window_callback_helper(
            window,
            event::WindowIconified{
                .iconified = iconified == GLFW_TRUE,
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
            event::FramebufferResized{
                .width         = width,
                .height        = height,
                .width_change  = width - w->properties().framebuffer_size.width,
                .height_change = height - w->properties().framebuffer_size.height,
            }
        );
    }

    void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        window_callback_helper(
            window,
            event::ButtonPressed{
                .button = static_cast<MouseButton>(button),
                .state  = static_cast<MouseButtonState>(action),
                .mods   = ModifierKey{ mods },
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
            event::CursorMoved{
                .x  = x,
                .y  = y,
                .dx = x - w->properties().cursor.x,
                .dy = y - w->properties().cursor.y,
            }
        );
    }

    void Window::cursor_enter_callback(GLFWwindow* window, int entered)
    {
        window_callback_helper(
            window,
            event::CursorEntered{
                .entered = entered == GLFW_TRUE,
            }
        );
    }

    void Window::scroll_callback(GLFWwindow* window, double x, double y)
    {
        window_callback_helper(
            window,
            event::Scrolled{
                .dx = x,
                .dy = y,
            }
        );
    }

    void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        window_callback_helper(
            window,
            event::KeyPressed{
                .key      = static_cast<KeyCode>(key),
                .scancode = scancode,
                .state    = static_cast<KeyState>(action),
                .mods     = ModifierKey{ mods },
            }
        );
    }

    void Window::char_callback(GLFWwindow* window, unsigned int codepoint)
    {
        window_callback_helper(
            window,
            event::CharInput{
                .codepoint = codepoint,
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
            event::FileDropped{
                .files = std::move(paths_vec),
            }
        );
    }

    void Window::window_maximize_callback(GLFWwindow* window, int maximized)
    {
        window_callback_helper(
            window,
            event::WindowMaximized{
                .maximized = maximized == GLFW_TRUE,
            }
        );
    }

    void Window::window_content_scale_callback(GLFWwindow* window, float xscale, float yscale)
    {
        window_callback_helper(
            window,
            event::WindowScaleChanged{
                .x = xscale,
                .y = yscale,
            }
        );
    }

    // this constructor must be called only from main thread (WindowManager run in main thread)
    Window::Window(Handle handle, Properties&& properties, bool bind_immediately)
        : m_handle{ handle }
        , m_properties{ std::move(properties) }
    {
        auto init = [&](auto& api) {
            bind();
            set_vsync(m_vsync);
            glfwSetWindowUserPointer(m_handle, this);
#if not __EMSCRIPTEN__
            assert(api.loader != nullptr);
            api.loader(handle, glfwGetProcAddress);
#else
            if (api.loader) {
                api.loader(handle, glfwGetProcAddress);
            }
#endif
            if (not bind_immediately) {
                unbind();
            }
        };

        Instance::get().m_api.visit(util::VisitOverloaded{
            [&](api::OpenGL& api) { init(api); },
            [&](api::OpenGLES& api) { init(api); },
            [&](api::NoApi&) { glfwSetWindowUserPointer(m_handle, this); },
        });
    }

    // clang-format off
    Window::Window(Window&& other) noexcept
        : m_handle            { std::exchange(other.m_handle, nullptr) }
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
            Instance::get().request_delete_window(m_handle);
        }

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
            if (not Instance::get().m_api.is<api::NoApi>()) {
                unbind();
            }
            glfwSetWindowUserPointer(m_handle, nullptr);    // remove user pointer
            Instance::get().request_delete_window(m_handle);
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

            if (not Instance::get().m_api.is<api::NoApi>()) {
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
        if (not Instance::get().m_api.is<api::NoApi>()) {
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
        Instance::get().enqueue_task([this] { glfwIconifyWindow(m_handle); });
    }

    void Window::restore() noexcept
    {
        Instance::get().enqueue_task([this] { glfwRestoreWindow(m_handle); });
    }

    void Window::maximize() noexcept
    {
        Instance::get().enqueue_task([this] { glfwMaximizeWindow(m_handle); });
    }

    void Window::show() noexcept
    {
        Instance::get().enqueue_task([this] { glfwShowWindow(m_handle); });
    }

    void Window::hide() noexcept
    {
        Instance::get().enqueue_task([this] { glfwHideWindow(m_handle); });
    }

    void Window::focus() noexcept
    {
        Instance::get().enqueue_task([this] { glfwFocusWindow(m_handle); });
    }

    void Window::set_vsync(bool value)
    {
        m_vsync = value;

        if (Instance::get().m_api.is<api::NoApi>()) {
            throw NoWindowContext{ "glfw-cpp is initialized to be NoApi" };
        }

        // 0 = immediate update, 1 = update synchronized with vertical retrace
        auto vsync = value ? 1 : 0;

        if (auto current = glfwGetCurrentContext(); current == m_handle) {
            glfwSwapInterval(vsync);
        } else {
            glfwMakeContextCurrent(m_handle);
            glfwSwapInterval(vsync);
            glfwMakeContextCurrent(current);
        }
    }

    void Window::set_window_size(int width, int height) noexcept
    {
        m_properties.dimensions = {
            .width  = width,
            .height = height,
        };

        Instance::get().enqueue_task([this, width, height] { glfwSetWindowSize(m_handle, width, height); });
    }

    void Window::set_window_pos(int x, int y) noexcept
    {
        m_properties.pos = { .x = x, .y = y };
        Instance::get().enqueue_task([this, x, y] { glfwSetWindowPos(m_handle, x, y); });
    }

    float Window::aspect_ratio() const noexcept
    {
        auto [width, height] = m_properties.dimensions;
        return (float)width / (float)height;
    }

    void Window::lock_aspect_ratio(float ratio) noexcept
    {
        if (ratio <= 0.0f) {
            Instance::log_w("(Window) Invalid aspect ratio: {}", ratio);
            return;
        }

        Instance::get().enqueue_task([this, ratio] {
            auto width  = m_properties.dimensions.width;
            auto height = int((float)width / ratio);
            glfwSetWindowAspectRatio(m_handle, width, height);
        });
    }

    void Window::lock_current_aspect_ratio() noexcept
    {
        Instance::get().enqueue_task([this] {
            auto [width, height] = m_properties.dimensions;
            glfwSetWindowAspectRatio(m_handle, width, height);
        });
    }

    void Window::unlock_aspect_ratio() noexcept
    {
        Instance::get().enqueue_task([this] {
            glfwSetWindowAspectRatio(m_handle, GLFW_DONT_CARE, GLFW_DONT_CARE);
        });
    }

    void Window::update_title(std::string_view title) noexcept
    {
        m_properties.title = title;
        Instance::get().enqueue_task([this, &title = m_properties.title] {
            glfwSetWindowTitle(m_handle, title.c_str());
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
        if (not Instance::get().m_api.is<api::NoApi>()) {
            glfwSwapBuffers(m_handle);
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
        Instance::get().enqueue_task([this] {
            if (m_capture_mouse) {
                auto& [x, y] = m_properties.cursor;
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

        auto& [_, pos, dim, frame, cursor, attr, btns, keys, __] = m_properties;
        event.visit(util::VisitOverloaded{
            // clang-format off
            [&](event::WindowMoved&        e) { pos    = { .x     = e.x,     .y      = e.y   }; },
            [&](event::WindowResized&      e) { dim    = { .width = e.width, .height = e.height }; },
            [&](event::FramebufferResized& e) { frame  = { .width = e.width, .height = e.height }; },
            [&](event::CursorMoved&        e) { cursor = { .x     = e.x,     .y      = e.y      }; },
            [&](event::CursorEntered&      e) { attr.hovered   = e.entered;   },
            [&](event::WindowFocused&      e) { attr.focused   = e.focused;   },
            [&](event::WindowIconified&    e) { attr.iconified = e.iconified; },
            [&](event::WindowMaximized&    e) { attr.maximized = e.maximized; },
            [&](event::KeyPressed&         e) { keys.set_value(e.key,    e.state != KS::Release); },
            [&](event::ButtonPressed&      e) { btns.set_value(e.button, e.state != MS::Release); },
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

        auto forward = Instance::get().send_intercept_event(*w, event);
        if (forward) {
            w->push_event(std::move(event));
        }
    }
}
