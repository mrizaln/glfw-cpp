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

    emscripten::glfw3::SetNextWindowCanvasSelector("#canvas");
    auto window = glfw->create_window({}, "Hello emscripten from glfw-cpp", 800, 600);
    emscripten::glfw3::MakeCanvasResizable(window.handle(), "#canvas-container", "#canvas-handle");

    // unfortunately for us the variables allocated on the stack currently will be destroyed immediately after
    // set_main_loop() since it won't block. thus all relevant resources must be moved inside the lambda.

    set_main_loop([glfw = std::move(glfw), window = std::move(window), elapsed = 0.0f]() mutable {
        using KC = glfw_cpp::KeyCode;
        using KS = glfw_cpp::KeyState;

        namespace ev = glfw_cpp::event;

        // polling for events needs to be done in the beginning
        // see: https://github.com/pongasoft/emscripten-glfw/issues/21#issuecomment-3218244944
        glfw->poll_events();    // calls `glfwPollEvents`, events then sent to each active Window instance

        // swap the event queue
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

        elapsed += static_cast<float>(window.delta_time());

        const float r = (std::sin(23.0F / 8.0F * elapsed) + 1.0F) * 0.1F + 0.4F;
        const float g = (std::cos(13.0F / 8.0F * elapsed) + 1.0F) * 0.2F + 0.3F;
        const float b = (std::sin(41.0F / 8.0F * elapsed) + 1.5F) * 0.2F;

        glClearColor(r, g, b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        // only updates delta time since glfwSwapBuffers is not implemented
        window.display();

        return not window.should_close();
    });
}
