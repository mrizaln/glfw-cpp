#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <iostream>
#include <cmath>

namespace glfw = glfw_cpp;

int main()
{
    // init() calls glfwInit() internally and Instance::Handle will call glfwTerminate() on dtor.
    // Note that the graphics API can't be changed later, this is a design choice.
    glfw::Instance::Handle instance = glfw::init(glfw::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw::Api::OpenGL::Profile::CORE,
        .m_loader  = [](glfw::Api::GlContext /* handle */,
                       glfw::Api::GlGetProc proc) { gladLoadGLLoader((GLADloadproc)proc); },
    });

    // WindowManager is responsible for managing windows (think of window group)
    glfw::WindowManager wm = instance->createWindowManager();

    // graphics API hints are omitted from the WindowHint, only other relevant hints are included.
    glfw::WindowHint hint = {};    // use default hint

    glfw::Window window = wm.createWindow(hint, "Learn glfw-cpp", 800, 600);

    window.run([&, elapsed = 0.0F](std::vector<glfw::Event>&& events) mutable {
        // events
        for (const glfw::Event& event : events) {
            if (auto* e = event.getIf<glfw::Event::KeyPressed>()) {
                if (e->m_key == glfw::KeyCode::Q) {
                    window.requestClose();
                }
            } else if (auto* e = event.getIf<glfw::Event::FramebufferResized>()) {
                glViewport(0, 0, e->m_width, e->m_height);
            }
        }

        // continuous key input (for movement for example)
        {
            using K          = glfw::KeyCode;
            const auto& keys = window.properties().m_keyState;

            if (keys.allPressed({ K::H, K::J, K::L, K::K })) {
                std::cout << "HJKL key pressed all at once";
            }

            if (keys.isPressed(K::LEFT_SHIFT) && keys.anyPressed({ K::W, K::A, K::S, K::D })) {
                std::cout << "WASD key pressed with shift key being held";
            } else if (keys.anyPressed({ K::W, K::A, K::S, K::D })) {
                std::cout << "WASD key pressed";
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
