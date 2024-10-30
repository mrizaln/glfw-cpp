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
         * @brief Bind the window context to the current thread (only makes sense for OpenGL and OpenGLES).
         *
         * The function has an assertion that failed if the current context is already bound to other thread.
         */
        void bind();

        /**
         * @brief Unbind the window context from the current thread (only makes sense for OpenGL and
         * OpenGLES).
         */
        void unbind();

        // destroy the underlying window resource and reset the instance to default-initialized state.
        // basically doing `*this = {}`
        void destroy();

        void iconify();
        void restore();
        void maximize();
        void show();
        void hide();
        void focus();

        void setWindowSize(int width, int height);
        void setWindowPos(int x, int y);

        // lock the aspect ratio of the window, the ratio is width/height.
        // the current width will be preserved while the height will be adjusted accordingly.
        void lockAspectRatio(float ratio);
        void lockCurrentAspectRatio();
        void unlockAspectRatio();

        void updateTitle(const std::string& title);

        // check if the Window should close it's window
        bool shouldClose() const;

        // poll just returns events polled by WindowManager and process any queued tasks if exists
        const EventQueue& poll();

        // swap glfw window buffer, do nothing if Api::NoApi.
        // returns deltaTime/frameTime (the retuned time is the time taken between display() calls).
        double display();

        // use window (check shouldClose(), poll(), and display() at the same time).
        // returns deltaTime/frameTime if window is not closed else std::nullopt.
        // the deltaTime returned is the time taken between display() calls.
        // will bind() as long as the function runs and unbind() at the end.
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

        // like use() but it loops until shouldClose() returns true.
        // the Window will be bind() at the entirety of the loop.
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

        void enqueueTask(Fun<void()>&& func);
        void requestClose();

        // will do nothing when Api::NoApi is set as the instance API
        void setVsync(bool value);
        void setCaptureMouse(bool value);
        void resizeEventQueue(std::size_t newSize);

        // The function added will be called from the window thread.
        bool              isVsyncEnabled() const { return m_vsync; }
        bool              isMouseCaptured() const { return m_captureMouse; }
        const Properties& properties() const { return m_properties; }
        double            deltaTime() const;
        Handle            handle() const { return m_handle; }

        // may return a defaulted std::thread::id
        std::thread::id attachedThreadId() const { return m_attachedThreadId; };

    private:
        Window(
            std::shared_ptr<WindowManager> manager,
            Handle                         handle,
            Properties&&                   properties,
            bool                           bindImmediately
        ) noexcept;

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

        void pushEvent(Event&& event);
        void processQueuedTasks();
        void updateDeltaTime();

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
