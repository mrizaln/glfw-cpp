#include "glfw_cpp/window_manager.hpp"
#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/window.hpp"

#include "util.hpp"
#include <algorithm>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <chrono>
#include <format>
#include <functional>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>

using LogLevel = glfw_cpp::Instance::LogLevel;

namespace
{
    void configureHints(const glfw_cpp::WindowHint& hint)
    {
        using F = glfw_cpp::WindowHint::FlagBit;

        const auto setFlag = [&](int flag, F bit) {
            glfwWindowHint(flag, (hint.m_flags & bit) != 0 ? GLFW_TRUE : GLFW_FALSE);
        };

        setFlag(GLFW_RESIZABLE, F::RESIZABLE);
        setFlag(GLFW_VISIBLE, F::VISIBLE);
        setFlag(GLFW_DECORATED, F::DECORATED);
        setFlag(GLFW_FOCUSED, F::FOCUSED);
        setFlag(GLFW_AUTO_ICONIFY, F::AUTO_ICONIFY);
        setFlag(GLFW_FLOATING, F::FLOATING);
        setFlag(GLFW_MAXIMIZED, F::MAXIMIZED);
        setFlag(GLFW_CENTER_CURSOR, F::CENTER_CURSOR);
        setFlag(GLFW_TRANSPARENT_FRAMEBUFFER, F::TRANSPARENT_FRAMEBUFFER);
        setFlag(GLFW_FOCUS_ON_SHOW, F::FOCUS_ON_SHOW);
        setFlag(GLFW_SCALE_TO_MONITOR, F::SCALE_TO_MONITOR);

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
    class WindowManager::ErrorAccessFromWrongThread : public std::runtime_error
    {
    public:
        ErrorAccessFromWrongThread(std::thread::id initThreadId)
            : std::runtime_error{
                std::format(
                    "(WindowManager) Instance accessed from different thread from initialization! "
                    "[at init: {} | current: {}]",
                    util::getThreadNum(initThreadId),
                    util::getThreadNum(std::this_thread::get_id())
                ),
            }
        {
        }
    };

    void WindowManager::GLFWwindowDeleter::operator()(GLFWwindow* handle) const
    {
        glfwDestroyWindow(handle);
    };

    WindowManager::WindowManager(std::thread::id threadId)
        : m_attachedThreadId{ threadId }
    {
        // yeah, that's it.
        // attached thread id will not be changed for the lifetime of this class instance.
    }

    // clang-format off
    WindowManager::WindowManager(WindowManager&& other) noexcept
        : m_attachedThreadId { std::exchange(other.m_attachedThreadId, std::thread::id{}) }
        , m_windows          { std::move(other.m_windows) }
        , m_windowDeleteQueue{ std::move(other.m_windowDeleteQueue) }
        , m_windowTaskQueue  { std::move(other.m_windowTaskQueue) }
        , m_taskQueue        { std::move(other.m_taskQueue) }
    // clang-format on
    {
    }

    WindowManager& WindowManager::operator=(WindowManager&& other) noexcept
    {
        if (this != &other) {
            m_attachedThreadId  = std::exchange(other.m_attachedThreadId, std::thread::id{});
            m_windows           = std::move(other.m_windows);
            m_windowDeleteQueue = std::move(other.m_windowDeleteQueue);
            m_windowTaskQueue   = std::move(other.m_windowTaskQueue);
            m_taskQueue         = std::move(other.m_taskQueue);
        }
        return *this;
    }

    Window WindowManager::createWindow(
        const WindowHint& hint,
        std::string_view  title,
        int               width,
        int               height,
        bool              bindImmediately
    )
    {
        validateAccess(true);

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
            throw std::runtime_error{ "Failed to create window" };
        }
        m_windows.emplace_back(handle);

        Instance::logI("(WindowManager) Window ({:#x}) created", (std::size_t)handle);

        Window::Properties prop{ .m_title = { title.begin(), title.end() } };

        glfwGetWindowPos(handle, &prop.m_pos.m_x, &prop.m_pos.m_y);
        glfwGetWindowSize(handle, &prop.m_dimension.m_width, &prop.m_dimension.m_height);
        glfwGetCursorPos(handle, &prop.m_cursor.m_x, &prop.m_cursor.m_y);

        return Window{ *this, handle, std::move(prop), bindImmediately };
    }

    void WindowManager::requestDeleteWindow(WindowManager::Handle handle)
    {
        validateAccess(false);
        std::unique_lock lock{ m_mutex };

        // accept request only for available windows
        if (std::ranges::find(m_windows, handle, &UniqueGLFWwindow::get) != m_windows.end()) {
            m_windowDeleteQueue.push_back(handle);
        }
    }

    void WindowManager::pollEvents(std::optional<std::chrono::milliseconds> pollRate)
    {
        validateAccess(true);

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
        validateAccess(true);
        if (timeout) {
            using SecondsDouble = std::chrono::duration<double>;
            const auto seconds  = std::chrono::duration_cast<SecondsDouble>(*timeout);
            glfwWaitEventsTimeout(seconds.count());
        } else {
            glfwWaitEvents();
        }
        checkTasks();
    }

    bool WindowManager::hasWindowOpened()
    {
        validateAccess(false);
        return !m_windows.empty();
    }

    void WindowManager::enqueueWindowTask(Handle handle, Fun<void()>&& task)
    {
        validateAccess(false);
        std::unique_lock lock{ m_mutex };
        m_windowTaskQueue.emplace_back(handle, std::move(task));
    }

    void WindowManager::enqueueTask(Fun<void()>&& task)
    {
        std::unique_lock lock{ m_mutex };
        validateAccess(false);
        m_taskQueue.emplace_back(std::move(task));
    }

    std::thread::id WindowManager::attachedThreadId() const
    {
        validateAccess(false);
        return m_attachedThreadId;
    }

    void WindowManager::validateAccess(bool checkThread) const
    {
        if (checkThread && m_attachedThreadId != std::this_thread::get_id()) {
            throw ErrorAccessFromWrongThread{ m_attachedThreadId };
        }

        if (m_attachedThreadId == std::thread::id{}) {
            throw std::runtime_error{ "WindowManager instance is moved" };
        }
    }

    void WindowManager::checkTasks()
    {
        // window deletion
        for (auto handle : std::exchange(m_windowDeleteQueue, {})) {
            if (std::erase_if(m_windows, [handle](auto& e) { return e.get() == handle; }) != 0) {
                Instance::logI("(WindowManager) Window ({:#x}) deleted", (std::size_t)handle);
            }
        }

        // window task requests
        for (auto&& [handle, task] : std::exchange(m_windowTaskQueue, {})) {
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
}
