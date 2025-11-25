#ifndef IMGUI_HPP_4W9EF8DHU
#define IMGUI_HPP_4W9EF8DHU

#include "glfw_cpp/event.hpp"

#include <cassert>
#include <utility>

#if __has_include(<imgui_impl_glfw.h>)
#    include <imgui_impl_glfw.h>
#else
#    error "This file requires imgui_impl_glfw.h to be exists and visible."
#endif

struct GLFWwindow;

namespace glfw_cpp::imgui
{
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
        friend ImguiHandle init_for_opengl(GLFWwindow* window) noexcept;
        friend ImguiHandle init_for_vulkan(GLFWwindow* window) noexcept;
        friend ImguiHandle init_for_other(GLFWwindow* window) noexcept;

        ImguiHandle() noexcept = default;

        ~ImguiHandle() noexcept { shutdown(); }

        ImguiHandle(const ImguiHandle&)            = delete;
        ImguiHandle& operator=(const ImguiHandle&) = delete;

        ImguiHandle(ImguiHandle&& other) noexcept
            : m_window{ std::exchange(other.m_window, nullptr) }
        {
        }

        ImguiHandle& operator=(ImguiHandle&& other) noexcept
        {
            if (this != &other) {
                ImGui_ImplGlfw_Shutdown();
                m_window = std::exchange(other.m_window, nullptr);
            }

            return *this;
        }

        bool is_initialized() const noexcept { return m_window != nullptr; }

        void new_frame() const noexcept { ImGui_ImplGlfw_NewFrame(); }

        /**
         * @brief Shutdown the imgui context.
         *
         * This function will shutdown the imgui context if it was initialized.
         */
        void shutdown() noexcept
        {
            if (is_initialized()) {
                m_window = nullptr;
                ImGui_ImplGlfw_Shutdown();
            }
        }

        void process_events(const EventQueue& event)
        {
            using namespace event;

            // TODO: handle monitor event
            event.visit(Overload{
                // clang-format off
                [&](const WindowFocused& e) { ImGui_ImplGlfw_WindowFocusCallback(m_window, e.focused); },
                [&](const CursorEntered& e) { ImGui_ImplGlfw_CursorEnterCallback(m_window, e.entered); },
                [&](const CursorMoved&   e) { ImGui_ImplGlfw_CursorPosCallback  (m_window, e.x, e.y); },
                [&](const ButtonPressed& e) { ImGui_ImplGlfw_MouseButtonCallback(m_window, underlying(e.button), underlying(e.state), underlying(e.mods)); },
                [&](const Scrolled&      e) { ImGui_ImplGlfw_ScrollCallback     (m_window, e.dx, e.dy); },
                [&](const KeyPressed&    e) { ImGui_ImplGlfw_KeyCallback        (m_window, underlying(e.key), e.scancode, underlying(e.state), underlying(e.mods)); },
                [&](const CharInput&     e) { ImGui_ImplGlfw_CharCallback       (m_window, e.codepoint); },
                [&](const auto&           ) { /* do nothing */ },
                // clang-format on

            });
        }

    private:
        GLFWwindow* m_window = nullptr;

        ImguiHandle(GLFWwindow* window) noexcept
            : m_window{ window }
        {
        }
    };

    /**
     * @brief Initialize imgui for opengl while also registering the window to the interceptor.
     *
     * @param window The window to register.
     *
     * @return The imgui context for glfw (proper RAII).
     *
     * @note Make sure the interceptor is registered to the `WindowManager` the `Window` is in before calling
     * this function.
     */
    inline ImguiHandle init_for_opengl(GLFWwindow* window) noexcept
    {
        ImGui_ImplGlfw_InitForOpenGL(window, false);
        return { window };
    }

    /**
     * @brief Initialize imgui for vulkan while also registering the window to the interceptor.
     *
     * @param window The window to register.
     *
     * @return The imgui context for glfw (proper RAII).
     *
     * @note Make sure the interceptor is registered to the `WindowManager` the `Window` is in before calling
     * this function.
     */
    inline ImguiHandle init_for_vulkan(GLFWwindow* window) noexcept
    {
        ImGui_ImplGlfw_InitForVulkan(window, false);
        return { window };
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
    inline ImguiHandle init_for_other(GLFWwindow* window) noexcept
    {
        ImGui_ImplGlfw_InitForOther(window, false);
        return { window };
    }
}

#endif /* end of include guard: IMGUI_HPP_4W9EF8DHU */
