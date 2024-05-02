#include "glfw_cpp/window_manager.hpp"
#include "glfw_cpp/context.hpp"
#include "glfw_cpp/window.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <chrono>
#include <format>
#include <functional>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

using LogLevel = glfw_cpp::Context::LogLevel;

namespace
{
    std::size_t getThreadNum(const std::thread::id& threadId)
    {
        auto hash{ std::hash<std::thread::id>{} };
        return hash(threadId);
    }

    void configureHints(glfw_cpp::WindowHint hint)
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
                    getThreadNum(initThreadId),
                    getThreadNum(std::this_thread::get_id())
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
        : m_windows          { std::move(other.m_windows) }
        , m_windowDeleteQueue{ std::move(other.m_windowDeleteQueue) }
        , m_taskQueue        { std::move(other.m_taskQueue) }
        , m_windowTaskQueue  { std::move(other.m_windowTaskQueue) }
        , m_windowCount      { std::exchange(other.m_windowCount, 0) }
        , m_attachedThreadId { std::exchange(other.m_attachedThreadId, std::thread::id{}) }
    // clang-format on
    {
    }

    WindowManager& WindowManager::operator=(WindowManager&& other) noexcept
    {
        if (this != &other) {
            m_windows           = std::move(other.m_windows);
            m_windowDeleteQueue = std::move(other.m_windowDeleteQueue);
            m_taskQueue         = std::move(other.m_taskQueue);
            m_windowTaskQueue   = std::move(other.m_windowTaskQueue);
            m_windowCount       = std::exchange(other.m_windowCount, 0);
            m_attachedThreadId  = std::exchange(other.m_attachedThreadId, std::thread::id{});
        }
        return *this;
    }

    Window WindowManager::createWindow(
        WindowHint  hint,
        std::string title,
        int         width,
        int         height,
        bool        bindImmediately
    )
    {
        validateAccess(true);

        configureHints(hint);

        auto handle = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
        if (handle == nullptr) {
            Context::logC("(WindowManager) Window creation failed");
            throw std::runtime_error{ "Failed to create window" };
        }

        auto id = ++m_windowCount;
        m_windows.emplace(id, handle);

        Context::logI(std::format("(WindowManager) Window ({}) created", id));

        return Window{
            *this,
            id,
            handle,
            WindowProperties{
                .m_title       = std::move(title),
                .m_width       = width,
                .m_height      = height,
                .m_cursorPos   = {},
                .m_mouseButton = {},
            },
            bindImmediately,
        };
    }

    void WindowManager::requestDeleteWindow(std::size_t id)
    {
        validateAccess(false);
        std::unique_lock lock{ m_mutex };

        // accept request only for available windows
        if (m_windows.contains(id)) {
            m_windowDeleteQueue.push(id);
        }
    }

    void WindowManager::pollEvents(std::optional<std::chrono::milliseconds> msPollRate)
    {
        validateAccess(true);

        if (msPollRate) {
            auto sleepUntilTime{ std::chrono::steady_clock::now() + *msPollRate };

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

    void WindowManager::waitEvents()
    {
        validateAccess(true);
        glfwWaitEvents();
        checkTasks();
    }

    bool WindowManager::hasWindowOpened()
    {
        validateAccess(false);
        return !m_windows.empty();
    }

    void WindowManager::enqueueWindowTask(std::size_t windowId, std::function<void()>&& task)
    {
        validateAccess(false);
        std::unique_lock lock{ m_mutex };
        m_windowTaskQueue.emplace(windowId, std::move(task));
    }

    void WindowManager::enqueueTask(std::function<void()>&& task)
    {
        std::unique_lock lock{ m_mutex };
        validateAccess(false);
        m_taskQueue.emplace(std::move(task));
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
        while (!m_windowDeleteQueue.empty()) {
            std::size_t windowId{ m_windowDeleteQueue.front() };
            m_windowDeleteQueue.pop();

            if (auto found{ m_windows.find(windowId) }; found != m_windows.end()) {
                const auto& [id, window]{ *found };
                Context::logI(std::format("(WindowManager) Window ({}) deleted", id));
                m_windows.erase(found);
            }
        }

        // window task requests
        while (!m_windowTaskQueue.empty()) {
            auto [id, fun]{ std::move(m_windowTaskQueue.front()) };
            m_windowTaskQueue.pop();
            if (m_windows.contains(id)) {
                fun();
            } else {
                Context::logW(std::format(
                    "(WindowManager) Task for window ({}) failed: window has destroyed", id
                ));
            }
        }

        // general task request
        while (!m_taskQueue.empty()) {
            auto fun{ std::move(m_taskQueue.front()) };
            m_taskQueue.pop();
            fun();
        }
    }
}
