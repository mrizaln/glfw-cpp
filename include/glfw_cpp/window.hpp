#ifndef WINDOW_HPP_IROQWEOX
#define WINDOW_HPP_IROQWEOX

#include "glfw_cpp/event.hpp"
#include "glfw_cpp/input.hpp"
#include "glfw_cpp/monitor.hpp"

#include <vector>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

struct GLFWwindow;

namespace glfw_cpp
{
    class WindowManager;

    /**
     * @class Window
     * @brief Wrapper class for `GLFWwindow`.
     *
     * This `Window` class is a RAII wrapper around `GLFWwindow`. It provides a more C++-like interface to the
     * `GLFWwindow` API. This class is mostly self-contained. It held a copy of the properties of the windows
     * so querying them is fast. All the operations to the `Window` should be thread-safe in the sense that it
     * can be called from any thread (unlike most operations in the GLFW C API which mostly require functions
     * to be called from the main thread). Because of this, each `Window` instance can be operated on each
     * separate thread.
     */
    class Window
    {
    public:
        friend WindowManager;

        struct Properties
        {
            std::string m_title;

            struct Position
            {
                int m_x;
                int m_y;
            } m_pos;

            struct Dimension
            {
                int m_width;
                int m_height;
            } m_dimension;

            struct FramebufferSize
            {
                int m_width;
                int m_height;
            } m_framebufferSize;

            struct CursorPos
            {
                double m_x;
                double m_y;
            } m_cursor;

            struct Atrribute
            {
                unsigned int m_iconified   : 1 = 0;
                unsigned int m_maximized   : 1 = 0;
                unsigned int m_focused     : 1 = 0;
                unsigned int m_visible     : 1 = 0;
                unsigned int m_hovered     : 1 = 0;
                unsigned int m_resizable   : 1 = 0;
                unsigned int m_floating    : 1 = 0;
                unsigned int m_autoIconify : 1 = 0;
                unsigned int m_focusOnShow : 1 = 0;
            } m_attribute;

            MouseButtonStateRecord m_mouseButtonState = {};
            KeyStateRecord         m_keyState         = {};
            Monitor                m_monitor          = {};
        };

        static constexpr std::size_t s_defaultEventQueueSize = 128;

        template <typename Sig>
        using Fun    = std::function<Sig>;    // use std::move_only_function in the future
        using Handle = GLFWwindow*;

        Window(Window&&) noexcept;
        Window& operator=(Window&&) noexcept;
        ~Window();

        Window()                        = default;
        Window(const Window&)           = delete;
        Window operator=(const Window&) = delete;

        /**
         * @brief Bind window context to current thread (only makes sense for OpenGL and OpenGLES).
         *
         * @throw glfw_cpp::NoWindowContext If the window doesn't have a context (e.g. Api::NoApi).
         * @throw glfw_cpp::AlreadyBound If the window context is already bound to other thread.
         */
        void bind();

        /**
         * @brief Unbind window context from current thread (only makes sense for OpenGL and OpenGLES).
         *
         * @throw glfw_cpp::NoWindowContext If the window doesn't have a context (e.g. Api::NoApi).
         */
        void unbind();

        /**
         * @brief Destroy the window and reset the instance to default-initialized state.
         *
         * Basically doing `*this = {}`.
         */
        void destroy() noexcept;

        /**
         * @brief Iconify the window.
         *
         * Corresponds to `glfwIconifyWindow`.
         */
        void iconify() noexcept;

        /**
         * @brief Restore the window.
         *
         * Corresponds to `glfwRestoreWindow`.
         */
        void restore() noexcept;

        /**
         * @brief Maximize the window.
         *
         * Corresponds to `glfwMaximizeWindow`.
         */
        void maximize() noexcept;

        /**
         * @brief Show the window if it's hidden.
         *
         * Corresponds to `glfwShowWindow`.
         */
        void show() noexcept;

        /**
         * @brief Hide the window if it's shown.
         *
         * Corresponds to `glfwHideWindow`.
         */
        void hide() noexcept;

        /**
         * @brief Focus the window.
         *
         * Corresponds to `glfwFocusWindow`.
         */
        void focus() noexcept;

