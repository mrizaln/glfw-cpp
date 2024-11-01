#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/window_manager.hpp"

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
    void configureHints(const glfw_cpp::WindowHint& hint) noexcept
    {
        using F = glfw_cpp::WindowHint::FlagBit;

        const auto setFlag = [&](int flag, F bit) {
            glfwWindowHint(flag, (hint.m_flags & bit) != 0 ? GLFW_TRUE : GLFW_FALSE);
        };

        setFlag(GLFW_RESIZABLE, F::Resizable);
        setFlag(GLFW_VISIBLE, F::Visible);
        setFlag(GLFW_DECORATED, F::Decorated);
        setFlag(GLFW_FOCUSED, F::Focused);
        setFlag(GLFW_AUTO_ICONIFY, F::AutoIconify);
        setFlag(GLFW_FLOATING, F::Floating);
        setFlag(GLFW_MAXIMIZED, F::Maximized);
        setFlag(GLFW_CENTER_CURSOR, F::CenterCursor);
        setFlag(GLFW_TRANSPARENT_FRAMEBUFFER, F::TransparentFramebuffer);
        setFlag(GLFW_FOCUS_ON_SHOW, F::FocusOnShow);
        setFlag(GLFW_SCALE_TO_MONITOR, F::ScaleToMonitor);

        glfwWindowHint(GLFW_RED_BITS, hint.m_redBits);
        glfwWindowHint(GLFW_GREEN_BITS, hint.m_greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, hint.m_blueBits);
        glfwWindowHint(GLFW_ALPHA_BITS, hint.m_alphaBits);
        glfwWindowHint(GLFW_DEPTH_BITS, hint.m_depthBits);
        glfwWindowHint(GLFW_STENCIL_BITS, hint.m_stencilBits);

        glfwWindowHint(GLFW_SAMPLES, hint.m_samples);
        glfwWindowHint(GLFW_REFRESH_RATE, hint.m_refreshRate);
    }
}

namespace glfw_cpp
{
    void WindowManager::GLFWwindowDeleter::operator()(GLFWwindow* handle) const noexcept
    {
        glfwDestroyWindow(handle);
    };

    WindowManager::WindowManager(std::thread::id threadId) noexcept
        : m_attachedThreadId{ threadId }
    {
        // yeah, that's it.
        // attached thread id will not be changed for the lifetime of this class instance.
    }

    Window WindowManager::createWindow(
        const WindowHint& hint,
        std::string_view  title,
        int               width,
        int               height,
        bool              bindImmediately
    )
    {
        validateAccess();
        configureHints(hint);

        const auto handle = glfwCreateWindow(
            width,
            height,
            title.data(),
            hint.m_monitor ? hint.m_monitor->handle() : nullptr,
            hint.m_share ? hint.m_share->handle() : nullptr
        );
        if (handle == nullptr) {
            Instance::logC("(WindowManager) Window creation failed");
            util::throwGlfwError();
        }
        m_windows.emplace_back(handle);

        Instance::logI("(WindowManager) Window ({:#x}) created", (std::size_t)handle);

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

        int    xPos, yPos, realWidth, realHeight, fbWidth, fbHeight;
        double xCursor, yCursor;
        glfwGetWindowPos(handle, &xPos, &yPos);
        glfwGetWindowSize(handle, &realWidth, &realHeight);
        glfwGetCursorPos(handle, &xCursor, &yCursor);
        glfwGetFramebufferSize(handle, &fbWidth, &fbHeight);

        auto wmCopy = std::enable_shared_from_this<WindowManager>::shared_from_this();

        return Window{ wmCopy, handle, Window::Properties{
            .m_title           = { title.begin(), title.end() },
            .m_pos             = { xPos, yPos },
            .m_dimension       = { realWidth, realHeight },
            .m_framebufferSize = { fbWidth, fbHeight },
            .m_cursor          = { xCursor, yCursor },
            .m_attribute       = {
                .m_iconified   = 0,
                .m_maximized   = (hint.m_flags & WindowHint::Maximized) != 0,
                .m_focused     = (hint.m_flags & WindowHint::Focused) != 0,
                .m_visible     = (hint.m_flags & WindowHint::Visible) != 0,
                .m_hovered     = (unsigned int)glfwGetWindowAttrib(handle, GLFW_HOVERED),
                .m_resizable   = (hint.m_flags & WindowHint::Resizable) != 0,
                .m_floating    = (hint.m_flags & WindowHint::Floating) != 0,
                .m_autoIconify = (hint.m_flags & WindowHint::AutoIconify) != 0,
                .m_focusOnShow = (hint.m_flags & WindowHint::FocusOnShow) != 0,
            },
        }, bindImmediately };
    }

    bool WindowManager::hasWindowOpened()
    {
        using Ptr                 = decltype(m_windows)::value_type;
        const auto shouldNotClose = [](Window::Handle h) { return glfwWindowShouldClose(h) != GLFW_TRUE; };
        return std::ranges::any_of(m_windows, shouldNotClose, &Ptr::get);
    }

