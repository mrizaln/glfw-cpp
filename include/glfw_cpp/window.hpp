#ifndef WINDOW_HPP_IROQWEOX
#define WINDOW_HPP_IROQWEOX

#include "glfw_cpp/event.hpp"
#include "glfw_cpp/input.hpp"

#include <deque>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

struct GLFWwindow;

namespace glfw_cpp
{
    class WindowManager;

    class Window
    {
    public:
        friend WindowManager;

        struct Properties
        {
            std::string m_title;

            struct
            {
                int m_x;
                int m_y;
            } m_pos;

            struct
            {
                int m_width;
                int m_height;
            } m_dimension;

            struct
            {
                double m_x;
                double m_y;
                bool   m_inside;
            } m_cursor;

            KeyStateRecord         m_keyState;
            MouseButtonStateRecord m_mouseButtonState;
        };

        template <typename Sig>
        using Fun    = std::function<Sig>;    // use std::move_only_function in the future
        using Handle = GLFWwindow*;

        Window(Window&&) noexcept;
        Window& operator=(Window&&) noexcept;
        ~Window();

        Window(const Window&)           = delete;
        Window operator=(const Window&) = delete;

        // use the context on current thread;
        void bind();
        void unbind();

        void setWindowSize(int width, int height);
        void updateTitle(const std::string& title);

        // check if the Window should close it's window
        bool shouldClose() const;

        // poll just returns events polled by WindowManager and process any queued tasks if exists
        std::deque<Event> poll();

        // swap glfw window buffer, do nothing if Api::NoApi.
        // returns deltaTime/frameTime (the retuned time is the time taken between display() calls).
        double display();

        // use window (check shouldClose(), poll(), and display() at the same time).
        // returns deltaTime/frameTime if window is not closed else std::nullopt.
        // the deltaTime returned is the time taken between display() calls.
        // will bind() as long as the function runs and unbind() at the end.
        std::optional<double> use(std::invocable<std::deque<Event>&&> auto&& func)
        {
            if (shouldClose()) {
                return std::nullopt;
            }

            bind();

            auto events = poll();
            func(std::move(events));
            auto delta = display();

            unbind();

            return delta;
        }

        // like use() but it loops until shouldClose() returns true.
        // the Window will be bind() at the entirety of the loop.
        void run(std::invocable<std::deque<Event>&&> auto&& func)
        {
            bind();

            while (!shouldClose()) {
                auto events = poll();
                func(std::move(events));
                display();
            }

            unbind();
        }

        void    enqueueTask(Fun<void()>&& func);
        void    requestClose();
        Window& setVsync(bool value);
        Window& setCaptureMouse(bool value);

        // The function added will be called from the window thread.
        bool              isVsyncEnabled() const { return m_vsync; }
        bool              isMouseCaptured() const { return m_captureMouse; }
        const Properties& properties() const { return m_properties; }
        double            deltaTime() const;
        Handle            handle() const { return m_handle; }

        // may return a defaulted std::thread::id
        std::thread::id attachedThreadId() const { return m_attachedThreadId; };

    private:
        Window() = default;

        Window(
            WindowManager& manager,
            Handle         handle,
            Properties&&   properties,
            bool           bindImmediately
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

        void pushEvent(Event&& event);
        void processQueuedTasks();
        void updateDeltaTime();

        void swap(Window& other) noexcept;

        WindowManager* m_manager;
        Handle         m_handle;

        // window stuff
        std::thread::id m_attachedThreadId;
        Properties      m_properties;
        double          m_lastFrameTime = 0.0;
        double          m_deltaTime     = 0.0;
        bool            m_vsync         = true;
        bool            m_captureMouse  = false;

        // queues
        std::deque<Fun<void()>> m_taskQueue;
        std::deque<Event>       m_eventQueue;
        mutable std::mutex      m_queueMutex;
    };
}

#endif /* end of include guard: WINDOW_HPP_IROQWEOX */