        /**
         * @brief Set window vertical sync (vsync).
         *
         * @param value True to enable vsync, false to disable.
         */
        void setVsync(bool value);

        /**
         * @brief Set window size.
         *
         * @param width The new width of the window.
         * @param height The new height of the window.
         *
         * The window size are in screen coordinates.
         */
        void setWindowSize(int width, int height) noexcept;

        /**
         * @brief Set window position.
         *
         * @param x The new x position of the window.
         * @param y The new y position of the window.
         *
         * The window position are in screen coordinates.
         */
        void setWindowPos(int x, int y) noexcept;

        /**
         * @brief Get current window aspect ratio.
         *
         * You could have use `glfw_cpp::Window::Properties::Dimension` to get the width and height of the
         * window then calculate the aspect ratio yourself, or you could use this function.
         */
        float aspectRatio() const noexcept;

        /**
         * @brief Lock the aspect ratio of the window.
         *
         * @param ratio The aspect ratio to lock to.
         *
         * The ratio is width/height. The current width will be preserved while the height will be adjusted
         * accordingly.
         */
        void lockAspectRatio(float ratio) noexcept;

        /**
         * @brief Lock the aspect ratio of the window to the current aspect ratio.
         */
        void lockCurrentAspectRatio() noexcept;

        /**
         * @brief Unlock the aspect ratio of the window.
         */
        void unlockAspectRatio() noexcept;

        /**
         * @brief Set the window title.
         *
         * @param title The new title of the window.
         */
        void updateTitle(std::string_view title) noexcept;

        /**
         * @brief Check if the window should close.
         *
         * Corresponds to `glfwWindowShouldClose`.
         */
        bool shouldClose() const noexcept;

        /**
         * @brief Swap the front and back event queue, then return the front queue.
         *
         * Besides of swapping the event queue double buffer, this function also runs tasks queued with
         * `glfw_cpp::Window::enqueueTask`.
         */
        const EventQueue& poll() noexcept;

        /**
         * @brief See last events queued before call to `poll()`
         *
         * This function just returns the last events happened before call to `poll()` it does not update the
         * events, and if consecutive call to this function happen while there is no `poll()` in between,
         * there will be no change to the event queue.
         */
        const EventQueue& events() noexcept { return m_eventQueueFront; }

        /**
         * @brief Swap the framebuffer of the window.
         *
         * @throw glfw_cpp::NoWindowContext If the window doesn't have a context (e.g. Api::NoApi).
         * @throw glfw_cpp::PlatformError If platform-specific error occurs.
         *
         * @return The time taken between the last call to this function and the current call.
         *
         * This function corresponds to `glfwSwapBuffers`.
         */
        double display();

        /**
         * @brief Use window, check if the window should close, poll events, and display the window at the
         * same time.
         *
         * @param func The function to be called between polling events and displaying the window.
         * @return std::nullopt if the window should close, otherwise the time it takes between the last call
         * to `display()` (whether through this function or directly) and the current call.
         *
         * This function is a convenience function that combines the `shouldClose()`, `poll()`, and
         * `display()` functions. It will bind the window at the beginning of the function and unbind it at
         * the end.
         */
        std::optional<double> use(std::invocable<const EventQueue&> auto&& func)
        {
            if (shouldClose()) {
                return std::nullopt;
            }

            bind();

            const auto& events = poll();
            func(events);
            auto delta = display();

            unbind();

            return delta;
        }

        /**
         * @brief Run the window loop; each iteration will check if the window should close, poll events, and
         * display the window.
         *
         * @param func The function to be called between polling events and displaying the window.
         *
         * This function is a convenience function that runs the window loop. Your works should be done in the
         * `func` parameter. The loop will continue until the window should close. The window will be bound
         * at the beginning of the loop and unbound at the end.
         *
         * Basically, this function is an analogue to `use` but with a loop.
         */
        void run(std::invocable<const EventQueue&> auto&& func)
        {
            bind();

            while (!shouldClose()) {
                const auto& events = poll();
                func(events);
                display();
            }

            unbind();
        }

