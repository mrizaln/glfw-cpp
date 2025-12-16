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
        static std::string_view                get_canvas_selector() { return get().canvas_selector; }
        static std::optional<std::string_view> get_resize_selector() { return get().resize_selector; }
        static std::optional<std::string_view> get_handle_selector() { return get().handle_selector; }

        static void set_canvas_selector(const char* s) { get().canvas_selector = s ? s : ""; }
        static void set_resize_selector(const char* s) { get().resize_selector = make_opt(s); }
        static void set_handle_selector(const char* s) { get().handle_selector = make_opt(s); }

        static void reset() { get() = {}; }

    private:
        static EmscriptenCtx& get()
        {
            static auto instance = EmscriptenCtx{};
            return instance;
        }

        std::string                canvas_selector = default_canvas_selector;
        std::optional<std::string> resize_selector = std::nullopt;
        std::optional<std::string> handle_selector = std::nullopt;
    };
}

#endif    // GLFW_CPP_EMSCRIPTEN_CTX_HPP
