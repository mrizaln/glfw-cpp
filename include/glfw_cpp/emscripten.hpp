#ifndef GLFW_CPP_EMSCRIPTEN_HPP
#define GLFW_CPP_EMSCRIPTEN_HPP

#include <optional>
#include <string_view>

struct GLFWwindow;

/**
 * @note Function for canvas selector for new window (`emscripten_glfw_set_next_window_canvas_selector`)
 * and funciton for canvas resizing (`emscripten_glfw_make_canvas_resizable`) are not directly wrapped in
 * here. Instead, those functionalities are integrated direclty with hints (see `glfw_cpp::hint::Emscripten`).
 */
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

    /**
     * @brief Check whether window is fullscreen.
     *
     * @param window Window handle.
     */
    bool is_window_fullscreen(GLFWwindow* window);

    /**
     * @brief Request the window to go fullscreen.
     *
     * @param window Window handle to the window that wants to go fullscreen.
     * @param lock_pointer Whether to lock the pointer.
     * @param resize_canvas
     * @return `Result::Success` if there was no issue, or an emscripten error code otherwise.
     *
     * @note Due to browser restrictions, this function should only be called from a user generated event
     * (like a keyboard event or a mouse button press).
     */
    Result request_fullscreen(GLFWwindow* window, bool lock_pointer, bool resize_canvas);

    /**
     * @brief Get timeout for super + <other> key press event before released event trigger.
     *
     * This code is part of workaround for emscripten: in the case when super key + any other key pressed the
     * up (release) event will never trigger. emscripten-glfw fixes this by adding a timeout for the press
     * event so that the release event trigger.
     *
     * The default value is 525ms for `timeout` and 125ms for `repeat_timeout`.
     */
    SuperPlusKeyTimeout get_super_plus_key_timeout();

    /**
     * @brief Set timeout for super + <other> key press event before released event trigger.
     *
     * @param timeout The desired timeout.
     *
     * This code is part of workaround for emscripten: in the case when super key + any other key pressed the
     * up (release) event will never trigger. emscripten-glfw fixes this by adding a timeout for the press
     * event so that the release event trigger.
     */
    void set_super_plus_key_timeout(SuperPlusKeyTimeout timeout);

    /**
     * @brief Convenient function to open a url.
     *
     * @param url The url to be opened.
     * @param target A string, without whitespace, specifying the name of the browsing context the resource is
     * being loaded into.
     *
     * @note Check https://developer.mozilla.org/en-US/docs/Web/API/Window/open for valid options for target.
     */
    void open_url(std::string_view url, std::optional<std::string_view> target = std::nullopt);

    /**
     * @brief Check if the runtime is running on Apple platform.
     */
    bool is_runtime_platform_apple();
#endif
}

#endif /* end of include guard: GLFW_CPP_EMSCRIPTEN_HPP */
