#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <cmath>
#include <iostream>

using namespace gl;    // from <glbinding/gl/gl.h>

int main()
{
    // Calls `glfwInit()` internally and returns an `std::unique_ptr<Instance>` that will call
    // `glfwTerminate()` on destruction. You can pass initialization hints here as argument.
    auto glfw = glfw_cpp::init({ .platform = glfw_cpp::hint::Platform::Any });

    // Set error callback for glfw functions.
    glfw->set_error_callback([](glfw_cpp::ErrorCode code, std::string_view msg) {
        fprintf(stderr, "glfw-cpp [%20s]: %s\n", to_string(code).data(), msg.data());
    });

    // Window creation hints are aggregated into one struct. the function will only set what you explicitly
    // set here.
    glfw->apply_hints({
        .api    = glfw_cpp::api::OpenGLES{ .version_major = 2, .version_minor = 0 },
        .window = { .focused = false },
    });

    // Window creation signature is the same but the last 2 arguments has its default value set to `nullptr`.
    auto window = glfw->create_window(800, 600, "Hello glfw-cpp");

    // make context current and load gl, use `get_current()` to get current context
    glfw_cpp::make_current(window.handle());
    glbinding::initialize(glfw_cpp::get_proc_address);

    // The `Window::run()` function is special function that loops over the lambda until close request
    // dispatched. Window will swap the buffer after calling the lambda in every iteration. Events from
    // `glfwPollEvents()` are aggregated into `EventQueue` that gets passed into the lambda which you can
    // process; no need for callbacks.

    // the underlying mechanism is just this:
    /*
        while (not window.should_close()) {
            const auto& evets = window.swap_events();    // swap events enqueued by `glfw_cpp::poll_events()`

            // event handling and rendering goes here...

            window.swap_buffers();    // swap buffers; may block until screen refresh if vsync is on
        }
    */

    window.run([&, elapsed = 0.0F](const glfw_cpp::EventQueue& events) mutable {
        using K      = glfw_cpp::KeyCode;
        namespace ev = glfw_cpp::event;

        // handling events

        // `EventQueue` contains multiple `Event` which was just a variant you can visit. To make things
        // easier the class also have visit member function (which is what being used below). You can always
        // use range-based for loop if you want to.
        {
            // clang-format off
            events.visit(ev::Overload{
                [&](const ev::KeyPressed&         e) { if (e.key == K::Q) window.request_close();           },
                [&](const ev::FramebufferResized& e) { glViewport(0, 0, e.width, e.height);                 },
                [&](const auto&                   e) { std::cout << glfw_cpp::event_name(e) << '\n'; },  // catch-all case
            });
            // clang-format on
        }

        // `Window` keeps a copy of (almost) every properties of the window (like pressed keys) in
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
        glfw->poll_events();
    });
}
