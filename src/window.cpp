#include "glfw_cpp/window.hpp"
#include "glfw_cpp/context.hpp"
#include "glfw_cpp/window_manager.hpp"
#include <variant>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <format>
#include <functional>
#include <mutex>
#include <optional>
#include <ranges>
#include <thread>
#include <utility>

namespace
{
    std::size_t getThreadNum(const std::thread::id& threadId)
    {
        auto hash{ std::hash<std::thread::id>{} };
        return hash(threadId);
    }

    // helper function that decides whether to execute the task immediately or enqueue it.
    void runTask(
        glfw_cpp::Window*        windowPtr,
        glfw_cpp::WindowManager* manager,
        std::function<void()>&&  func
    )
    {
        // If the Window is attached to the same thread as the windowManager, execute the task
        // immediately, else enqueue the task.
        if (windowPtr->attachedThreadId() == manager->attachedThreadId()) {
            func();
        } else {
            windowPtr->enqueueTask(std::move(func));
        }
    }
}

namespace glfw_cpp
{
    void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        Window* windowWindow{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };
        if (windowWindow == nullptr) {
            return;
        }

        runTask(windowWindow, windowWindow->m_manager, [windowWindow, width, height] {
            if (windowWindow->m_framebufferSizeCallback) {
                windowWindow->m_framebufferSizeCallback(*windowWindow, width, height);
            }
            windowWindow->setWindowSize(width, height);
        });
    }

    void Window::keyCallback(GLFWwindow* window, int key, int /* scancode */, int action, int mods)
    {
        auto* windowWindow{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };
        if (windowWindow == nullptr) {
            return;
        }

        runTask(windowWindow, windowWindow->m_manager, [windowWindow, key, action, mods] {
            auto& keyMap{ windowWindow->m_keyMap };
            auto  range{ keyMap.equal_range(key) };
            for (auto& [_, handler] : std::ranges::subrange(range.first, range.second)) {
                auto& [hmod, haction, hfun]{ handler };
                if (haction != KeyActionType::CALLBACK) {
                    continue;
                }

                // ignore release and repeat event for now
                if (action == GLFW_RELEASE || action == GLFW_REPEAT) {
                    continue;
                }

                // modifier match
                if ((mods & hmod) != 0) {
                    hfun(*windowWindow);
                }

                // don't have modifier
                if (mods == 0 && hmod == 0) {
                    hfun(*windowWindow);
                }
            }
        });
    }

    void Window::cursorPosCallback(GLFWwindow* window, double xPos, double yPos)
    {
        auto* windowWindow{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };
        if (windowWindow == nullptr) {
            return;
        }

        runTask(windowWindow, windowWindow->m_manager, [windowWindow, xPos, yPos] {
            if (windowWindow->m_cursorPosCallback) {
                windowWindow->m_cursorPosCallback(*windowWindow, xPos, yPos);
            }
            windowWindow->m_properties.m_cursorPos = { xPos, yPos };
        });
    }

    void Window::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
    {
        auto* windowWindow{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };
        if (windowWindow == nullptr) {
            return;
        }

        runTask(windowWindow, windowWindow->m_manager, [windowWindow, xOffset, yOffset] {
            if (windowWindow->m_scrollCallback) {
                windowWindow->m_scrollCallback(*windowWindow, xOffset, yOffset);
            }
        });
    }

    void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        auto* windowWindow{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };
        if (windowWindow == nullptr) {
            return;
        }

        MouseButton::Button buttonButton;
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT: buttonButton = MouseButton::Button::LEFT; break;
        case GLFW_MOUSE_BUTTON_RIGHT: buttonButton = MouseButton::Button::RIGHT; break;
        case GLFW_MOUSE_BUTTON_MIDDLE: buttonButton = MouseButton::Button::MIDDLE; break;
        default: return;    // ignore other buttons
        }

        MouseButton::State buttonState{ action == GLFW_PRESS ? MouseButton::State::PRESSED
                                                             : MouseButton::State::RELEASED };

        runTask(
            windowWindow,
            windowWindow->m_manager,
            [windowWindow, buttonButton, buttonState, mods] {
                if (windowWindow->m_mouseButtonCallback) {
                    windowWindow->m_mouseButtonCallback(
                        *windowWindow, buttonButton, buttonState, mods
                    );
                }
                windowWindow->m_properties.m_mouseButton[buttonButton] = buttonState;
            }
        );
    }

    // this constructor must be called only from main thread (WindowManager run in main thread)
    Window::Window(
        Context&           context,
        WindowManager&     manager,
        std::size_t        id,
        GLFWwindow*        handle,
        WindowProperties&& properties,
        bool               bindImmediately
    )
        : m_context{ &context }
        , m_manager{ &manager }
        , m_id{ id }
        , m_windowHandle{ handle }
        , m_properties{ std::move(properties) }
        , m_attachedThreadId{ std::nullopt }
    {
        bind();
        if (!m_contextInitialized) {
            if (auto* api = std::get_if<Api::OpenGL>(&m_context->m_api)) {
                api->m_loader(handle, glfwGetProcAddress);
            } else if (auto* api = std::get_if<Api::OpenGLES>(&m_context->m_api)) {
                api->m_loader(handle, glfwGetProcAddress);
            } else {
                // don't need to do anything for NoApi
            }
            m_contextInitialized = true;

            glfwSetFramebufferSizeCallback(m_windowHandle, Window::framebufferSizeCallback);
            glfwSetKeyCallback(m_windowHandle, Window::keyCallback);
            glfwSetCursorPosCallback(m_windowHandle, Window::cursorPosCallback);
            glfwSetScrollCallback(m_windowHandle, Window::scrollCallback);
            glfwSetMouseButtonCallback(m_windowHandle, mouseButtonCallback);
        }
        setVsync(m_vsync);
        glfwSetWindowUserPointer(m_windowHandle, this);

        if (!bindImmediately) {
            unbind();
        }
    }

    // clang-format off
    Window::Window(Window&& other) noexcept
        : m_context                { std::exchange(other.m_context, nullptr) }
        , m_manager                { std::exchange(other.m_manager, nullptr) }
        , m_id                     { std::exchange(other.m_id, 0) }
        , m_contextInitialized     { std::exchange(other.m_contextInitialized, false) }
        , m_windowHandle           { std::exchange(other.m_windowHandle, nullptr) }
        , m_properties             { std::move(other.m_properties) }
        , m_vsync                  { other.m_vsync }
        , m_keyMap                 { std::move(other.m_keyMap) }
        , m_cursorPosCallback      { std::exchange(other.m_cursorPosCallback, nullptr) }
        , m_scrollCallback         { std::exchange(other.m_scrollCallback, nullptr) }
        , m_framebufferSizeCallback{ std::exchange(other.m_framebufferSizeCallback, nullptr) }
        , m_mouseButtonCallback    { std::exchange(other.m_mouseButtonCallback, nullptr) }
        , m_taskQueue              { std::move(other.m_taskQueue) }
        , m_lastFrameTime          { other.m_lastFrameTime }
        , m_deltaTime              { other.m_deltaTime }
        , m_attachedThreadId       { std::exchange(other.m_attachedThreadId, std::nullopt) }
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

        m_context                 = std::exchange(other.m_context, nullptr);
        m_manager                 = std::exchange(other.m_manager, nullptr);
        m_id                      = std::exchange(other.m_id, 0);
        m_contextInitialized      = std::exchange(other.m_contextInitialized, false);
        m_windowHandle            = std::exchange(other.m_windowHandle, nullptr);
        m_properties              = std::move(other.m_properties);
        m_vsync                   = other.m_vsync;
        m_keyMap                  = std::move(other.m_keyMap);
        m_cursorPosCallback       = std::exchange(other.m_cursorPosCallback, nullptr);
        m_scrollCallback          = std::exchange(other.m_scrollCallback, nullptr);
        m_framebufferSizeCallback = std::exchange(other.m_framebufferSizeCallback, nullptr);
        m_mouseButtonCallback     = std::exchange(other.m_mouseButtonCallback, nullptr);
        m_taskQueue               = std::move(other.m_taskQueue);
        m_lastFrameTime           = other.m_lastFrameTime;
        m_deltaTime               = other.m_deltaTime;
        m_attachedThreadId        = other.m_attachedThreadId;

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
        if (!m_attachedThreadId.has_value()) {
            // no thread attached, attach to this thread

            m_attachedThreadId = std::this_thread::get_id();
            m_context->logI(std::format("(Window) Context ({}) attached (+)", m_id));

            if (!std::holds_alternative<Api::NoApi>(m_context->m_api)) {
                glfwMakeContextCurrent(m_windowHandle);
            }

        } else if (m_attachedThreadId == std::this_thread::get_id()) {

            // same thread, do nothing

        } else {
            // different thread, cannot attach

            m_context->logC(std::format(
                "(Window) Context ({}) already attached to another thread [{:#x}], cannot attach "
                "to this thread [{:#x}].",
                m_id,
                getThreadNum(*m_attachedThreadId),
                getThreadNum(std::this_thread::get_id())
            ));

            // should I throw instead?
            assert(false && "Context already attached to another thread");
        }
    }

    void Window::unbind()
    {
        if (!std::holds_alternative<Api::NoApi>(m_context->m_api)) {
            glfwMakeContextCurrent(nullptr);
        }

        if (m_attachedThreadId.has_value()) {
            m_context->logI(std::format("(Window) Context ({}) detached (-)", m_id));
        }

        m_attachedThreadId.reset();
    }

    void Window::use(std::invocable auto&& fn)
    {
        bind();
        fn();
        unbind();
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
    }

    void Window::updateTitle(const std::string& title)
    {
        m_properties.m_title = title;
        m_manager->enqueueWindowTask(m_id, [this] {
            glfwSetWindowTitle(m_windowHandle, m_properties.m_title.c_str());
        });
    }

    void Window::run(std::function<void()>&& func)
    {
        while (glfwWindowShouldClose(m_windowHandle) == GLFW_FALSE) {
            {
                std::scoped_lock lock{ m_windowMutex };
                updateDeltaTime();
                processInput();
                processQueuedTasks();
                func();
            }

            if (!std::holds_alternative<Api::NoApi>(m_context->m_api)) {
                glfwSwapBuffers(m_windowHandle);
            }
        }
    }

    void Window::enqueueTask(std::function<void()>&& func)
    {
        std::scoped_lock lock{ m_queueMutex };
        m_taskQueue.push(std::move(func));
    }

    void Window::requestClose()
    {
        glfwSetWindowShouldClose(m_windowHandle, 1);
        m_context->logI(std::format("(Window) Window ({}) requested to close", m_id));
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

    Window& Window::setCursorPosCallback(CursorPosCallbackFun&& func)
    {
        m_cursorPosCallback = std::move(func);
        return *this;
    }

    Window& Window::setScrollCallback(ScrollCallbackFun&& func)
    {
        m_scrollCallback = std::move(func);
        return *this;
    }

    Window& Window::setFramebuffersizeCallback(FramebufferSizeCallbackFun&& func)
    {
        m_framebufferSizeCallback = std::move(func);
        return *this;
    }

    Window& Window::setMouseButtonCallback(MouseButtonCallback&& func)
    {
        m_mouseButtonCallback = std::move(func);
        return *this;
    }

    Window& Window::addKeyEventHandler(
        KeyEvent                       key,
        KeyModifier                    mods,
        KeyActionType                  action,
        std::function<void(Window&)>&& func
    )
    {
        m_keyMap.emplace(key, KeyEventHandler{ .mods = mods, .action = action, .handler = func });
        return *this;
    }

    Window& Window::addKeyEventHandler(
        std::initializer_list<KeyEvent> keys,
        KeyModifier                     mods,
        KeyActionType                   action,
        std::function<void(Window&)>&&  func
    )
    {
        for (auto key : keys) {
            m_keyMap.emplace(
                key, KeyEventHandler{ .mods = mods, .action = action, .handler = func }
            );
        }
        return *this;
    }

    void Window::processInput()
    {
        // TODO: move this part to 'main thread' (glfwGetKey must be called from main thread [for
        // now it's okay, idk why tho])
        const auto getMods = [win = m_windowHandle] {
            int mods{ 0 };
            mods |= glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) & GLFW_MOD_SHIFT;
            mods |= glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) & GLFW_MOD_SHIFT;
            mods |= glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) & GLFW_MOD_CONTROL;
            mods |= glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) & GLFW_MOD_CONTROL;
            mods |= glfwGetKey(win, GLFW_KEY_LEFT_ALT) & GLFW_MOD_ALT;
            mods |= glfwGetKey(win, GLFW_KEY_RIGHT_ALT) & GLFW_MOD_ALT;
            mods |= glfwGetKey(win, GLFW_KEY_LEFT_SUPER) & GLFW_MOD_SUPER;
            mods |= glfwGetKey(win, GLFW_KEY_RIGHT_SUPER) & GLFW_MOD_SUPER;

            return mods;
        };
        auto mods{ getMods() };

        // continuous key input
        for (auto& [key, handler] : m_keyMap) {
            auto& [hmod, haction, hfun]{ handler };
            if (haction != KeyActionType::CONTINUOUS) {
                continue;
            }
            if (glfwGetKey(m_windowHandle, key) != GLFW_PRESS) {
                continue;
            }

            if (((mods & hmod) != 0) || hmod == 0) {    // modifier match or don't have any modifier
                hfun(*this);
            }
        }
    }

    void Window::processQueuedTasks()
    {
        decltype(m_taskQueue) taskQueue;
        {
            std::scoped_lock lock{ m_queueMutex };
            taskQueue.swap(m_taskQueue);
        }
        while (!taskQueue.empty()) {
            auto&& func{ taskQueue.front() };
            func();
            taskQueue.pop();
        }
    }

    void Window::updateDeltaTime()
    {
        double currentTime{ glfwGetTime() };
        m_deltaTime     = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;
    }
}
