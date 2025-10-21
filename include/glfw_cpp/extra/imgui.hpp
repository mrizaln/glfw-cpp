#ifndef IMGUI_HPP_4W9EF8DHU
#define IMGUI_HPP_4W9EF8DHU

#include "glfw_cpp/event.hpp"
#include "glfw_cpp/window.hpp"

#include <cassert>
#include <shared_mutex>
#include <unordered_set>
#include <utility>

#if __has_include(<imgui_impl_glfw.h>)
#    include <imgui_impl_glfw.h>
#else
#    error "This file requires imgui_impl_glfw.h to be exists and visible."
#endif

namespace glfw_cpp::extra
{
    /**
     * @class ImguiInterceptor
     * @brief Interceptor for GLFW events for ImGui.
     *
     * The problem with the default ImGui impl for GLFW is that its event handling is global. The events are
     * handled in the main thread (where you call `glfw_cpp::WindowManager::poll_events()` or
     * `glfwPollEvents()`). This means that if you have the window ran in a different thread than the main
     * thread, there will be data races.
     *
     * This class is an interceptor that will intercept GLFW events and enqueue them to be handled on the
     * `Window`'s thread. This way you can have the window in a different thread and still have ImGui working
     * properly without any data races. The intercepted events then forwarded to the usual `glfw_cpp` event
     * handling mechanism without any modification to the event.
     */
    class ImguiInterceptor : public DefaultEventInterceptor
    {
    public:
        /**
         * @brief Add a window to the list of windows that should be intercepted.
         *
         * @param window The window handle to the window to intercept.
         */
        void add_window(Window::Handle window) noexcept
        {
            auto lock = std::unique_lock{ m_mutex };
            m_windows.insert(window);
        }

        /**
         * @brief Remove a window from the list of windows that should be intercepted.
         *
         * @param window The window handle to the window to remove.
         */
        void remove_window(Window::Handle window) noexcept
        {
            auto lock = std::unique_lock{ m_mutex };
            m_windows.erase(window);
        }

        /**
         * @brief Check if a window is being intercepted.
         *
         * @param window The window handle to check.
         */
        bool is_intercepting(Window::Handle window) const noexcept
        {
            auto lock = std::shared_lock{ m_mutex };
            return m_windows.contains(window);
        }

        // overrides
        // ---------

        bool on_window_focused(Window& window, event::WindowFocused& focus) noexcept override
        {
            if (auto lock = std::shared_lock{ m_mutex }; m_windows.contains(window.handle())) {
                window.enqueue_task([focus](Window& win) {
                    ImGui_ImplGlfw_WindowFocusCallback(win.handle(), focus.focused ? 1 : 0);
                });
            }
            return true;
        }

        bool on_cursor_entered(Window& window, event::CursorEntered& cursor) noexcept override
        {
            if (auto lock = std::shared_lock{ m_mutex }; m_windows.contains(window.handle())) {
                window.enqueue_task([cursor](Window& win) {
                    ImGui_ImplGlfw_CursorEnterCallback(win.handle(), cursor.entered ? 1 : 0);
                });
            }
            return true;
        }

        bool on_cursor_moved(Window& window, event::CursorMoved& cursor) noexcept override
        {
            if (auto lock = std::shared_lock{ m_mutex }; m_windows.contains(window.handle())) {
                window.enqueue_task([cursor](Window& win) {
                    ImGui_ImplGlfw_CursorPosCallback(win.handle(), cursor.x, cursor.y);
                });
            }
            return true;
        }

        bool on_button_pressed(Window& window, event::ButtonPressed& button) noexcept override
        {
            if (auto lock = std::shared_lock{ m_mutex }; m_windows.contains(window.handle())) {
                window.enqueue_task([button](Window& win) {
                    auto btn    = static_cast<int>(button.button);
                    auto action = static_cast<int>(button.state);
                    auto mods   = static_cast<int>(button.mods);
                    ImGui_ImplGlfw_MouseButtonCallback(win.handle(), btn, action, mods);
                });
            }
            return true;
        }

        bool on_scrolled(Window& window, event::Scrolled& scroll) noexcept override
        {
            if (auto lock = std::shared_lock{ m_mutex }; m_windows.contains(window.handle())) {
                window.enqueue_task([scroll](Window& win) {
                    ImGui_ImplGlfw_ScrollCallback(win.handle(), scroll.dx, scroll.dy);
                });
            }
            return true;
        }

        bool on_key_pressed(Window& window, event::KeyPressed& key) noexcept override
        {
            if (auto lock = std::shared_lock{ m_mutex }; m_windows.contains(window.handle())) {
                window.enqueue_task([key](Window& win) {
                    auto k      = static_cast<int>(key.key);
                    auto scan   = key.scancode;
                    auto action = static_cast<int>(key.state);
                    auto mods   = static_cast<int>(key.mods);
                    ImGui_ImplGlfw_KeyCallback(win.handle(), k, scan, action, mods);
                });
            }
            return true;
        }

