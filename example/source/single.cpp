#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>
#include <GLFW/glfw3.h>

#include <cmath>

namespace glfw = glfw_cpp;

int main()
{
    // Context here refers to global state that glfw initializes not OpenGL context.
    // This class can only have one valid instance and throws when instantiated again.
    // The class can be moved around, though if you have instantiated a WindowManager (and
    // subsequently Window), it will become a problem since WindowManager and Window each have a
    // pointer to Context and if Context is moved the pointer will point to a moved value.
    glfw::Context context{ glfw::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw::Api::OpenGL::Profile::CORE,
        .m_loader  = [](auto /* handle */, auto proc) { gladLoadGLLoader((GLADloadproc)proc); },
    } };

    glfw::WindowManager windowManager{ context };

    glfw::WindowHint hint{};    // use default hint
    glfw::Window     window = windowManager.createWindow(hint, "Learn glfw-cpp", 800, 600);

    window.addKeyEventHandler(GLFW_KEY_Q, 0, glfw::Window::KeyActionType::CALLBACK, [](auto& w) {
        w.requestClose();
    });
    window.setFramebuffersizeCallback([](glfw::Window& /* window */, int width, int height) {
        glViewport(0, 0, width, height);
    });

    window.run([&, elapsed = 0.0F]() mutable {
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
