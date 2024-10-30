#include "glfw_cpp/window.hpp"
#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/window_manager.hpp"
#include "glfw_cpp/event.hpp"

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
        windowCallbackHelper(
            window,
            Event::WindowMoved{
                .m_xPos = x,
                .m_yPos = y,
            }
        );
    }
    void Window::window_size_callback(GLFWwindow* window, int width, int height)
    {
        windowCallbackHelper(
            window,
            Event::WindowResized{
                .m_width  = width,
                .m_height = height,
            }
        );
    }

    void Window::window_close_callback(GLFWwindow* window)
    {
        windowCallbackHelper(window, Event::WindowClosed{});
    }

    void Window::window_refresh_callback(GLFWwindow* window)
    {
        windowCallbackHelper(window, Event::WindowRefreshed{});
    }

    void Window::window_focus_callback(GLFWwindow* window, int focused)
    {
        windowCallbackHelper(
            window,
            Event::WindowFocused{
                .m_focused = focused == GLFW_TRUE,
            }
        );
    }

    void Window::window_iconify_callback(GLFWwindow* window, int iconified)
    {
        windowCallbackHelper(
            window,
            Event::WindowIconified{
                .m_iconified = iconified == GLFW_TRUE,
            }
        );
    }

    void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        windowCallbackHelper(
            window,
            Event::FramebufferResized{
                .m_width  = width,
                .m_height = height,
            }
        );
    }

    void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        windowCallbackHelper(
            window,
            Event::ButtonPressed{
                .m_button = static_cast<MouseButton>(button),
                .m_state  = static_cast<MouseButtonState>(action),
                .m_mods   = static_cast<ModifierKey::Base>(mods),
            }
        );
    }

    void Window::cursor_pos_callback(GLFWwindow* window, double x, double y)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        // kinda useless using the helper here, but eh, I like consistency
        windowCallbackHelper(
            window,
            Event::CursorMoved{
                .m_xPos   = x,
                .m_yPos   = y,
                .m_xDelta = x - w->properties().m_cursor.m_x,
                .m_yDelta = y - w->properties().m_cursor.m_y,
            }
        );
    }

    void Window::cursor_enter_callback(GLFWwindow* window, int entered)
    {
        windowCallbackHelper(
            window,
            Event::CursorEntered{
                .m_entered = entered == GLFW_TRUE,
            }
        );
    }

    void Window::scroll_callback(GLFWwindow* window, double x, double y)
    {
        windowCallbackHelper(
            window,
            Event::Scrolled{
                .m_xOffset = x,
                .m_yOffset = y,
            }
        );
    }

    void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        windowCallbackHelper(
            window,
            Event::KeyPressed{
                .m_key      = static_cast<KeyCode>(key),
                .m_scancode = scancode,
                .m_state    = static_cast<KeyState>(action),
                .m_mods     = static_cast<ModifierKey::Base>(mods),
            }
        );
    }

    void Window::char_callback(GLFWwindow* window, unsigned int codepoint)
    {
        windowCallbackHelper(
            window,
            Event::CharInput{
                .m_codepoint = codepoint,
            }
        );
    }

    void Window::file_drop_callback(GLFWwindow* window, int count, const char** paths)
    {
        std::vector<std::filesystem::path> pathsVec(static_cast<std::size_t>(count));
        for (std::size_t i = 0; i < pathsVec.size(); ++i) {
            pathsVec[i] = std::filesystem::path{ paths[i] };
        }

        windowCallbackHelper(
            window,
            Event::FileDropped{
                .m_files = std::move(pathsVec),
            }
        );
    }

    void Window::window_maximize_callback(GLFWwindow* window, int maximized)
    {
        windowCallbackHelper(
            window,
            Event::WindowMaximized{
                .m_maximized = maximized == GLFW_TRUE,
            }
        );
    }

    void Window::window_content_scale_callback(GLFWwindow* window, float xscale, float yscale)
    {
        windowCallbackHelper(
            window,
            Event::WindowScaleChanged{
                .m_xScale = xscale,
                .m_yScale = yscale,
            }
        );
    }

    // this constructor must be called only from main thread (WindowManager run in main thread)
    Window::Window(
        WindowManager::Shared manager,
        Handle                handle,
        Properties&&          properties,
        bool                  bindImmediately
    ) noexcept
        : m_manager{ std::move(manager) }
        , m_handle{ handle }
        , m_attachedThreadId{}
        , m_properties{ std::move(properties) }
    {
        bind();

        if (auto* api = std::get_if<Api::OpenGL>(&Instance::get().m_api)) {
            api->m_loader(handle, glfwGetProcAddress);
        } else if (auto* api = std::get_if<Api::OpenGLES>(&Instance::get().m_api)) {
            api->m_loader(handle, glfwGetProcAddress);
        } else {
            // don't need to do anything for NoApi
        }

        setVsync(m_vsync);
        glfwSetWindowUserPointer(m_handle, this);

        if (!bindImmediately) {
            unbind();
        }
    }

    // clang-format off
    Window::Window(Window&& other) noexcept
        : m_manager          { std::exchange(other.m_manager, nullptr) }
        , m_handle           { std::exchange(other.m_handle, nullptr) }
        , m_attachedThreadId { std::exchange(other.m_attachedThreadId, {}) }
        , m_properties       { std::move(other.m_properties) }
        , m_lastFrameTime    { other.m_lastFrameTime }
        , m_deltaTime        { other.m_deltaTime }
        , m_vsync            { other.m_vsync }
        , m_captureMouse     { other.m_captureMouse }
        , m_eventQueueFront  { std::move(other.m_eventQueueFront) }
        , m_eventQueueBack   { std::move(other.m_eventQueueBack) }
        , m_taskQueue        { std::move(other.m_taskQueue) }
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
            m_manager->requestDeleteWindow(m_handle);
        }

        m_manager          = std::exchange(other.m_manager, nullptr);
        m_handle           = std::exchange(other.m_handle, nullptr);
        m_attachedThreadId = other.m_attachedThreadId;
        m_properties       = std::move(other.m_properties);
        m_lastFrameTime    = other.m_lastFrameTime;
        m_deltaTime        = other.m_deltaTime;
        m_vsync            = other.m_vsync;
        m_captureMouse     = other.m_captureMouse;
        m_eventQueueFront  = std::move(other.m_eventQueueFront);
        m_eventQueueBack   = std::move(other.m_eventQueueBack);
        m_taskQueue        = std::move(other.m_taskQueue);

        if (m_handle != nullptr) {
            glfwSetWindowUserPointer(m_handle, this);
        }

        return *this;
    }

    Window::~Window()
    {
        if (m_handle != nullptr) {
            unbind();
            glfwSetWindowUserPointer(m_handle, nullptr);    // remove user pointer
            m_manager->requestDeleteWindow(m_handle);
        } else {
            // window is in invalid state (probably moved)
        }
    }

    void Window::bind()
    {
        if (m_attachedThreadId == std::thread::id{}) {
            // no thread attached, attach to this thread

            m_attachedThreadId = std::this_thread::get_id();
            Instance::logD("(Window) Context ({:#x}) attached (+)", (std::size_t)m_handle);

            if (!std::holds_alternative<Api::NoApi>(Instance::get().m_api)) {
                glfwMakeContextCurrent(m_handle);
            }

        } else if (m_attachedThreadId == std::this_thread::get_id()) {

            // same thread, do nothing

        } else {
            // different thread, cannot attach

            Instance::logC(
                "(Window) Context ({:#x}) already attached to another thread [{:#x}], cannot attach to this "
                "thread [{:#x}].",
                (std::size_t)m_handle,
                util::getThreadNum(m_attachedThreadId),
                util::getThreadNum(std::this_thread::get_id())
            );

            // should I throw instead?
            assert(false && "Context already attached to another thread");
        }
    }

    void Window::unbind()
    {
        if (!std::holds_alternative<Api::NoApi>(Instance::get().m_api)) {
            glfwMakeContextCurrent(nullptr);
        }

        if (m_attachedThreadId != std::thread::id{}) {
            Instance::logD("(Window) Context ({:#x}) detached (-)", (std::size_t)m_handle);
            m_attachedThreadId = std::thread::id{};
        }
    }

    void Window::destroy()
    {
        *this = {};
    }

    void Window::iconify()
    {
        m_manager->enqueueWindowTask(m_handle, [this] { glfwIconifyWindow(m_handle); });
    }

    void Window::restore()
    {
        m_manager->enqueueWindowTask(m_handle, [this] { glfwRestoreWindow(m_handle); });
    }

    void Window::maximize()
    {
        m_manager->enqueueWindowTask(m_handle, [this] { glfwMaximizeWindow(m_handle); });
    }

    void Window::show()
    {
        m_manager->enqueueWindowTask(m_handle, [this] { glfwShowWindow(m_handle); });
    }

    void Window::hide()
    {
        m_manager->enqueueWindowTask(m_handle, [this] { glfwHideWindow(m_handle); });
    }

    void Window::focus()
    {
        m_manager->enqueueWindowTask(m_handle, [this] { glfwFocusWindow(m_handle); });
    }

    void Window::setVsync(bool value)
    {
        m_vsync = value;

        if (!std::holds_alternative<Api::NoApi>(Instance::get().m_api)) {
            // 0 = immediate updates, 1 = update synchronized with vertical retrace
            glfwSwapInterval(static_cast<int>(value));
        }
    }

    void Window::setWindowSize(int width, int height)
    {
        m_properties.m_dimension = {
            .m_width  = width,
            .m_height = height,
        };

        m_manager->enqueueWindowTask(m_handle, [this, width, height] {
            glfwSetWindowSize(m_handle, width, height);
        });
    }

    void Window::setWindowPos(int x, int y)
    {
        m_properties.m_pos = {
            .m_x = x,
            .m_y = y,
        };

        m_manager->enqueueWindowTask(m_handle, [this, x, y] { glfwSetWindowPos(m_handle, x, y); });
    }

    void Window::lockAspectRatio(float ratio)
    {
        if (ratio <= 0.0f) {
            Instance::logW("(Window) Invalid aspect ratio: {}", ratio);
            return;
        }

        m_manager->enqueueWindowTask(m_handle, [=, this] {
            auto width  = m_properties.m_dimension.m_width;
            auto height = int((float)width / ratio);
            glfwSetWindowAspectRatio(m_handle, width, height);
        });
    }

    void Window::lockCurrentAspectRatio()
    {
        m_manager->enqueueWindowTask(m_handle, [this] {
            auto [width, height] = m_properties.m_dimension;
            glfwSetWindowAspectRatio(m_handle, width, height);
        });
    }

    void Window::unlockAspectRatio()
    {
        m_manager->enqueueWindowTask(m_handle, [this] {
            glfwSetWindowAspectRatio(m_handle, GLFW_DONT_CARE, GLFW_DONT_CARE);
        });
    }

    void Window::updateTitle(const std::string& title)
    {
        m_properties.m_title = title;
        m_manager->enqueueWindowTask(m_handle, [this] {
            glfwSetWindowTitle(m_handle, m_properties.m_title.c_str());
        });
    }

    bool Window::shouldClose() const
    {
        return glfwWindowShouldClose(m_handle) == GLFW_TRUE;
    }

    // TODO: confirm if there is not data race on m_eventQueueFront since it returned freely here despite the
    // m_eventQueueBack locked on pushEvent() function
    const EventQueue& Window::poll()
    {
        processQueuedTasks();
        std::scoped_lock lock{ m_queueMutex };
        m_eventQueueFront.swap(m_eventQueueBack);
        m_eventQueueBack.reset();
        return m_eventQueueFront;
    }

    double Window::display()
    {
        if (!std::holds_alternative<Api::NoApi>(Instance::get().m_api)) {
            glfwSwapBuffers(m_handle);
        }
        updateDeltaTime();
        return m_deltaTime;
    }

    void Window::enqueueTask(Fun<void()>&& func)
    {
        std::scoped_lock lock{ m_queueMutex };
        m_taskQueue.push_back(std::move(func));
    }

    void Window::requestClose()
    {
        glfwSetWindowShouldClose(m_handle, 1);
        Instance::logI("(Window) Window ({:#x}) requested to close", (std::size_t)m_handle);
    }

    double Window::deltaTime() const
    {
        return m_deltaTime;
    }

    void Window::setCaptureMouse(bool value)
    {
        m_captureMouse = value;
        m_manager->enqueueTask([this] {
            if (m_captureMouse) {
                auto [x, y] = m_properties.m_cursor;
                glfwGetCursorPos(m_handle, &x, &y);    // prevent sudden jump on first capture
                glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        });
    }

    void Window::resizeEventQueue(std::size_t newSize)
    {
        std::scoped_lock lock{ m_queueMutex };
        m_eventQueueFront.resize(newSize, EventQueue::ResizePolicy::DISCARD_OLD);
        m_eventQueueBack.resize(newSize, EventQueue::ResizePolicy::DISCARD_OLD);
    }

    void Window::pushEvent(Event&& event)
    {
        std::scoped_lock lock{ m_queueMutex };

        // intercept some events to update properties before pushing them to the queue
        using KS = KeyState;
        using MS = MouseButtonState;
        using EV = Event;

        auto& [_, pos, dim, frame, cursor, attr, btns, keys, __] = m_properties;
        event.visit(util::VisitOverloaded{
            // clang-format off
            [&](EV::WindowMoved&        e) { pos    = { .m_x     = e.m_xPos,  .m_y      = e.m_yPos   }; },
            [&](EV::WindowResized&      e) { dim    = { .m_width = e.m_width, .m_height = e.m_height }; },
            [&](EV::FramebufferResized& e) { frame  = { .m_width = e.m_width, .m_height = e.m_height }; },
            [&](EV::CursorMoved&        e) { cursor = { .m_x     = e.m_xPos,  .m_y      = e.m_yPos   }; },
            [&](EV::CursorEntered&      e) { attr.m_hovered   = e.m_entered;   },
            [&](EV::WindowFocused&      e) { attr.m_focused   = e.m_focused;   },
            [&](EV::WindowIconified&    e) { attr.m_iconified = e.m_iconified; },
            [&](EV::WindowMaximized&    e) { attr.m_maximized = e.m_maximized; },
            [&](EV::KeyPressed&         e) { keys.setValue(e.m_key,    e.m_state != KS::RELEASE); },
            [&](EV::ButtonPressed&      e) { btns.setValue(e.m_button, e.m_state != MS::RELEASE); },
            [&] /* else */ (auto&)         { /* do nothing */ }
            // clang-format on
        });

        m_eventQueueBack.push(std::move(event));
    }

    void Window::processQueuedTasks()
    {
        auto queue = [&] {
            std::scoped_lock lock{ m_queueMutex };
            return std::exchange(m_taskQueue, {});
        }();

        for (auto& func : queue) {
            func();
        }
    }

    void Window::updateDeltaTime()
    {
        double currentTime{ glfwGetTime() };
        m_deltaTime     = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;
    }

    void Window::windowCallbackHelper(GLFWwindow* window, Event&& event) noexcept
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        auto forward = w->m_manager->sendInterceptEvent(*w, event);
        if (forward) {
            w->pushEvent(std::move(event));
        }
    }
}
