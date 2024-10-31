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
    /**
     * @brief Turns fps to milliseconds.
     */
    inline std::chrono::milliseconds operator""_fps(unsigned long long fps)
    {
        namespace chr = std::chrono;
        auto duration{ chr::duration_cast<chr::milliseconds>(chr::milliseconds{ 1000 } / fps) };
        return duration;
    }

    class Instance;
    class Window;
    struct IEventInterceptor;

    /**
     * @struct WindowHint
     * @brief A struct that holds window hints.
     *
     * The window hints included here are only the relevant ones. Graphics API is omitted since the API is not
     * allowed to change at runtime (my design choice).
     */
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

    /**
     * @class WindowManager
     * @brief A class that manages windows.
     *
     * This class is responsible for creating and managing windows. It is also responsible for polling events
     * and executing tasks in the main thread.
     */
    class WindowManager : public std::enable_shared_from_this<WindowManager>
    {
    public:
        friend Instance;
        friend Window;

        using Shared = std::shared_ptr<WindowManager>;

        template <typename Sig>
        using Fun = std::function<Sig>;

        WindowManager()                                = default;
        ~WindowManager()                               = default;
        WindowManager(const WindowManager&)            = delete;
        WindowManager& operator=(const WindowManager&) = delete;
        WindowManager(WindowManager&&)                 = delete;
        WindowManager& operator=(WindowManager&&)      = delete;

        /**
         * @brief Create a window.
         *
         * @param hint The window hint.
         * @param title The window title.
         * @param width The window width.
         * @param height The window height.
         * @param bindImmediately Whether to bind the window immediately to current thread.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw glfw_cpp::WrongThreadAccess The function is called not from the main thread.
         * @throw glfw_cpp::ApiUnavailable The requested client API is unavailable.
         * @throw glfw_cpp::VersionUnavailable The requested client API version is unavailable.
         * @throw glfw_cpp::FormatUnavailable The requested format is unavailable.
         * @throw glfw_cpp::NoWindowContext The specified window does not have an OpenGL or OpenGL ES context.
         * @throw glfw_cpp::PlatformError A platform-specific error occurred.
         */
        Window createWindow(
            const WindowHint& hint,
            std::string_view  title,
            int               width,
            int               height,
            bool              bindImmediately = true
        );

        /**
         * @brief Set an event interceptor.
         *
         * @param eventInterceptor The event interceptor.
         *
         * @return The old event interceptor.
         */
        IEventInterceptor* setEventInterceptor(IEventInterceptor* eventInterceptor) noexcept
        {
            return std::exchange(m_eventInterceptor, eventInterceptor);
        }

        /**
         * @brief Check if any window managed by this manager is still open.
         *
         * @thread_safety This function can be called from any thread.
         */
        bool hasWindowOpened();

        /**
         * @brief Poll events for all windows.
         *
         * @param pollRate The poll rate, or `std::nullopt` if no sleep is needed.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw glfw_cpp::WrongThreadAccess The function is called not from the main thread.
         */
        void pollEvents(std::optional<std::chrono::milliseconds> pollRate = {});

        /**
         * @brief Wait for events for all windows.
         *
         * @param timeout The timeout, or `std::nullopt` if no timeout is needed.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw glfw_cpp::WrongThreadAccess The function is called not from the main thread.
         */
        void waitEvents(std::optional<std::chrono::milliseconds> timeout = {});

        /**
         * @brief Request to delete a window.
         *
         * @param handle The window handle.
         *
         * @thread_safety This function can be called from any thread.
         */
        void requestDeleteWindow(Window::Handle handle);

        /**
         * @brief Enqueue a window task to be processed in the main thread.
         *
         * @param handle The window handle.
         * @param task The task.
         *
         * @thread_safety This function can be called from any thread.
         *
         * While this function can be called from any thread, this function is meant to be called from the
         * thread a Window is running on. This function meant to be used for Window tasks that need to be
         * executed in the main thread.
         */
        void enqueueWindowTask(Window::Handle handle, Fun<void()>&& task);

        /**
         * @brief Enqueue a task to be processed in the main thread.
         *
         * @param task The task.
         *
         * @thread_safety This function can be called from any thread.
         *
         * This function can be used for any task that needs to be executed in the main thread.
         */
        void enqueueTask(Fun<void()>&& task);

        /**
         * @brief Get the thread id this manager is attached to.
         */
        std::thread::id attachedThreadId() const noexcept { return m_attachedThreadId; }

    private:
        struct GLFWwindowDeleter
        {
            void operator()(Window::Handle handle) const noexcept;
        };
        using UniqueGLFWwindow = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>;

        struct WindowTask
        {
            Window::Handle m_handle;
            Fun<void()>    m_task;
        };

        WindowManager(std::thread::id threadId) noexcept;

        // send event to interceptor, returns the value the interceptor returns. but if there is not
        // interceptor, the returned value will always be true.
        bool sendInterceptEvent(Window& window, Event& event) noexcept;

        void validateAccess() const;
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
