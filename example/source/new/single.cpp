#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <cmath>
#include <iostream>

int main()
{
    // `glfw_cpp::init()` calls `glfwInit()` internally and returns an `glfw_cpp::Instance::Unique` that will
    // call `glfwTerminate()` on dtor. Note that the graphics API can't be changed later, this is a design
    // choice.
    auto instance = glfw_cpp::init(glfw_cpp::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw_cpp::Api::OpenGL::Profile::Core,
        .m_loader  = [](glfw_cpp::Api::GlContext /* handle */,
                       glfw_cpp::Api::GlGetProc proc) { gladLoadGLLoader((GLADloadproc)proc); },
    });

    // `WindowManager` is responsible for managing windows (think of window group). The only way to construct
    // it is through this `glfw_cpp::Instance::create_window_manager()` function which returns a
    // `std::shared_ptr<WindowManager>`. Each window created with this instance claims ownership of it (hence
    // the shared_ptr).
    auto wm = instance->create_window_manager();

    // graphics API hints are omitted from the `WindowHint`, only other relevant hints are included.
    auto hint = glfw_cpp::WindowHint{};    // use default hint

    auto window = wm->create_window(hint, "Learn glfw-cpp", 800, 600);

    window.run([&, elapsed = 0.0F](const glfw_cpp::EventQueue& events) mutable {
        // handling events
        {
            using K      = glfw_cpp::KeyCode;
            namespace ev = glfw_cpp::event;

            // clang-format off
            events.visit(ev::Overload{
                [&](const ev::KeyPressed&         e) { if (e.m_key == K::Q) window.request_close();         },
                [&](const ev::FramebufferResized& e) { glViewport(0, 0, e.m_width, e.m_height);             },
                [&](const auto&                   e) { std::cout << "event happened " << (void*)&e << '\n'; },  // catch-all case
            });
            // clang-format on
        }

        // `glfw_cpp::Window` keep a copy of (almost) every properties of the window (like pressed keys) in
        // itself. You can query it for continuous key input (for movement) for example.
        {
            using K          = glfw_cpp::KeyCode;
            const auto& keys = window.properties().m_key_state;

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

        wm->poll_events();
    });
}
