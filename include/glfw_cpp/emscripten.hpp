#ifndef GLFW_CPP_EMSCRIPTEN_HPP
#define GLFW_CPP_EMSCRIPTEN_HPP

#include <optional>
#include <string_view>

struct GLFWwindow;

namespace glfw_cpp::em
{
#if __EMSCRIPTEN__
    // compatible with `EMSCRIPTEN_RESULT`
    enum class Result : int
    {
        Success           = 0,
        Deferred          = 1,
        NotSupported      = -1,
        FailedNotDeferred = -2,
        InvalidTarget     = -3,
        UnknownTarget     = -4,
        InvalidParam      = -5,
        Failed            = -6,
        NoData            = -7,
        TimedOut          = -8,

    };

    struct SuperPlusKeyTimeout
    {
        int timeout;           // in ms
        int repeat_timeout;    // in ms
    };

    bool is_window_fullscreen(GLFWwindow* window);

    Result request_fullscreen(GLFWwindow* window, bool lock_pointer, bool resize_canvas);

    SuperPlusKeyTimeout get_super_plus_key_timeout();

    void set_super_plus_key_timeout(SuperPlusKeyTimeout timeout);

    void open_url(std::string_view url, std::optional<std::string_view> target = std::nullopt);

    bool is_runtime_platform_apple();
#endif
}

#endif /* end of include guard: GLFW_CPP_EMSCRIPTEN_HPP */
