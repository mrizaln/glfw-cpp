#include <GL/gl.h>
#include <GLFW/emscripten_glfw3.h>
#include <emscripten/emscripten.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <cmath>
#include <iostream>

template <typename Fn>
concept Loop = std::move_constructible<Fn> and requires (Fn fn) {
    { fn() } -> std::same_as<bool>;
};

template <Loop Fn>
void set_main_loop(Fn fn)
{
    auto wrapper = [](void* ptr) {
        auto* fn = static_cast<Fn*>(ptr);
        if (not(*fn)()) {
            delete fn;
            emscripten_cancel_main_loop();
        }
    };
    emscripten_set_main_loop_arg(wrapper, new Fn{ std::move(fn) }, 0, false);
}

template <typename... Args>
void println(std::format_string<Args...> fmt, Args&&... args)
{
    std::cout << std::format(fmt, std::forward<Args>(args)...) << '\n';
}

int main()
{
    auto api = glfw_cpp::api::OpenGLES{
        .major  = 2,
        .minor  = 0,
        .loader = nullptr,    // in emscripten, this can be null
    };

    auto logger = [](glfw_cpp::LogLevel level, std::string message) {
        println("{}: {}", glfw_cpp::to_string(level), message);
    };

    auto glfw = glfw_cpp::init(api, logger);
    auto wm   = glfw->create_window_manager();

    emscripten::glfw3::SetNextWindowCanvasSelector("#canvas");
    auto window = wm->create_window({}, "hello glfw-cpp", 800, 600);
    emscripten::glfw3::MakeCanvasResizable(window.handle(), "#canvas-container", "#canvas-handle");

    // unfortunately for us, the code will run after set_main_loop, so to prevent the resources from being
    // destroyed immediately before the main loop started, all relevant resources are moved to the lambda

    auto loop = [glfw = std::move(glfw), wm = std::move(wm), window = std::move(window), time = 0.0f]()    //
        mutable                                                                                            //
    {
        // polling events need to be done in the beginning
        wm->poll_events();

        using KC     = glfw_cpp::KeyCode;
        using KS     = glfw_cpp::KeyState;
        namespace ev = glfw_cpp::event;

        window.poll().visit(ev::Overload{
            [&](const ev::KeyPressed& e) {
                if (e.state != KS::Press) {
                    return;
                }
                switch (e.key) {
                case KC::Q: window.request_close(); break;
                case KC::F: {
                    if (not emscripten::glfw3::IsWindowFullscreen(window.handle())) {
                        emscripten::glfw3::RequestFullscreen(window.handle(), false, true);
                    }
                } break;
                default: /* nothing */;
                }
            },
            [&](const ev::FramebufferResized& e) { glViewport(0, 0, e.width, e.height); },
            [&](const auto&) { /* catch-all case, do nothing */ },
        });

        time += static_cast<float>(window.delta_time());

        const float r = (std::sin(23.0F / 8.0F * time) + 1.0F) * 0.1F + 0.4F;
        const float g = (std::cos(13.0F / 8.0F * time) + 1.0F) * 0.2F + 0.3F;
        const float b = (std::sin(41.0F / 8.0F * time) + 1.5F) * 0.2F;

        glClearColor(r, g, b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        // only updates delta time since glfwSwapBuffers is not implemented
        window.display();

        return not window.should_close();
    };

    set_main_loop(std::move(loop));
}
