#ifndef WINDOW_HPP_IROQWEOX
#define WINDOW_HPP_IROQWEOX

#include "glfw_cpp/event.hpp"
#include "glfw_cpp/input.hpp"
#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/monitor.hpp"

#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

struct GLFWwindow;

namespace glfw_cpp
{
    class Instance;

    struct Dimensions
    {
        int  width;
        int  height;
        auto operator<=>(const Dimensions&) const = default;
    };

    struct FramebufferSize
    {
        int  width;
        int  height;
        auto operator<=>(const FramebufferSize&) const = default;
    };

    struct CursorPosition
    {
        double x;
        double y;
        auto   operator<=>(const CursorPosition&) const = default;
    };

    // TODO: context related attributes and framebuffer related attributes
    struct Attributes
    {
        unsigned int focused                 : 1 = 0;
        unsigned int iconified               : 1 = 0;
        unsigned int maximized               : 1 = 0;
        unsigned int hovered                 : 1 = 0;
        unsigned int visible                 : 1 = 0;
        unsigned int resizable               : 1 = 0;
        unsigned int decorated               : 1 = 0;    // TODO: implement setter and updates
        unsigned int auto_iconify            : 1 = 0;
        unsigned int floating                : 1 = 0;
        unsigned int transparent_framebuffer : 1 = 0;    // TODO: implement setter and updates
        unsigned int focus_on_show           : 1 = 0;
        unsigned int mouse_passthrough       : 1 = 0;    // TODO: implement setter and updates

        bool operator==(const Attributes&) const = default;
    };

    struct Properties
    {
        std::string            title;
        Position               position;
        Dimensions             dimensions;
        FramebufferSize        framebuffer_size;
        CursorPosition         cursor_position;
        MouseButtonStateRecord mouse_button_state;
        KeyStateRecord         key_state;
        Monitor                monitor;
    };

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
        friend Instance;

        static constexpr std::size_t s_default_eventqueue_size = 128;

        template <typename Sig>
        using Fun    = std::function<Sig>;    // use std::move_only_function in the future
        using Handle = GLFWwindow*;

        Window(Window&&) noexcept;
        Window& operator=(Window&&) noexcept;
        ~Window();

        Window()                        = default;
        Window(const Window&)           = delete;
        Window operator=(const Window&) = delete;

        explicit operator bool() const noexcept { return m_handle != nullptr; }

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
         *
         * @throw glfw_cpp::NoWindowContext If the window doesn't have a context (i.e. Api::NoApi).
         *
         * Corresponds to `glfwSwapInterval(1)` on true or `glfwSwapInterval(0)` on false.
         *
         * If the window context is not current at the point of calling, this function will bind it
         * temporarily before restoring it to previously bound context.
         */
        void set_vsync(bool value);

        /**
         * @brief Toggle window vertical sync (vsync).
         *
         * @throw glfw_cpp::NoWindowContext If the window doesn't have a context (i.e. Api::NoApi).
         *
         * Corresponds to `glfwSwapInterval`.
         */
        void toggle_vsync() { set_vsync(!is_vsync_enabled()); }

        /**
         * @brief Set whether the window should be resizable by the user.
         *
         * @param value True to make the window resizable, false to disable.
         *
         * Corresponds to setting `GLFW_RESIZABLE` attribute.
         */
        void set_resizable(bool value);

        /**
         * @brief Set whether the window should be floating or not.
         *
         * @param value True to make it floating, false to disable.
         *
         * Also known as always-on-top.
         * Corresponds to setting `GLFW_FLOATING` attribute.
         */
        void set_floating(bool value);

        /**
         * @brief Set whether the window on fullscreen should iconify on focus loss, a close widget, etc.
         *
         * @param value True to make it auto iconify, false to disable.
         *
         * Corresponds to setting `GLFW_AUTO_ICONIFY` attribute.
         */
        void set_auto_iconify(bool value);

        /**
         * @brief Set whether the window will be given input focus when window is shown from hidden state.
         *
         * @param value True to make it auto focus, false to disable.
         *
         * Corresponds to setting `GLFW_FOCUS_ON_SHOW` attribute.
         */
        void set_focus_on_show(bool value);

        /**
         * @brief Set window size.
         *
         * @param width The new width of the window.
         * @param height The new height of the window.
         *
         * The window size are in screen coordinates.
         */
        void set_window_size(int width, int height) noexcept;

        /**
         * @brief Set window position.
         *
         * @param x The new x position of the window.
         * @param y The new y position of the window.
         *
         * The window position are in screen coordinates.
         */
        void set_window_pos(int x, int y) noexcept;

        /**
         * @brief Get current window aspect ratio.
         *
         * You could have use `glfw_cpp::Window::Properties::Dimension` to get the width and height of the
         * window then calculate the aspect ratio yourself, or you could use this function.
         */
        float aspect_ratio() const noexcept;

        /**
         * @brief Lock the aspect ratio of the window.
         *
         * @param ratio The aspect ratio to lock to.
         *
         * The ratio is width/height. The current width will be preserved while the height will be adjusted
         * accordingly.
         */
        void lock_aspect_ratio(float ratio) noexcept;

        /**
         * @brief Lock the aspect ratio of the window to the current aspect ratio.
         */
        void lock_current_aspect_ratio() noexcept;

        /**
         * @brief Unlock the aspect ratio of the window.
         */
        void unlock_aspect_ratio() noexcept;

