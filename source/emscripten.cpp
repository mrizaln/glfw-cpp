#include "glfw_cpp/emscripten.hpp"

#if not __EMSCRIPTEN__
    #error "Not compiled by emscripten, this should not be built"
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/emscripten_glfw3.h>

namespace glfw_cpp::em
{
    bool is_window_fullscreen(GLFWwindow* window)
    {
        return emscripten::glfw3::IsWindowFullscreen(window);
    }

    Result request_fullscreen(GLFWwindow* window, bool lock_pointer, bool resize_canvas)
    {
        auto ret = emscripten::glfw3::RequestFullscreen(window, lock_pointer, resize_canvas);
        return static_cast<Result>(ret);
    }

    SuperPlusKeyTimeout get_super_plus_key_timeout()
    {
        auto [timeout, repeat_timeout] = emscripten::glfw3::GetSuperPlusKeyTimeouts();
        return { timeout, repeat_timeout };
    }

    void set_super_plus_key_timeout(SuperPlusKeyTimeout timeout)
    {
        emscripten::glfw3::SetSuperPlusKeyTimeouts(timeout.timeout, timeout.repeat_timeout);
    }

    void open_url(std::string_view url, std::optional<std::string_view> target)
    {
        emscripten::glfw3::OpenURL(url, target);
    }

    bool is_runtime_platform_apple()
    {
        return emscripten::glfw3::IsRuntimePlatformApple();
    }
}
