#include <emscripten/emscripten.h>

#include <GL/gl.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <cmath>
#include <iostream>

template <typename Fn>
void set_main_loop(Fn fn)
{
    emscripten_set_main_loop_arg([](void* fn) { (*static_cast<Fn*>(fn))(); }, (void*)&fn, 0, true);
}

int main()
{
    auto api = glfw_cpp::api::OpenGLES{
        .major  = 3,
        .minor  = 0,
        .loader = nullptr,    // in emscripten, this can be null
    };

    auto logger = [](glfw_cpp::LogLevel level, std::string message) {
        std::cout << glfw_cpp::to_string(level) << ": " << message << '\n';
    };

    auto glfw   = glfw_cpp::init(api, logger);
    auto window = glfw->create_window({}, "Hello emscripten from glfw-cpp", 800, 600);

    set_main_loop([&, elapsed = 0.0f]() mutable {
        using K      = glfw_cpp::KeyCode;
        namespace ev = glfw_cpp::event;

        // poll must be done in the beginning before drawing
        // see: https://github.com/pongasoft/emscripten-glfw/issues/21#issuecomment-3218244944
        glfw->poll_events();

        // handling events
        {
            // clang-format off
            window.poll().visit(ev::Overload{
                // [&](const ev::KeyPressed&         e) { if (e.key == K::Q) window.request_close();           },
                [&](const ev::FramebufferResized& e) { glViewport(0, 0, e.width, e.height);                 },
                [&](const auto&                   e) { std::cout << glfw_cpp::event_name(e) << '\n'; },  // catch-all case
            });
            // clang-format on
        }

        // `glfw_cpp::Window` keep a copy of (almost) every properties of the window (like pressed keys) in
        // itself. You can query it for continuous key input (for movement) for example.
        {
            const auto& keys = window.properties().key_state;

            if (keys.all_pressed({ K::H, K::J, K::L, K::K })) {
                std::cout << "HJKL key pressed all at once\n";
            }

            if (keys.is_pressed(K::LeftShift) && keys.any_pressed({ K::W, K::A, K::S, K::D })) {
                std::cout << "WASD key pressed with shift key being held\n";
            } else if (keys.any_pressed({ K::W, K::A, K::S, K::D })) {
                std::cout << "WASD key pressed\n";
            }
        }

        elapsed += static_cast<float>(window.delta_time());

        // funny color cycle
        const float r = (std::sin(23.0F / 8.0F * elapsed) + 1.0F) * 0.1F + 0.4F;
        const float g = (std::cos(13.0F / 8.0F * elapsed) + 1.0F) * 0.2F + 0.3F;
        const float b = (std::sin(41.0F / 8.0F * elapsed) + 1.5F) * 0.2F;

        glClearColor(r, g, b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        // poll events from the OS
        window.display();
    });
}
