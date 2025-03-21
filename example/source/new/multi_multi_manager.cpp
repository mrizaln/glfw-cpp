#include <cstdlib>
#include <ctime>
#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>
#include <GLFW/glfw3.h>

#include <cmath>
#include <thread>

namespace glfw = glfw_cpp;

void window_thread(glfw::Window&& window)
{
    window.bind();

    auto mul = 1.0F / static_cast<float>(std::rand() % 10 + 1);

    window.run([&, elapsed = 0.0F](const auto& events) mutable {
        for (const glfw::Event& event : events) {
            if (auto e = event.get_if<glfw::Event::KeyPressed>()) {
                if (e->m_key == glfw::KeyCode::Q) {
                    window.request_close();
                }
            } else if (auto e = event.get_if<glfw::Event::FramebufferResized>()) {
                glViewport(0, 0, e->m_width, e->m_height);
            }
        }
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

    auto instance = glfw_cpp::init(glfw::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw::Api::OpenGL::Profile::Core,
        .m_loader  = [](auto /* handle */, auto proc) { gladLoadGLLoader((GLADloadproc)proc); },
    });

    // WindowManager does not have a restriction like Instance. It can be instantiated more than
    // once, but most WindowManager operations still must be called from main thread. You can think
    // of WindowManager as an orchestrator of multiple Windows created by itself.
    auto wm1 = instance->create_window_manager();
    auto wm2 = instance->create_window_manager();

    auto window11 = wm1->create_window({}, "Learn glfw-cpp 11", 800, 600, false);
    auto window12 = wm1->create_window({}, "Learn glfw-cpp 12", 800, 600, false);
    auto window21 = wm2->create_window({}, "Learn glfw-cpp 21", 800, 600, false);
    auto window22 = wm2->create_window({}, "Learn glfw-cpp 22", 800, 600, false);

    auto thread11 = std::jthread{ window_thread, std::move(window11) };
    auto thread12 = std::jthread{ window_thread, std::move(window12) };
    auto thread21 = std::jthread{ window_thread, std::move(window21) };
    auto thread22 = std::jthread{ window_thread, std::move(window22) };

    while (wm1->has_window_opened() || wm2->has_window_opened()) {
        wm1->poll_events();
        wm2->poll_events();

        using glfw::operator""_fps;
        std::this_thread::sleep_for(120_fps);
    }
}
