#ifndef GLFW_CPP_IMGUI_HPP
#define GLFW_CPP_IMGUI_HPP

#include "glfw_cpp/event.hpp"

#include <cassert>
#include <utility>

#if __has_include(<imgui_impl_glfw.h>)
    #include <imgui_impl_glfw.h>
#else
    #error "This file requires imgui_impl_glfw.h to be exists and visible."
#endif

struct GLFWwindow;

namespace glfw_cpp::imgui
{
    /**
     * @brief Initialize imgui for opengl without installing a callback.
     *
     * @param window The window imgui will belong to.
     * @return An imgui handle for glfw (proper RAII).
     */
    inline void init_for_opengl(GLFWwindow* window) noexcept
    {
        ImGui_ImplGlfw_InitForOpenGL(window, false);
    }

    /**
     * @brief Initialize imgui for vulkan without installing a callback.
     *
     * @param window The window imgui will belong to.
     * @return An imgui handle for glfw (proper RAII).
     */
    inline void init_for_vulkan(GLFWwindow* window) noexcept
    {
        ImGui_ImplGlfw_InitForVulkan(window, false);
    }

    /**
     * @brief Initialize imgui for other without installing a callback.
     *
     * @param window The window imgui will belong to.
     * @return An imgui handle for glfw (proper RAII).
     */
    inline void init_for_other(GLFWwindow* window) noexcept
    {
        ImGui_ImplGlfw_InitForOther(window, false);
    }

    /**
     * @brief Process events from window.
     *
     * @param event The event queue from window.
     *
     * Since glfw_cpp already installs all the callback to GLFW internals and accumulate it into a queue
     * for each window, we need to send the events to imgui manually.
     */
    inline void process_events(GLFWwindow* window, const EventQueue& event)
    {
        using namespace event;

        // TODO: handle monitor event
        event.visit(Overload{
            // clang-format off
            [&](const WindowFocused& e) { ImGui_ImplGlfw_WindowFocusCallback(window, e.focused); },
            [&](const CursorEntered& e) { ImGui_ImplGlfw_CursorEnterCallback(window, e.entered); },
            [&](const CursorMoved&   e) { ImGui_ImplGlfw_CursorPosCallback  (window, e.x, e.y); },
            [&](const ButtonPressed& e) { ImGui_ImplGlfw_MouseButtonCallback(window, underlying(e.button), underlying(e.state), underlying(e.mods)); },
            [&](const Scrolled&      e) { ImGui_ImplGlfw_ScrollCallback     (window, e.dx, e.dy); },
            [&](const KeyPressed&    e) { ImGui_ImplGlfw_KeyCallback        (window, underlying(e.key), e.scancode, underlying(e.state), underlying(e.mods)); },
            [&](const CharInput&     e) { ImGui_ImplGlfw_CharCallback       (window, e.codepoint); },
            [&](const auto&           ) { /* do nothing */ },
            // clang-format on

        });
    }

    /**
     * @brief Process one event from window.
     *
     * @param event The event from window.
     *
     * Since glfw_cpp already installs all the callback to GLFW internals and accumulate it into a queue
     * for each window, we need to send the events to imgui manually.
     */
    inline void process_event(GLFWwindow* window, const Event& event)
    {
        using namespace event;

        // TODO: handle monitor event
        event.visit(Overload{
            // clang-format off
            [&](const WindowFocused& e) { ImGui_ImplGlfw_WindowFocusCallback(window, e.focused); },
            [&](const CursorEntered& e) { ImGui_ImplGlfw_CursorEnterCallback(window, e.entered); },
            [&](const CursorMoved&   e) { ImGui_ImplGlfw_CursorPosCallback  (window, e.x, e.y); },
            [&](const ButtonPressed& e) { ImGui_ImplGlfw_MouseButtonCallback(window, underlying(e.button), underlying(e.state), underlying(e.mods)); },
            [&](const Scrolled&      e) { ImGui_ImplGlfw_ScrollCallback     (window, e.dx, e.dy); },
            [&](const KeyPressed&    e) { ImGui_ImplGlfw_KeyCallback        (window, underlying(e.key), e.scancode, underlying(e.state), underlying(e.mods)); },
            [&](const CharInput&     e) { ImGui_ImplGlfw_CharCallback       (window, e.codepoint); },
            [&](const auto&           ) { /* do nothing */ },
            // clang-format on

        });
    }

    /**
     * @brief Creates new imgui frame.
     */
    inline void new_frame()
    {
        ImGui_ImplGlfw_NewFrame();
    }

    /**
     * @brief Shutdown the imgui context.
     *
     * This function will shutdown the imgui context if it was initialized.
     */
    inline void shutdown() noexcept
    {
        ImGui_ImplGlfw_Shutdown();
    }
}

#endif /* end of include guard: GLFW_CPP_IMGUI_HPP */