        bool on_char_input(Window& window, event::CharInput& key) noexcept override
        {
            if (auto lock = std::shared_lock{ m_mutex }; m_windows.contains(window.handle())) {
                window.enqueue_task([key](Window& win) {
                    ImGui_ImplGlfw_CharCallback(win.handle(), key.codepoint);
                });
            }
            return true;
        }

        // TODO: add on monitor callback

        // ---------

    private:
        // list of windows that should be intercepted
        std::unordered_set<Window::Handle> m_windows;
        mutable std::shared_mutex          m_mutex;
    };

    /**
     * @class ImguiContext
     * @brief RAII wrapper for ImGui context for GLFW.
     *
     * This class will automatically shutdown the ImGui context for GLFW when it goes out of scope. You can
     * also manually shutdown the context by calling the `shutdown()` function. At that point the context does
     * not represent a valid ImGui context anymore.
     */
    class [[nodiscard]] ImguiHandle
    {
    public:
        friend ImguiHandle init_imgui_for_opengl(
            ImguiInterceptor& interceptor,
            Window::Handle    window
        ) noexcept;

        friend ImguiHandle init_imgui_for_vulkan(
            ImguiInterceptor& interceptor,
            Window::Handle    window
        ) noexcept;

        friend ImguiHandle init_imgui_for_other(
            ImguiInterceptor& interceptor,
            Window::Handle    window
        ) noexcept;

        ImguiHandle() noexcept = default;

        ~ImguiHandle() noexcept { shutdown(); }

        ImguiHandle(const ImguiHandle&)            = delete;
        ImguiHandle& operator=(const ImguiHandle&) = delete;

        ImguiHandle(ImguiHandle&& other) noexcept
            : m_interceptor{ std::exchange(other.m_interceptor, nullptr) }
            , m_window{ std::exchange(other.m_window, nullptr) }
        {
        }

        ImguiHandle& operator=(ImguiHandle&& other) noexcept
        {
            if (this != &other) {
                shutdown();
                m_interceptor = std::exchange(other.m_interceptor, nullptr);
                m_window      = std::exchange(other.m_window, nullptr);
            }

            return *this;
        }

        bool is_initialized() const noexcept { return m_interceptor != nullptr and m_window != nullptr; }

        void new_frame() const noexcept { ImGui_ImplGlfw_NewFrame(); }

        /**
         * @brief Shutdown the imgui context.
         *
         * This function will shutdown the imgui context if it was initialized.
         */
        void shutdown() noexcept
        {
            if (is_initialized()) {
                ImGui_ImplGlfw_Shutdown();
                m_interceptor->remove_window(m_window);
                m_interceptor = nullptr;
                m_window      = nullptr;
            }
        }

    private:
        ImguiInterceptor* m_interceptor = nullptr;
        Window::Handle    m_window      = nullptr;

        ImguiHandle(ImguiInterceptor* interceptor, Window::Handle window) noexcept
            : m_interceptor{ interceptor }
            , m_window{ window }
        {
        }
    };

    /**
     * @brief Initialize imgui for opengl while also registering the window to the interceptor.
     *
     * @param interceptor The interceptor to use.
     * @param window The window to register.
     *
     * @return The imgui context for glfw (proper RAII).
     *
     * @note Make sure the interceptor is registered to the `WindowManager` the `Window` is in before calling
     * this function.
     */
    inline ImguiHandle init_imgui_for_opengl(ImguiInterceptor& interceptor, Window::Handle window) noexcept
    {
        interceptor.add_window(window);
        ImGui_ImplGlfw_InitForOpenGL(window, false);
        return { &interceptor, window };
    }

    /**
     * @brief Initialize imgui for vulkan while also registering the window to the interceptor.
     *
     * @param interceptor The interceptor to use.
     * @param window The window to register.
     *
     * @return The imgui context for glfw (proper RAII).
     *
     * @note Make sure the interceptor is registered to the `WindowManager` the `Window` is in before calling
     * this function.
     */
    inline ImguiHandle init_imgui_for_vulkan(ImguiInterceptor& interceptor, Window::Handle window) noexcept
    {
        interceptor.add_window(window);
        ImGui_ImplGlfw_InitForVulkan(window, false);
        return { &interceptor, window };
    }

    /**
     * @brief Initialize imgui for other while also registering the window to the interceptor.
     *
     * @param interceptor The interceptor to use.
     * @param window The window to register.
     *
     * @return The imgui context for glfw (proper RAII).
     *
     * @note Make sure the interceptor is registered to the `WindowManager` the `Window` is in before calling
     * this function.
     */
    inline ImguiHandle init_imgui_for_other(ImguiInterceptor& interceptor, Window::Handle window) noexcept
    {
        interceptor.add_window(window);
        ImGui_ImplGlfw_InitForOther(window, false);
        return { &interceptor, window };
    }
}

#endif /* end of include guard: IMGUI_HPP_4W9EF8DHU */
