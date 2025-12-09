#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>

#include <glfw_cpp/glfw_cpp.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <ranges>

using namespace gl;    // from <glbinding/gl/gl.h>

using Clock = std::chrono::steady_clock;

float to_fps(Clock::duration duration)
{
    using Sec = std::chrono::duration<float>;
    auto sec  = std::chrono::duration_cast<Sec>(duration);
    return 1.0f / sec.count();
}

int main()
{
    auto glfw = glfw_cpp::init({});

    glfw->set_error_callback([](glfw_cpp::ErrorCode code, std::string_view msg) {
        fprintf(stderr, "glfw-cpp [%20s]: %s\n", to_string(code).data(), msg.data());
    });

    glfw->apply_hints({
        .api = glfw_cpp::api::OpenGL{
            .version_major = 3,
            .version_minor = 3,
            .profile       = glfw_cpp::gl::Profile::Core,
        },
    });

    // default constructed windows will have nullptr as its handle
    auto windows = std::array<glfw_cpp::Window, 3>{};

    for (auto i : std::views::iota(0u, windows.size())) {
        windows[i] = glfw->create_window(800, 600, std::format("Hello glfw-cpp {}", i));

        glfw_cpp::make_current(windows[i].handle());
        glbinding::initialize(i, glfw_cpp::get_proc_address);    // initialize current context with handle i

        windows[i].set_vsync(false);
    }

    while (glfw->has_window_opened()) {
        for (auto i : std::views::iota(0u, windows.size())) {
            auto& win = windows[i];

            auto delta = win.use([&](const glfw_cpp::EventQueue& events) {
                // see https://glbinding.org/ for more detail
                glbinding::useCurrentContext();    // use current active context
                // glbinding::useContext(i);       // or use the handle registerned beforehand

                events.visit(glfw_cpp::event::Overload{
                    [&](const glfw_cpp::event::KeyPressed& e) {
                        e.key == glfw_cpp::KeyCode::Q ? win.request_close() : void();
                    },
                    [&](const auto&) { /* do nothing */ },

                });

                auto epoch = std::chrono::steady_clock::now().time_since_epoch();
                auto time  = std::chrono::duration_cast<std::chrono::duration<float>>(epoch).count();

                // funny color cycle
                const float r = (std::sin(23.0F / 8.0F * time) + 1.0F) * 0.1F + 0.4F;
                const float g = (std::cos(13.0F / 8.0F * time) + 1.0F) * 0.2F + 0.3F;
                const float b = (std::sin(41.0F / 8.0F * time) + 1.5F) * 0.2F;

                glClearColor(r, g, b, 1.0F);
                glClear(GL_COLOR_BUFFER_BIT);
            });

            if (!delta.has_value()) {
                win.hide();
            }
        }

        using glfw_cpp::operator""_fps;
        glfw->poll_events(120_fps);
    }
}