        /**
         * @brief Enqueue a task to be run in the window thread.
         *
         * @param func The function to be run in the window thread.
         *
         * The function will be ran at the time `poll()` is called.
         */
        void enqueueTask(Fun<void()>&& func) noexcept;

        /**
         * @brief Request the window to close.
         *
         * Corresponds to `glfwSetWindowShouldClose`.
         */
        void requestClose() noexcept;

        /**
         * @brief Set the mouse capture state.
         *
         * @param value True to capture the mouse, false to release it.
         */
        void setCaptureMouse(bool value) noexcept;

        /**
         * @brief Resize the event queue to the new size.
         *
         * @param newSize The new size of the event queue.
         *
         * Every EventQueue in each Window has a fized size. The default size is `s_defaultEventQueueSize`.
         */
        void resizeEventQueue(std::size_t newSize) noexcept;

        /**
         * @brief Get the properties of the window.
         *
         * @return The properties of the window.
         */
        const Properties& properties() const noexcept { return m_properties; }

        /**
         * @brief Get last frame time.
         */
        double deltaTime() const noexcept { return m_deltaTime; }

        /**
         * @brief Check whether the window vsync is enabled.
         */
        bool isVsyncEnabled() const noexcept { return m_vsync; }

        /**
         * @brief Check whether the mouse is captured.
         */
        bool isMouseCaptured() const noexcept { return m_captureMouse; }

        /**
         * @brief Get the underlying `GLFWwindow` handle.
         */
        Handle handle() const noexcept { return m_handle; }

        /**
         * @brief Get the thread id that the window is attached to.
         *
         * @return The thread id that the window is attached to. If the window is not attached to any thread,
         * it will return a default-constructed `std::thread::id`.
         */
        std::thread::id attachedThreadId() const noexcept { return m_attachedThreadId; };

    private:
        Window(
            std::shared_ptr<WindowManager> manager,
            Handle                         handle,
            Properties&&                   properties,
            bool                           bindImmediately
        );

        static void window_pos_callback(GLFWwindow* window, int x, int y);
        static void window_size_callback(GLFWwindow* window, int width, int height);
        static void window_close_callback(GLFWwindow* window);
        static void window_refresh_callback(GLFWwindow* window);
        static void window_focus_callback(GLFWwindow* window, int focused);
        static void window_iconify_callback(GLFWwindow* window, int iconified);
        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void cursor_pos_callback(GLFWwindow* window, double x, double y);
        static void cursor_enter_callback(GLFWwindow* window, int entered);
        static void scroll_callback(GLFWwindow* window, double x, double y);
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void char_callback(GLFWwindow* window, unsigned int codepoint);
        static void file_drop_callback(GLFWwindow* window, int count, const char** paths);
        static void window_maximize_callback(GLFWwindow* window, int maximized);
        static void window_content_scale_callback(GLFWwindow* window, float xscale, float yscale);

        // NOTE: These two callbacks are beyond glfw_cpp::Window control alone, maybe setting it in
        //       the glfw_cpp::Instance makes more sense. But then, there's the problem of how to
        //       'broadcast' the event to all opened windows...
        /*
            static void monitor_callback(GLFWmonitor* monitor, int action);
            static void joystick_callback(int jid, int action);
        */

        static void windowCallbackHelper(GLFWwindow* window, Event&& event) noexcept;

        void pushEvent(Event&& event) noexcept;
        void processQueuedTasks() noexcept;
        void updateDeltaTime() noexcept;

        std::shared_ptr<WindowManager> m_manager = nullptr;
        Handle                         m_handle  = nullptr;

        // window stuff
        std::thread::id m_attachedThreadId = {};
        Properties      m_properties       = {};
        double          m_lastFrameTime    = 0.0;
        double          m_deltaTime        = 0.0;
        bool            m_vsync            = true;
        bool            m_captureMouse     = false;

        // queues
        EventQueue               m_eventQueueFront{ s_defaultEventQueueSize };
        EventQueue               m_eventQueueBack{ s_defaultEventQueueSize };
        std::vector<Fun<void()>> m_taskQueue;
        mutable std::mutex       m_queueMutex;
    };
}

#endif /* end of include guard: WINDOW_HPP_IROQWEOX */