    void WindowManager::pollEvents(std::optional<std::chrono::milliseconds> pollRate)
    {
        validateAccess();

        if (pollRate) {
            auto sleepUntilTime{ std::chrono::steady_clock::now() + *pollRate };

            glfwPollEvents();
            checkTasks();

            if (sleepUntilTime > std::chrono::steady_clock::now()) {
                std::this_thread::sleep_until(sleepUntilTime);
            }
        } else {
            glfwPollEvents();
            checkTasks();
        }
    }

    void WindowManager::waitEvents(std::optional<std::chrono::milliseconds> timeout)
    {
        validateAccess();
        if (timeout) {
            using SecondsDouble = std::chrono::duration<double>;
            const auto seconds  = std::chrono::duration_cast<SecondsDouble>(*timeout);
            glfwWaitEventsTimeout(seconds.count());
        } else {
            glfwWaitEvents();
        }
        checkTasks();
    }

    void WindowManager::requestDeleteWindow(Window::Handle handle)
    {
        std::unique_lock lock{ m_mutex };

        // accept request only for available windows
        if (std::ranges::find(m_windows, handle, &UniqueGLFWwindow::get) != m_windows.end()) {
            m_windowDeleteQueue.push_back(handle);
        }
    }

    void WindowManager::enqueueWindowTask(Window::Handle handle, Fun<void()>&& task)
    {
        std::unique_lock lock{ m_mutex };
        m_windowTaskQueue.emplace_back(handle, std::move(task));
    }

    void WindowManager::enqueueTask(Fun<void()>&& task)
    {
        std::unique_lock lock{ m_mutex };
        m_taskQueue.emplace_back(std::move(task));
    }

    void WindowManager::validateAccess() const
    {
        if (m_attachedThreadId != std::this_thread::get_id()) {
            throw WrongThreadAccess{
                util::getThreadNum(m_attachedThreadId),
                util::getThreadNum(std::this_thread::get_id()),
            };
        }
    }

    void WindowManager::checkTasks()
    {
        // window deletion
        for (auto handle : util::lockExchange(m_mutex, m_windowDeleteQueue, {})) {
            if (std::erase_if(m_windows, [handle](auto& e) { return e.get() == handle; }) != 0) {
                Instance::logI("(WindowManager) Window ({:#x}) deleted", (std::size_t)handle);
            }
        }

        // window task requests
        for (auto&& [handle, task] : util::lockExchange(m_mutex, m_windowTaskQueue, {})) {
            if (std::ranges::find(m_windows, handle, &UniqueGLFWwindow::get) != m_windows.end()) {
                task();
            } else {
                Instance::logW(
                    "(WindowManager) Task for window ({:#x}) failed: window has destroyed",
                    (std::size_t)handle
                );
            }
        }

        // general task request
        for (auto&& task : std::exchange(m_taskQueue, {})) {
            task();
        }
    }

    bool WindowManager::sendInterceptEvent(Window& window, Event& event) noexcept
    {
        if (!m_eventInterceptor) {
            return true;
        }

        auto& intr = *m_eventInterceptor;
        return event.visit(Event::Overloaded{
            // clang-format off
            [&](Event::WindowMoved&        event) { return intr.onWindowMoved       (window, event); },
            [&](Event::WindowResized&      event) { return intr.onWindowResized     (window, event); },
            [&](Event::WindowClosed&       event) { return intr.onWindowClosed      (window, event); },
            [&](Event::WindowRefreshed&    event) { return intr.onWindowRefreshed   (window, event); },
            [&](Event::WindowFocused&      event) { return intr.onWindowFocused     (window, event); },
            [&](Event::WindowIconified&    event) { return intr.onWindowIconified   (window, event); },
            [&](Event::WindowMaximized&    event) { return intr.onWindowMaximized   (window, event); },
            [&](Event::WindowScaleChanged& event) { return intr.onWindowScaleChanged(window, event); },
            [&](Event::FramebufferResized& event) { return intr.onFramebufferResized(window, event); },
            [&](Event::ButtonPressed&      event) { return intr.onButtonPressed     (window, event); },
            [&](Event::CursorMoved&        event) { return intr.onCursorMoved       (window, event); },
            [&](Event::CursorEntered&      event) { return intr.onCursorEntered     (window, event); },
            [&](Event::Scrolled&           event) { return intr.onScrolled          (window, event); },
            [&](Event::KeyPressed&         event) { return intr.onKeyPressed        (window, event); },
            [&](Event::CharInput&          event) { return intr.onCharInput         (window, event); },
            [&](Event::FileDropped&        event) { return intr.onFileDropped       (window, event); },
            [&](Event::Empty&                   ) { return true; /* always true                   */ },
            // clang-format on
        });
    }
}
