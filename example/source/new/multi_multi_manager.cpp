#include <cstdlib>
#include <ctime>
#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>
#include <GLFW/glfw3.h>

#include <cmath>
#include <thread>

namespace glfw = glfw_cpp;

void windowThread(glfw::Window&& window)
{
    window.bind();

    auto mul = 1.0F / static_cast<float>(std::rand() % 10 + 1);

    window.run([&, elapsed = 0.0F](auto&& events) mutable {
        for (const glfw::Event& event : events) {
            if (auto e = event.getIf<glfw::Event::KeyPressed>()) {
                if (e->m_key == glfw::KeyCode::Q) {
                    window.requestClose();
                }
            } else if (auto e = event.getIf<glfw::Event::FramebufferResized>()) {
                glViewport(0, 0, e->m_width, e->m_height);
            }
        }
        elapsed += (float)window.deltaTime();

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
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    auto instance = glfw_cpp::init(glfw::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw::Api::OpenGL::Profile::CORE,
        .m_loader  = [](auto /* handle */, auto proc) { gladLoadGLLoader((GLADloadproc)proc); },
    });

    // WindowManager does not have a restriction like Instance. It can be instantiated more than
    // once, but most WindowManager operations still must be called from main thread. You can think
    // of WindowManager as an orchestrator of multiple Windows created by itself.
    auto wm1 = instance->createWindowManager();
    auto wm2 = instance->createWindowManager();

    auto window11 = wm1.createWindow({}, "Learn glfw-cpp 11", 800, 600, false);
    auto window12 = wm1.createWindow({}, "Learn glfw-cpp 12", 800, 600, false);
    auto window21 = wm2.createWindow({}, "Learn glfw-cpp 21", 800, 600, false);
    auto window22 = wm2.createWindow({}, "Learn glfw-cpp 22", 800, 600, false);

    auto thread11 = std::jthread{ windowThread, std::move(window11) };
    auto thread12 = std::jthread{ windowThread, std::move(window12) };
    auto thread21 = std::jthread{ windowThread, std::move(window21) };
    auto thread22 = std::jthread{ windowThread, std::move(window22) };

    while (wm1.hasWindowOpened() || wm2.hasWindowOpened()) {
        wm1.pollEvents();
        wm2.pollEvents();

        using glfw::operator""_fps;
        std::this_thread::sleep_for(120_fps);
    }
}
