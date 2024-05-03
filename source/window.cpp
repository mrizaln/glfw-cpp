#include "glfw_cpp/window.hpp"
#include "glfw_cpp/context.hpp"
#include "glfw_cpp/window_manager.hpp"
#include "glfw_cpp/event.hpp"

#include "util.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <format>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>

namespace glfw_cpp
{
    void Window::window_pos_callback(GLFWwindow* window, int x, int y)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_eventQueue.emplace_back(Event::WindowMoved{
            .m_xPos = x,
            .m_yPos = y,
        });
    }
    void Window::window_size_callback(GLFWwindow* window, int width, int height)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_properties.m_width  = width;
        w->m_properties.m_height = height;

        w->m_eventQueue.emplace_back(Event::WindowResized{
            .m_width  = width,
            .m_height = height,
        });
    }

    void Window::window_close_callback(GLFWwindow* window)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_eventQueue.emplace_back(Event::WindowClosed{});
    }

    void Window::window_refresh_callback(GLFWwindow* window)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_eventQueue.emplace_back(Event::WindowRefreshed{});
    }

    void Window::window_focus_callback(GLFWwindow* window, int focused)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_eventQueue.emplace_back(Event::WindowFocused{
            .m_focused = focused == GLFW_TRUE,
        });
    }

    void Window::window_iconify_callback(GLFWwindow* window, int iconified)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_eventQueue.emplace_back(Event::WindowIconified{
            .m_iconified = iconified == GLFW_TRUE,
        });
    }

    void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_eventQueue.emplace_back(Event::FramebufferResized{
            .m_width  = width,
            .m_height = height,
        });
    }

    void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        if (button > GLFW_MOUSE_BUTTON_LAST) {
            Context::logW(std::format("(Window) Invalid mouse button: {}", button));
            return;
        }

        w->m_eventQueue.emplace_back(Event::ButtonPressed{
            .m_button = static_cast<MouseButton>(button),
            .m_state  = static_cast<MouseButtonState>(action),
            .m_mods   = static_cast<ModifierKey::Base>(mods),
        });
    }

    void Window::cursor_pos_callback(GLFWwindow* window, double x, double y)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_properties.m_cursorPos = {
            .m_x = x,
            .m_y = y,
        };

        w->m_eventQueue.emplace_back(Event::CursorMoved{
            .m_xPos = x,
            .m_yPos = y,
        });
    }

    void Window::cursor_enter_callback(GLFWwindow* window, int entered)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_eventQueue.emplace_back(Event::CursorEntered{
            .m_entered = entered == GLFW_TRUE,
        });
    }

    void Window::scroll_callback(GLFWwindow* window, double x, double y)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_eventQueue.emplace_back(Event::Scrolled{
            .m_xOffset = x,
            .m_yOffset = y,
        });
    }

    void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_eventQueue.emplace_back(Event::KeyPressed{
            .m_key      = static_cast<KeyCode>(key),
            .m_scancode = scancode,
            .m_state    = static_cast<KeyState>(action),
            .m_mods     = static_cast<ModifierKey::Base>(mods),
        });
    }

    void Window::char_callback(GLFWwindow* window, unsigned int codepoint)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (w == nullptr) {
            return;
        }

        w->m_eventQueue.emplace_back(Event::CharInput{
            .m_codepoint = codepoint,
        });
    }

    // this constructor must be called only from main thread (WindowManager run in main thread)
    Window::Window(
        WindowManager& manager,
        std::size_t    id,
        GLFWwindow*    handle,
        Properties&&   properties,
        bool           bindImmediately
    )
        : m_manager{ &manager }
        , m_id{ id }
        , m_attachedThreadId{}
        , m_windowHandle{ handle }
        , m_properties{ std::move(properties) }
    {
        bind();

        if (auto* api = std::get_if<Api::OpenGL>(&Context::get().m_api)) {
            api->m_loader(handle, glfwGetProcAddress);
        } else if (auto* api = std::get_if<Api::OpenGLES>(&Context::get().m_api)) {
            api->m_loader(handle, glfwGetProcAddress);
        } else {
            // don't need to do anything for NoApi
        }

        glfwSetWindowPosCallback(m_windowHandle, window_pos_callback);
        glfwSetWindowSizeCallback(m_windowHandle, window_size_callback);
        glfwSetWindowCloseCallback(m_windowHandle, window_close_callback);
        glfwSetWindowRefreshCallback(m_windowHandle, window_refresh_callback);
        glfwSetWindowFocusCallback(m_windowHandle, window_focus_callback);
        glfwSetWindowIconifyCallback(m_windowHandle, window_iconify_callback);
        glfwSetFramebufferSizeCallback(m_windowHandle, framebuffer_size_callback);
        glfwSetMouseButtonCallback(m_windowHandle, mouse_button_callback);
        glfwSetCursorPosCallback(m_windowHandle, cursor_pos_callback);
        glfwSetCursorEnterCallback(m_windowHandle, cursor_enter_callback);
        glfwSetScrollCallback(m_windowHandle, scroll_callback);
        glfwSetKeyCallback(m_windowHandle, key_callback);
        glfwSetCharCallback(m_windowHandle, char_callback);

        setVsync(m_vsync);
        glfwSetWindowUserPointer(m_windowHandle, this);

        if (!bindImmediately) {
            unbind();
        }
    }

    // clang-format off
    Window::Window(Window&& other) noexcept
        : m_manager          { std::exchange(other.m_manager, nullptr) }
        , m_id               { std::exchange(other.m_id, 0) }
        , m_attachedThreadId { std::exchange(other.m_attachedThreadId, {}) }
        , m_windowHandle     { std::exchange(other.m_windowHandle, nullptr) }
        , m_properties       { std::move(other.m_properties) }
        , m_lastFrameTime    { other.m_lastFrameTime }
        , m_deltaTime        { other.m_deltaTime }
        , m_vsync            { other.m_vsync }
        , m_captureMouse     { other.m_captureMouse }
        , m_taskQueue        { std::move(other.m_taskQueue) }
        , m_eventQueue       { std::move(other.m_eventQueue) }
    // clang-format on
    {
        glfwSetWindowUserPointer(m_windowHandle, this);
    }

    Window& Window::operator=(Window&& other) noexcept
    {
        if (this == &other) {
            return *this;
        }

        if (m_windowHandle != nullptr && m_id != 0) {
            unbind();
            glfwSetWindowUserPointer(m_windowHandle, nullptr);    // remove user pointer
            m_manager->requestDeleteWindow(m_id);
        }

        m_manager          = std::exchange(other.m_manager, nullptr);
        m_id               = std::exchange(other.m_id, 0);
        m_attachedThreadId = other.m_attachedThreadId;
        m_windowHandle     = std::exchange(other.m_windowHandle, nullptr);
        m_properties       = std::move(other.m_properties);
        m_lastFrameTime    = other.m_lastFrameTime;
        m_deltaTime        = other.m_deltaTime;
        m_vsync            = other.m_vsync;
        m_captureMouse     = other.m_captureMouse;
        m_taskQueue        = std::move(other.m_taskQueue);
        m_eventQueue       = std::move(other.m_eventQueue);

        glfwSetWindowUserPointer(m_windowHandle, this);

        return *this;
    }

    Window::~Window()
    {
        if (m_windowHandle != nullptr && m_id != 0) {
            unbind();
            glfwSetWindowUserPointer(m_windowHandle, nullptr);    // remove user pointer
            m_manager->requestDeleteWindow(m_id);
        } else {
            // window is in invalid state (probably moved)
        }
    }

    void Window::bind()
    {
        if (m_attachedThreadId == std::thread::id{}) {
            // no thread attached, attach to this thread

            m_attachedThreadId = std::this_thread::get_id();
            Context::logI(std::format("(Window) Context ({}) attached (+)", m_id));

            if (!std::holds_alternative<Api::NoApi>(Context::get().m_api)) {
                glfwMakeContextCurrent(m_windowHandle);
            }

        } else if (m_attachedThreadId == std::this_thread::get_id()) {

            // same thread, do nothing

        } else {
            // different thread, cannot attach

            Context::logC(std::format(
                "(Window) Context ({}) already attached to another thread [{:#x}], cannot attach "
                "to this thread [{:#x}].",
                m_id,
                util::getThreadNum(m_attachedThreadId),
                util::getThreadNum(std::this_thread::get_id())
            ));

            // should I throw instead?
            assert(false && "Context already attached to another thread");
        }
    }

    void Window::unbind()
    {
        if (!std::holds_alternative<Api::NoApi>(Context::get().m_api)) {
            glfwMakeContextCurrent(nullptr);
        }

        if (m_attachedThreadId != std::thread::id{}) {
            Context::logI(std::format("(Window) Context ({}) detached (-)", m_id));
            m_attachedThreadId = std::thread::id{};
        }
    }

    Window& Window::setVsync(bool value)
    {
        m_vsync = value;

        // 0 = immediate updates, 1 = update synchronized with vertical retrace
        glfwSwapInterval(static_cast<int>(value));

        return *this;
    }

    void Window::setWindowSize(int width, int height)
    {
        m_properties.m_width  = width;
        m_properties.m_height = height;

        m_manager->enqueueWindowTask(m_id, [this, width, height] {
            glfwSetWindowSize(m_windowHandle, width, height);
        });
    }

    void Window::updateTitle(const std::string& title)
    {
        m_properties.m_title = title;
        m_manager->enqueueWindowTask(m_id, [this] {
            glfwSetWindowTitle(m_windowHandle, m_properties.m_title.c_str());
        });
    }

    void Window::run(Fun<void(std::deque<Event>&&)>&& func)
    {
        while (glfwWindowShouldClose(m_windowHandle) == GLFW_FALSE) {
            updateDeltaTime();
            processQueuedTasks();

            auto events = [&] {
                std::scoped_lock lock{ m_queueMutex };
                return std::exchange(m_eventQueue, {});
            }();

            func(std::move(events));

            if (!std::holds_alternative<Api::NoApi>(Context::get().m_api)) {
                glfwSwapBuffers(m_windowHandle);
            }
        }
    }

    void Window::enqueueTask(Fun<void()>&& func)
    {
        std::scoped_lock lock{ m_queueMutex };
        m_taskQueue.push_back(std::move(func));
    }

    void Window::requestClose()
    {
        glfwSetWindowShouldClose(m_windowHandle, 1);
        Context::logI(std::format("(Window) Window ({}) requested to close", m_id));
    }

    double Window::deltaTime() const
    {
        return m_deltaTime;
    }

    Window& Window::setCaptureMouse(bool value)
    {
        m_captureMouse = value;
        m_manager->enqueueTask([this] {
            if (m_captureMouse) {
                auto [x, y] = m_properties.m_cursorPos;
                glfwGetCursorPos(m_windowHandle, &x, &y);    // prevent sudden jump on first capture
                glfwSetInputMode(m_windowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(m_windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        });
        return *this;
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
}
