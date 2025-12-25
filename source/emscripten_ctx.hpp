#ifndef GLFW_CPP_EMSCRIPTEN_CTX_HPP
#define GLFW_CPP_EMSCRIPTEN_CTX_HPP

#include "glfw_cpp/constants.hpp"

#include <string>
#include <string_view>

namespace
{
    std::optional<std::string> make_opt(const char* str)
    {
        return str ? std::optional<std::string>{ str } : std::nullopt;
    }
}

namespace glfw_cpp
{
    class EmscriptenCtx
    {
    public:
        static void set_has_context(bool value) { get().m_has_context = value; }
        static bool get_has_context() { return get().m_has_context; }

        static std::string_view                get_canvas_selector() { return get().m_canvas_selector; }
        static std::optional<std::string_view> get_resize_selector() { return get().m_resize_selector; }
        static std::optional<std::string_view> get_handle_selector() { return get().m_handle_selector; }

        static void set_canvas_selector(const char* s) { get().m_canvas_selector = s ? s : ""; }
        static void set_resize_selector(const char* s) { get().m_resize_selector = make_opt(s); }
        static void set_handle_selector(const char* s) { get().m_handle_selector = make_opt(s); }

        static void reset() { get() = {}; }

    private:
        static EmscriptenCtx& get()
        {
            static auto instance = EmscriptenCtx{};
            return instance;
        }

        // I need to cache to see whether the currently created window has context since there is no way to
        // get it from emscripten-glfw
        bool m_has_context = true;

        std::string                m_canvas_selector = default_canvas_selector;
        std::optional<std::string> m_resize_selector = std::nullopt;
        std::optional<std::string> m_handle_selector = std::nullopt;
    };
}

#endif    // GLFW_CPP_EMSCRIPTEN_CTX_HPP
