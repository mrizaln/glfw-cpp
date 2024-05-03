#include <fmt/core.h>
#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>
#include <GLFW/glfw3.h>

#include <cmath>

namespace glfw = glfw_cpp;

int main()
{
    // Context here refers to global state that glfw initializes not OpenGL context.
    // This class can only have one valid instance and throws when instantiated again.
    // The class can be moved around.
    auto context = glfw_cpp::init(glfw::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw::Api::OpenGL::Profile::CORE,
        .m_loader  = [](auto /* handle */, auto proc) { gladLoadGLLoader((GLADloadproc)proc); },
    });

    auto windowManager = context->createWindowManager();

    glfw::WindowHint hint{};    // use default hint
    glfw::Window     window = windowManager.createWindow(hint, "Learn glfw-cpp", 800, 600);

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
        const auto r = (std::sin(23.0F / 8.0F * elapsed) + 1.0F) * 0.1F + 0.4F;
        const auto g = (std::cos(13.0F / 8.0F * elapsed) + 1.0F) * 0.2F + 0.3F;
        const auto b = (std::sin(41.0F / 8.0F * elapsed) + 1.5F) * 0.2F;

        glClearColor(r, g, b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        windowManager.pollEvents();
    });
}
