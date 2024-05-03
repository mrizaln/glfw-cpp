#ifndef WINDOW_HPP_IROQWEOX
#define WINDOW_HPP_IROQWEOX

#include "glfw_cpp/event.hpp"

#include <deque>
#include <functional>
#include <mutex>
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
            int         m_width;
            int         m_height;

            struct
            {
                double m_x;
                double m_y;
            } m_cursorPos;
        };

        template <typename Sig>
        using Fun = std::function<Sig>;    // use std::move_only_function in the future

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

        // main rendering loop
        void run(Fun<void(std::deque<Event>&&)>&& func);

        void    enqueueTask(Fun<void()>&& func);
        void    requestClose();
        Window& setVsync(bool value);
        Window& setCaptureMouse(bool value);

        // The function added will be called from the window thread.
        bool              isVsyncEnabled() const { return m_vsync; }
        bool              isMouseCaptured() const { return m_captureMouse; }
        const Properties& properties() const { return m_properties; }
        double            deltaTime() const;
        GLFWwindow*       handle() const { return m_windowHandle; }
        std::size_t       id() const { return m_id; }

        // may return a defaulted std::thread::id
        std::thread::id attachedThreadId() const { return m_attachedThreadId; };

    private:
        Window() = default;

        Window(
            WindowManager& manager,
            std::size_t    id,
            GLFWwindow*    handle,
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

        // TODO; implement more callbacks

        void processQueuedTasks();
        void updateDeltaTime();

        void swap(Window& other) noexcept;

        WindowManager* m_manager;

        // window stuff
        std::size_t     m_id;
        std::thread::id m_attachedThreadId;
        GLFWwindow*     m_windowHandle;
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
