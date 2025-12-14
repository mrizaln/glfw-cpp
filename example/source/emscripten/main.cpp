#include <GLES2/gl2.h>
#include <emscripten/emscripten.h>
#include <glfw_cpp/emscripten.hpp>
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
    auto glfw = glfw_cpp::init({});

    glfw->set_error_callback([](auto code, auto msg) {
        println("glfw-cpp [{:-^20}]: {}", to_string(code), msg);
    });

    glfw->apply_hints({
        .api = glfw_cpp::api::OpenGLES{ 
            .version_major = 2, 
            .version_minor = 0, 
        },
        .emscripten = {
            .canvas_selector = "#canvas",
            .resize_selector = "#canvas-container",
            .handle_selector = "#canvas-handle",
        },
    });

    auto window = glfw->create_window(800, 600, "Hello emscripten from glfw-cpp");

    glfw_cpp::make_current(window.handle());

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
        window.swap_events().visit(ev::Overload{
            [&](const ev::KeyPressed& e) {
                if (e.state != KS::Press) {
                    return;
                }
                switch (e.key) {
                case KC::Q: window.request_close(); break;
                case KC::F: {
                    if (not glfw_cpp::em::is_window_fullscreen(window.handle())) {
                        glfw_cpp::em::request_fullscreen(window.handle(), false, true);
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
        window.swap_buffers();

        return not window.should_close();
    });
}
