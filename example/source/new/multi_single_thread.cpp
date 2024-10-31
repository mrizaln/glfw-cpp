#include <fmt/core.h>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <ranges>

int main()
{
    auto glfw = glfw_cpp::init(glfw_cpp::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw_cpp::Api::OpenGL::Profile::Core,
        .m_loader  = [](auto handle,
                       auto proc) { glbinding::initialize((glbinding::ContextHandle)handle, proc); },
    });

    auto wm = glfw->createWindowManager();

    std::array<glfw_cpp::Window, 2> windows;
    for (auto i : std::views::iota(0u, 2u)) {
        windows[i] = wm->createWindow({}, fmt::format("Learn glfw-cpp {}", i), 800, 600, false);
    }

    while (wm->hasWindowOpened()) {
        for (auto& win : windows) {
            auto delta = win.use([&](const auto& events) {
                std::ranges::for_each(events, [&](const glfw_cpp::Event& ev) {
                    if (auto* e = ev.getIf<glfw_cpp::Event::KeyPressed>()) {
                        if (e->m_key == glfw_cpp::KeyCode::Q) {
                            win.requestClose();
                        }
                    }
                });

                auto epoch = std::chrono::steady_clock::now().time_since_epoch();
                auto time  = std::chrono::duration_cast<std::chrono::duration<float>>(epoch).count();

                // funny color cycle
                const float r = (std::sin(23.0F / 8.0F * time) + 1.0F) * 0.1F + 0.4F;
                const float g = (std::cos(13.0F / 8.0F * time) + 1.0F) * 0.2F + 0.3F;
                const float b = (std::sin(41.0F / 8.0F * time) + 1.5F) * 0.2F;

                gl::glClearColor(r, g, b, 1.0F);
                gl::glClear(gl::GL_COLOR_BUFFER_BIT);
            });
            if (!delta.has_value()) {
                win.hide();
            }
        }

        wm->pollEvents();
    }
}
