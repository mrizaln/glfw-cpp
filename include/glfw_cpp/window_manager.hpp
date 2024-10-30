#ifndef WINDOW_MANAGER_HPP_OR5VIUQW
#define WINDOW_MANAGER_HPP_OR5VIUQW

#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/monitor.hpp"
#include "glfw_cpp/window.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>
#include <string_view>
#include <thread>

struct GLFWwindow;

namespace glfw_cpp
{
    // turns fps to milliseconds
    inline std::chrono::milliseconds operator""_fps(unsigned long long fps)
    {
        namespace chr = std::chrono;
        auto duration{ chr::duration_cast<chr::milliseconds>(chr::milliseconds{ 1000 } / fps) };
        return duration;
    }

    class Instance;
    class Window;
    struct IEventInterceptor;

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

        Monitor* m_monitor = nullptr;
        Window*  m_share   = nullptr;

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

    class WindowManager : public std::enable_shared_from_this<WindowManager>
    {
    public:
        friend Instance;
        friend Window;

        using Shared = std::shared_ptr<WindowManager>;

        template <typename Sig>
        using Fun = std::function<Sig>;

        class ErrorAccessFromWrongThread;

        WindowManager()                                = default;
        ~WindowManager()                               = default;
        WindowManager(const WindowManager&)            = delete;
        WindowManager& operator=(const WindowManager&) = delete;
        WindowManager(WindowManager&&)                 = delete;
        WindowManager& operator=(WindowManager&&)      = delete;

        // @thread_safety: call this function from the main thread only
        Window createWindow(
            const WindowHint& hint,
            std::string_view  title,
            int               width,
            int               height,
            bool              bindImmediately = true
        );

        // set an event interceptor for windows events that managed by this instance.
        // returns old IEventInterceptor (nullptr means no eventInterceptor set before).
        IEventInterceptor* setEventInterceptor(IEventInterceptor* eventInterceptor)
        {
            return std::exchange(m_eventInterceptor, eventInterceptor);
        }

        // this function poll events for all windows and then sleep for specified time. won't sleep
        // after polling events if `msPollRate` is `std::nullopt`.
        // @thread_safety: call this function from the main thread only
        void pollEvents(std::optional<std::chrono::milliseconds> pollRate = {});

        // like pollEvents, but this function will block the thread until an event is received or a
        // timeout reached.
        // @thread_safety: call this function from the main thread only
        void waitEvents(std::optional<std::chrono::milliseconds> timeout = {});

        // @thread_safety: this function can be called from any thread
        void requestDeleteWindow(Window::Handle handle);

        // this function is supposed to be called from a window thread.
        // @thread_safety: this function can be called from any thread
        void enqueueWindowTask(Window::Handle handle, Fun<void()>&& task);

        // this function can be called for any task that needs to be executed in the main thread.
        // for window task, use `enqueueWindowTask` instead.
        // @thread_safety: this function can be called from any thread
        void enqueueTask(Fun<void()>&& task);

        bool hasWindowOpened();

        std::thread::id attachedThreadId() const;

    private:
        struct GLFWwindowDeleter
        {
            void operator()(Window::Handle handle) const;
        };
        using UniqueGLFWwindow = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>;

        struct WindowTask
        {
            Window::Handle m_handle;
            Fun<void()>    m_task;
        };

        WindowManager(std::thread::id threadId);

        // send event to interceptor, returns the value the interceptor returns. but if there is not
        // interceptor, the returned value will always be true.
        bool sendInterceptEvent(Window& window, Event& event) noexcept;

        void validateAccess(bool checkThread) const;
        void checkTasks();

        mutable std::mutex m_mutex;    // protects current instance
        std::thread::id    m_attachedThreadId;

        std::vector<UniqueGLFWwindow> m_windows;
        std::vector<Window::Handle>   m_windowDeleteQueue;
        std::vector<WindowTask>       m_windowTaskQueue;    // window task (checked)
        std::vector<Fun<void()>>      m_taskQueue;          // general task

        IEventInterceptor* m_eventInterceptor = nullptr;
    };
}

#endif /* end of include guard: WINDOW_MANAGER_HPP_OR5VIUQW */
