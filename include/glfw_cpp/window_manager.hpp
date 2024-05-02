#ifndef WINDOW_MANAGER_HPP_OR5VIUQW
#define WINDOW_MANAGER_HPP_OR5VIUQW

#include "glfw_cpp/context.hpp"

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
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

    struct WindowHint
    {
        using Flag = std::int32_t;
        enum FlagBit : Flag
        {
            RESIZABLE               = 1 << 0,
            VISIBLE                 = 1 << 1,
            DECORATED               = 1 << 2,
            FOCUSED                 = 1 << 3,
            AUTO_ICONIFY            = 1 << 4,
            FLOATING                = 1 << 5,
            MAXIMIZED               = 1 << 6,
            CENTER_CURSOR           = 1 << 7,
            TRANSPARENT_FRAMEBUFFER = 1 << 8,
            FOCUS_ON_SHOW           = 1 << 9,
            SCALE_TO_MONITOR        = 1 << 10,

            DEFAULT = RESIZABLE | VISIBLE | DECORATED | FOCUSED | AUTO_ICONIFY | FOCUS_ON_SHOW,
        };

        Flag m_flags = FlagBit::DEFAULT;

        int m_redBits     = 8;
        int m_greenBits   = 8;
        int m_blueBits    = 8;
        int m_alphaBits   = 8;
        int m_depthBits   = 24;
        int m_stencilBits = 8;

        int m_samples     = 0;
        int m_refreshRate = -1;
    };

    class WindowManager
    {
    public:
        class ErrorAccessFromWrongThread;

        WindowManager(Context& context);
        WindowManager(WindowManager&&) noexcept;
        WindowManager& operator=(WindowManager&&) noexcept;

        WindowManager()                                = default;
        ~WindowManager()                               = default;
        WindowManager(const WindowManager&)            = delete;
        WindowManager& operator=(const WindowManager&) = delete;

        // @thread_safety: call this function from the main thread only
        Window createWindow(
            WindowHint  hint,
            std::string title,
            int         width,
            int         height,
            bool        bindImmediately = true
        );

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
        struct GLFWwindowDeleter
        {
            void operator()(GLFWwindow* handle) const;
        };
        using UniqueGLFWwindow = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>;

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
