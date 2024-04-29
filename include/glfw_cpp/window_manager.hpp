#ifndef WINDOW_MANAGER_HPP_OR5VIUQW
#define WINDOW_MANAGER_HPP_OR5VIUQW

#include "glfw_cpp/context.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <chrono>
#include <cstddef>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <utility>

namespace glfw_cpp
{
    // turns fps to milliseconds
    inline std::chrono::milliseconds operator""_fps(unsigned long long fps)
    {
        using namespace std::chrono_literals;
        auto duration{ std::chrono::duration_cast<std::chrono::milliseconds>(1000ms / fps) };
        return duration;
    }

    class Window;

    class WindowManager
    {
    public:
        class ErrorAccessFromWrongThread;

        class UniqueGLFWwindow
        {
        public:
            UniqueGLFWwindow(GLFWwindow* handle)
                : m_handle{ handle }
            {
            }

            UniqueGLFWwindow(const UniqueGLFWwindow&)            = delete;
            UniqueGLFWwindow& operator=(const UniqueGLFWwindow&) = delete;

            UniqueGLFWwindow(UniqueGLFWwindow&& other) noexcept
                : m_handle{ std::exchange(other.m_handle, nullptr) }
            {
            }

            UniqueGLFWwindow& operator=(UniqueGLFWwindow&& other) noexcept
            {
                if (this != &other) {
                    m_handle = std::exchange(other.m_handle, nullptr);
                }
                return *this;
            }

            ~UniqueGLFWwindow()
            {
                if (m_handle != nullptr) {
                    glfwDestroyWindow(m_handle);
                }
            }

            operator bool() const { return m_handle != nullptr; }
            GLFWwindow* get() { return m_handle; }

        private:
            GLFWwindow* m_handle;
        };

        WindowManager(Context& context);
        WindowManager(WindowManager&&) noexcept;
        WindowManager& operator=(WindowManager&&) noexcept;

        WindowManager()                                = default;
        ~WindowManager()                               = default;
        WindowManager(const WindowManager&)            = delete;
        WindowManager& operator=(const WindowManager&) = delete;

        // @thread_safety: call this function from the main thread only
        Window createWindow(std::string title, int width, int height, bool bindImmediately = true);

        // this function poll events for all windows and then sleep for specified time. won't sleep
        // after polling events if `msPollRate` is `std::nullopt`.
        // @thread_safety: call this function from the main thread only
        void pollEvents(std::optional<std::chrono::milliseconds> msPollRate = {});

        // like pollEvents, but this function will block the thread until an event is received.
        // @thread_safety: call this function from the main thread only
        void waitEvents();

        // @thread_safety: this function can be called from any thread
        void requestDeleteWindow(std::size_t id);

        // this function is supposed to be called from a window thread.
        // @thread_safety: this function can be called from any thread
        void enqueueWindowTask(std::size_t windowId, std::function<void()>&& task);

        // this function can be called for any task that needs to be executed in the main thread.
        // for window task, use `enqueueWindowTask` instead.
        // @thread_safety: this function can be called from any thread
        void enqueueTask(std::function<void()>&& task);

        bool hasWindowOpened();

        std::thread::id attachedThreadId() const;

    private:
        Context*           m_context;
        mutable std::mutex m_mutex;    // protects current instance

        std::unordered_map<std::size_t, UniqueGLFWwindow>         m_windows;
        std::queue<std::size_t>                                   m_windowDeleteQueue;
        std::queue<std::function<void()>>                         m_taskQueue;
        std::queue<std::pair<std::size_t, std::function<void()>>> m_windowTaskQueue;

        std::size_t     m_windowCount{ 0 };
        std::thread::id m_attachedThreadId;

        void validateAccess(bool checkThread) const;

        void checkTasks();
    };

}

#endif /* end of include guard: WINDOW_MANAGER_HPP_OR5VIUQW */
