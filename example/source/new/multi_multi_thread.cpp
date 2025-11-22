#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <thread>

namespace glfw = glfw_cpp;

void window_thread(glfw::Window&& window)
{
    window.bind();

    auto mul = 1.0F / static_cast<float>(std::rand() % 10 + 1);

    window.run([&, elapsed = 0.0F](const auto& events) mutable {
        events.visit(glfw_cpp::event::Overload{
            [&](const glfw_cpp::event::KeyPressed& e) {
                if (e.key == glfw_cpp::KeyCode::Q) {
                    window.request_close();
                }
            },
            [&](const glfw_cpp::event::FramebufferResized& e) { glViewport(0, 0, e.width, e.height); },
            [&](const auto&) { /* do nothing */ },
        });
        elapsed += (float)window.delta_time();

        // funny color cycle
        const auto r = (std::sin(mul * 23.0F / 8.0F * elapsed) + 1.0F) * 0.1F + 0.4F;
        const auto g = (std::cos(mul * 13.0F / 8.0F * elapsed) + 1.0F) * 0.2F + 0.3F;
        const auto b = (std::sin(mul * 41.0F / 8.0F * elapsed) + 1.5F) * 0.2F;

        glClearColor(r, g, b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
    });
}

int main()
{
    // forgive me for using rand and srand, I'm lazy :D
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    auto glfw = glfw_cpp::init(glfw::api::OpenGL{
        .major   = 3,
        .minor   = 3,
        .profile = glfw::api::gl::Profile::Core,
        .loader  = [](auto /* handle */, auto proc) { gladLoadGLLoader((GLADloadproc)proc); },
    });

    auto window1 = glfw->create_window({}, "Hello glfw-cpp 1", 800, 600, false);
    auto window2 = glfw->create_window({}, "Hello glfw-cpp 2", 800, 600, false);
    auto window3 = glfw->create_window({}, "Hello glfw-cpp 3", 800, 600, false);
    auto window4 = glfw->create_window({}, "Hello glfw-cpp 4", 800, 600, false);

    auto thread11 = std::jthread{ window_thread, std::move(window1) };
    auto thread12 = std::jthread{ window_thread, std::move(window2) };
    auto thread21 = std::jthread{ window_thread, std::move(window3) };
    auto thread22 = std::jthread{ window_thread, std::move(window4) };

    while (glfw->has_window_opened()) {
        using glfw::operator""_fps;
        glfw->poll_events(120_fps);
    }
}
