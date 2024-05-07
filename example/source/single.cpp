#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <cmath>

namespace glfw = glfw_cpp;

int main()
{
    // Instance here refers to global state that glfw_cpp initializes in order to communicate with
    // the internals of GLFW. This class can only have one valid instance and throws when
    // instantiated again. The init function returns a RAII handle that automatically deinit the
    // Instance on destruction.
    glfw::Instance::Handle instance = glfw_cpp::init(glfw::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw::Api::OpenGL::Profile::CORE,
        .m_loader =
            [](glfw::Api::GlContext /* handle */, glfw::Api::GlGetProc proc) {
                // why two arguments?
                // Some GL loader libraries need a handle to load the proc.
                // Case on point: glbinding (though it is not required, but reocommended)
                gladLoadGLLoader((GLADloadproc)proc);
            },
    });

    glfw::WindowManager wm     = instance->createWindowManager();
    glfw::WindowHint    hint   = {};    // use default hint
    glfw::Window        window = wm.createWindow(hint, "Learn glfw-cpp", 800, 600);

    window.run([&, elapsed = 0.0F](std::deque<glfw::Event>&& events) mutable {
        for (const glfw::Event& event : events) {
            if (auto* e = event.getIf<glfw::Event::KeyPressed>()) {
                if (e->m_key == glfw::KeyCode::Q) {
                    window.requestClose();
                }
            } else if (auto* e = event.getIf<glfw::Event::FramebufferResized>()) {
                glViewport(0, 0, e->m_width, e->m_height);
            }
        }
        elapsed += static_cast<float>(window.deltaTime());

        // funny color cycle
        const float r = (std::sin(23.0F / 8.0F * elapsed) + 1.0F) * 0.1F + 0.4F;
        const float g = (std::cos(13.0F / 8.0F * elapsed) + 1.0F) * 0.2F + 0.3F;
        const float b = (std::sin(41.0F / 8.0F * elapsed) + 1.5F) * 0.2F;

        glClearColor(r, g, b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        wm.pollEvents();
    });
}
