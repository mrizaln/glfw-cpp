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

    WindowManager::WindowManager(Context& context)
        : m_context{ &context }
        , m_attachedThreadId{ std::this_thread::get_id() }
    {
        // yeah, that's it.
        // attached thread id will not be changed for the lifetime of this class instance.
    }

    // clang-format off
    WindowManager::WindowManager(WindowManager&& other) noexcept
        : m_context          { std::exchange(other.m_context, nullptr) }
        , m_windows          { std::move(other.m_windows) }
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
            m_context           = std::exchange(other.m_context, nullptr);
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
        std::string title,
        int         width,
        int         height,
        bool        bindImmediately
    )
    {
        validateAccess(true);

        UniqueGLFWwindow glfwWindow{
            glfwCreateWindow(width, height, title.data(), nullptr, nullptr)
        };
        if (!glfwWindow) {
            m_context->logC("(WindowManager) Window creation failed");
            throw std::runtime_error{ "Failed to create window" };
        }

        auto  id     = ++m_windowCount;
        auto* handle = glfwWindow.get();
        m_windows.emplace(id, std::move(glfwWindow));

        m_context->logI(std::format("(WindowManager) Window ({}) created", id));

        return Window{
            *m_context,
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

        if (m_context == nullptr) {
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
                m_context->logI(std::format("(WindowManager) Window ({}) deleted", id));
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
                m_context->logW(std::format(
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
