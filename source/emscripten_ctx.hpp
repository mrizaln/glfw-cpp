#ifndef GLFW_CPP_EMSCRIPTEN_CTX_HPP
#define GLFW_CPP_EMSCRIPTEN_CTX_HPP

#include "glfw_cpp/constants.hpp"

#include <string>
#include <string_view>

namespace glfw_cpp
{
    class EmscriptenCtx
    {
    public:
        static void set_canvas_selector(std::string_view selector) { instance().canvas_selector = selector; }
        static std::string_view get_canvas_selector() { return instance().canvas_selector; }

        static void set_resize_selector(std::string_view selector) { instance().resize_selector = selector; }
        static std::string_view get_resize_selector() { return instance().resize_selector; }

        static void set_handle_selector(std::string_view selector) { instance().handle_selector = selector; }
        static std::string_view get_handle_selector() { return instance().handle_selector; }

        static void reset() { instance() = {}; }

    private:
        static EmscriptenCtx& instance()
        {
            static auto instance = EmscriptenCtx{};
            return instance;
        }

        std::string canvas_selector = default_canvas_selector;
        std::string resize_selector = {};
        std::string handle_selector = {};
    };
}

#endif    // GLFW_CPP_EMSCRIPTEN_CTX_HPP
