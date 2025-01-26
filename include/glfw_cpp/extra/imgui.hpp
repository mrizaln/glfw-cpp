#ifndef IMGUI_HPP_4W9EF8DHU
#define IMGUI_HPP_4W9EF8DHU

#include "glfw_cpp/event.hpp"
#include "glfw_cpp/window.hpp"

#include <unordered_set>

#if __has_include(<imgui.h>) and __has_include(<imgui_impl_glfw.h>)
#    include <imgui_impl_glfw.h>
#else
#    error "This file requires imgui.h and imgui_impl_glfw.h to be included."
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
    struct ImguiInterceptor : public DefaultEventInterceptor
    {
    public:
        /**
         * @brief Add a window to the list of windows that should be intercepted.
         *
         * @param window The window handle to the window to intercept.
         */
        void add_window(Window::Handle window) noexcept { m_windows.insert(window); }

        /**
         * @brief Remove a window from the list of windows that should be intercepted.
         *
         * @param window The window handle to the window to remove.
         */
        void remove_window(Window::Handle window) noexcept { m_windows.erase(window); }

        /**
         * @brief Check if a window is being intercepted.
         *
         * @param window The window handle to check.
         */
        bool is_intercepting(Window::Handle window) const noexcept { return m_windows.contains(window); }

        // overrides
        // ---------

        bool on_window_focused(Window& window, Event::WindowFocused& focus) noexcept override
        {
            if (m_windows.contains(window.handle())) {
                window.enqueue_task([focus](Window& win) {
                    ImGui_ImplGlfw_WindowFocusCallback(win.handle(), focus.m_focused ? 1 : 0);
                });
            }
            return true;
        }

        bool on_cursor_entered(Window& window, Event::CursorEntered& cursor) noexcept override
        {
            if (m_windows.contains(window.handle())) {
                window.enqueue_task([cursor](Window& win) {
                    ImGui_ImplGlfw_CursorEnterCallback(win.handle(), cursor.m_entered ? 1 : 0);
                });
            }
            return true;
        }

        bool on_cursor_moved(Window& window, Event::CursorMoved& cursor) noexcept override
        {
            if (m_windows.contains(window.handle())) {
                window.enqueue_task([cursor](Window& win) {
                    ImGui_ImplGlfw_CursorPosCallback(win.handle(), cursor.m_x, cursor.m_y);
                });
            }
            return true;
        }

        bool on_button_pressed(Window& window, Event::ButtonPressed& button) noexcept override
        {
            if (m_windows.contains(window.handle())) {
                window.enqueue_task([button](Window& win) {
                    auto btn    = static_cast<int>(button.m_button);
                    auto action = static_cast<int>(button.m_state);
                    auto mods   = static_cast<int>(button.m_mods);
                    ImGui_ImplGlfw_MouseButtonCallback(win.handle(), btn, action, mods);
                });
            }
            return true;
        }

        bool on_scrolled(Window& window, Event::Scrolled& scroll) noexcept override
        {
            if (m_windows.contains(window.handle())) {
                window.enqueue_task([scroll](Window& win) {
                    ImGui_ImplGlfw_ScrollCallback(win.handle(), scroll.m_dx, scroll.m_dy);
                });
            }
            return true;
        }

        bool on_key_pressed(Window& window, Event::KeyPressed& key) noexcept override
        {
            if (m_windows.contains(window.handle())) {
                window.enqueue_task([key](Window& win) {
                    auto k      = static_cast<int>(key.m_key);
                    auto scan   = key.m_scancode;
                    auto action = static_cast<int>(key.m_state);
                    auto mods   = static_cast<int>(key.m_mods);
                    ImGui_ImplGlfw_KeyCallback(win.handle(), k, scan, action, mods);
                });
            }
            return true;
        }

        bool on_char_input(Window& window, Event::CharInput& key) noexcept override
        {
            if (m_windows.contains(window.handle())) {
                window.enqueue_task([key](Window& win) {
                    ImGui_ImplGlfw_CharCallback(win.handle(), key.m_codepoint);
                });
            }
            return true;
        }

        // TODO: add on monitor callback

        // ---------

    private:
        // list of windows that should be intercepted
        std::unordered_set<Window::Handle> m_windows;
    };

    /**
     * @class ImguiContext
     * @brief RAII wrapper for ImGui context for GLFW.
     *
     * This class will automatically shutdown the ImGui context for GLFW when it goes out of scope. You can
     * also manually shutdown the context by calling the `shutdown()` function. At that point the context does
     * not represent a valid ImGui context anymore.
     */
    class [[nodiscard]] ImguiContext
    {
    public:
        friend ImguiContext init_imgui_for_opengl(
            ImguiInterceptor& interceptor,
            Window::Handle    window
        ) noexcept;

        friend ImguiContext init_imgui_for_vulkan(
            ImguiInterceptor& interceptor,
            Window::Handle    window
        ) noexcept;

        friend ImguiContext init_imgui_for_other(
            ImguiInterceptor& interceptor,
            Window::Handle    window
        ) noexcept;

        /**
         * @brief Default constructor.
         *
         * A default constructed `ImguiContext` is as good as an uninitialized one.
         */
        ImguiContext() noexcept = default;

        ~ImguiContext() noexcept { shutdown(); }

        ImguiContext(const ImguiContext&)            = delete;
        ImguiContext& operator=(const ImguiContext&) = delete;

        ImguiContext(ImguiContext&& other) noexcept
            : m_interceptor{ std::exchange(other.m_interceptor, nullptr) }
            , m_window{ std::exchange(other.m_window, nullptr) }
        {
        }

        ImguiContext& operator=(ImguiContext&& other) noexcept
        {
            if (this == &other) {
                return *this;
            }

            shutdown();

            m_interceptor = std::exchange(other.m_interceptor, nullptr);
            m_window      = std::exchange(other.m_window, nullptr);

            return *this;
        }

        bool is_initialized() const noexcept { return m_interceptor != nullptr and m_window != nullptr; }

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
        enum class Api : std::uint8_t
        {
            OpenGL,
            Vulkan,
            Other,
        };

        ImguiInterceptor* m_interceptor = nullptr;
        Window::Handle    m_window      = nullptr;

        ImguiContext(Api api, ImguiInterceptor* interceptor, Window::Handle window) noexcept
            : m_interceptor{ interceptor }
            , m_window{ window }
        {
            switch (api) {
            case Api::OpenGL: ImGui_ImplGlfw_InitForOpenGL(window, false); break;
            case Api::Vulkan: ImGui_ImplGlfw_InitForVulkan(window, false); break;
            case Api::Other: ImGui_ImplGlfw_InitForOther(window, false); break;
            }
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
    ImguiContext init_imgui_for_opengl(ImguiInterceptor& interceptor, Window::Handle window) noexcept
    {
        interceptor.add_window(window);
        return { ImguiContext::Api::OpenGL, &interceptor, window };
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
    ImguiContext init_imgui_for_vulkan(ImguiInterceptor& interceptor, Window::Handle window) noexcept
    {
        interceptor.add_window(window);
        return { ImguiContext::Api::Vulkan, &interceptor, window };
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
    ImguiContext init_imgui_for_other(ImguiInterceptor& interceptor, Window::Handle window) noexcept
    {
        interceptor.add_window(window);
        return { ImguiContext::Api::Other, &interceptor, window };
    }
}

#endif /* end of include guard: IMGUI_HPP_4W9EF8DHU */
