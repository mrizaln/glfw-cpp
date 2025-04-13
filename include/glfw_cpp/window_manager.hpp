#ifndef WINDOW_MANAGER_HPP_OR5VIUQW
#define WINDOW_MANAGER_HPP_OR5VIUQW

#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/window.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

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
    class Monitor;
    class IEventInterceptor;

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
            Resizable              = 1 << 0,
            Visible                = 1 << 1,
            Decorated              = 1 << 2,
            Focused                = 1 << 3,
            AutoIconify            = 1 << 4,
            Floating               = 1 << 5,
            Maximized              = 1 << 6,
            CenterCursor           = 1 << 7,
            TransparentFramebuffer = 1 << 8,
            FocusOnShow            = 1 << 9,
            ScaleToMonitor         = 1 << 10,

            Default = Resizable | Visible | Decorated | Focused | AutoIconify | FocusOnShow,
        };

        Monitor* monitor = nullptr;
        Window*  share   = nullptr;

        Flag flags = FlagBit::Default;

        int red_bits     = 8;
        int green_bits   = 8;
        int blue_bits    = 8;
        int alpha_bits   = 8;
        int depth_bits   = 24;
        int stencil_bits = 8;

        int samples      = 0;
        int refresh_rate = -1;
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
         * @param bind_immediately Whether to bind the window immediately to current thread.
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
        Window create_window(
            const WindowHint& hint,
            std::string_view  title,
            int               width,
            int               height,
            bool              bind_immediately = true
        );

        /**
         * @brief Set an event interceptor.
         *
         * @param event_interceptor The event interceptor (nullptr to remove).
         *
         * @return The old event interceptor.
         *
         * The interceptor should have a longer lifetime than this `WindowManager` and the memory management
         * of it is the responsibility of the caller.
         */
        IEventInterceptor* set_event_interceptor(IEventInterceptor* event_interceptor) noexcept
        {
            return std::exchange(m_event_interceptor, event_interceptor);
        }

        /**
         * @brief Check if any window managed by this manager is still open.
         *
         * @thread_safety This function can be called from any thread.
         */
        bool has_window_opened();

        /**
         * @brief Poll events for all windows.
         *
         * @param poll_rate The poll rate, or `std::nullopt` if no sleep is needed.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw glfw_cpp::WrongThreadAccess The function is called not from the main thread.
         */
        void poll_events(std::optional<std::chrono::milliseconds> poll_rate = {});

        /**
         * @brief Wait for events for all windows.
         *
         * @param timeout The timeout, or `std::nullopt` if no timeout is needed.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw glfw_cpp::WrongThreadAccess The function is called not from the main thread.
         */
        void wait_events(std::optional<std::chrono::milliseconds> timeout = {});

        /**
         * @brief Request to delete a window.
         *
         * @param handle The window handle.
         *
         * @thread_safety This function can be called from any thread.
         */
        void request_delete_window(Window::Handle handle);

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
        void enqueue_window_task(Window::Handle handle, Fun<void()>&& task);

        /**
         * @brief Enqueue a task to be processed in the main thread.
         *
         * @param task The task.
         *
         * @thread_safety This function can be called from any thread.
         *
         * This function can be used for any task that needs to be executed in the main thread.
         */
        void enqueue_task(Fun<void()>&& task);

        /**
         * @brief Get the thread id this manager is attached to.
         */
        std::thread::id attached_thread_id() const noexcept { return m_attached_thread_id; }

    private:
        struct GLFWwindowDeleter
        {
            void operator()(Window::Handle handle) const noexcept;
        };
        using UniqueGLFWwindow = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>;

        struct WindowTask
        {
            Window::Handle handle;
            Fun<void()>    task;
        };

        WindowManager(std::thread::id thread_id, IEventInterceptor* event_interceptor) noexcept;

        // send event to interceptor, returns the value the interceptor returns. but if there is not
        // interceptor, the returned value will always be true.
        bool send_intercept_event(Window& window, Event& event) noexcept;

        void validate_access() const;
        void check_tasks();

        mutable std::mutex m_mutex;    // protects current instance
        std::thread::id    m_attached_thread_id;

        std::vector<UniqueGLFWwindow> m_windows;
        std::vector<Window::Handle>   m_window_delete_queue;
        std::vector<WindowTask>       m_window_task_queue;    // window task (checked)
        std::vector<Fun<void()>>      m_task_queue;           // general task

        IEventInterceptor* m_event_interceptor = nullptr;
    };
}

#endif /* end of include guard: WINDOW_MANAGER_HPP_OR5VIUQW */