        /**
         * @brief Set the window title.
         *
         * @param title The new title of the window.
         */
        void update_title(std::string_view title) noexcept;

        /**
         * @brief Check if the window should close.
         *
         * Corresponds to `glfwWindowShouldClose`.
         */
        bool should_close() const noexcept;

        /**
         * @brief Swap the front and back event queue, then return the front queue.
         *
         * Besides of swapping the event queue double buffer, this function also runs tasks queued with
         * `glfw_cpp::Window::enqueue_task`.
         */
        const EventQueue& poll() noexcept;

        /**
         * @brief See last events queued before call to `poll()`
         *
         * This function just returns the last events happened before call to `poll()` it does not update the
         * events, and if consecutive call to this function happen while there is no `poll()` in between,
         * there will be no change to the event queue.
         */
        const EventQueue& events() noexcept { return m_event_queue_front; }

        /**
         * @brief Swap the framebuffer of the window.
         *
         * @throw glfw_cpp::NoWindowContext If the window doesn't have a context (e.g. Api::NoApi).
         * @throw glfw_cpp::PlatformError If platform-specific error occurs.
         *
         * @return The time taken between the last call to this function and the current call.
         *
         * This function corresponds to `glfwSwapBuffers`. This function also updates the delta time recorded
         * on the window instance (delta time defined as interval between calls to `display()`).
         *
         * If you are using emscripten, this function will not block the execution regardless the vsync
         * option. But instead it will return immediately since `glfwSwapBuffers` is not implemented for wasm
         * platform. In fact, there is no need to do buffer swapping since the browser will do it
         * automatically. Instead, you need to manage the update frequency yourself. Calling this function is
         * still required though to update the delta time stored in this window instance.
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
         * This function is a convenience function that combines the `should_close()`, `poll()`, and
         * `display()` functions. It will bind the window at the beginning of the function and unbind it at
         * the end.
         */
        std::optional<double> use(std::invocable<const EventQueue&> auto&& func)
        {
            if (should_close()) {
                return std::nullopt;
            }

            auto prev = glfw_cpp::get_current();
            glfw_cpp::make_current(handle());

            const auto& events = poll();
            func(events);
            auto delta = display();

            glfw_cpp::make_current(prev);
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
         *
         * Warning! this function will loop as fast as possible in emscripten, rendering the browser unusable.
         * Read more on `display()`.
         */
        void run(std::invocable<const EventQueue&> auto&& func)
        {
            auto prev = glfw_cpp::get_current();
            glfw_cpp::make_current(handle());

            while (!should_close()) {
                const auto& events = poll();
                func(events);
                display();
            }

            glfw_cpp::make_current(prev);
        }

        /**
         * @brief Enqueue a task to be run in the window thread.
         *
         * @param func The function to be run in the window thread.
         *
         * The function will be ran at the time `poll()` is called.
         */
        void enqueue_task(Fun<void(Window&)>&& func) noexcept;

        /**
         * @brief Request the window to close.
         *
         * Corresponds to `glfwSetWindowShouldClose`.
         */
        void request_close() noexcept;

        /**
         * @brief Set the mouse capture state.
         *
         * @param value True to capture the mouse, false to release it.
         */
        void set_capture_mouse(bool value) noexcept;

        /**
         * @brief Toggle the mouse capture state.
         */
        void toggle_capture_mouse() noexcept { set_capture_mouse(!is_mouse_captured()); }

        /**
         * @brief Resize the event queue to the new size.
         *
         * @param new_size The new size of the event queue.
         *
         * Every EventQueue in each Window has a fized size. The default size is `s_default_eventqueue_size`.
         */
        void resize_event_queue(std::size_t new_size) noexcept;

        /**
         * @brief Get the properties of the window.
         *
         * @return The properties of the window.
         */
        const Properties& properties() const noexcept { return m_properties; }

        /**
         * @brief Get the attributes of the window.
         *
         * @return The attributes of the window.
         */
        const Attributes& attributes() const noexcept { return m_attributes; }

        /**
         * @brief Get last frame time.
         */
        double delta_time() const noexcept { return m_delta_time; }

        /**
         * @brief Check whether the window vsync is enabled.
         */
        bool is_vsync_enabled() const noexcept { return m_vsync; }

        /**
         * @brief Check whether the mouse is captured.
         */
        bool is_mouse_captured() const noexcept { return m_capture_mouse; }

        /**
         * @brief Get the underlying `GLFWwindow` handle.
         */
        Handle handle() const noexcept { return m_handle; }

    private:
        Window(Handle handle, Properties&& properties, Attributes&& attributes);

        void push_event(Event&& event) noexcept;
        void process_queued_tasks() noexcept;
        void update_delta_time() noexcept;

        Handle m_handle = nullptr;

        // window stuff
        Properties m_properties      = {};
        Attributes m_attributes      = {};
        double     m_last_frame_time = 0.0;
        double     m_delta_time      = 0.0;
        bool       m_vsync           = true;
        bool       m_capture_mouse   = false;
        bool       m_has_context     = false;

        // queues
        EventQueue                      m_event_queue_front = EventQueue{ s_default_eventqueue_size };
        EventQueue                      m_event_queue_back  = EventQueue{ s_default_eventqueue_size };
        std::vector<Fun<void(Window&)>> m_task_queue;
        mutable std::mutex              m_queue_mutex;
    };
}

#endif /* end of include guard: WINDOW_HPP_IROQWEOX */
