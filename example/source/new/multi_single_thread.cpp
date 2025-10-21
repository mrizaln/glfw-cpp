#include <fmt/core.h>
#include <fmt/std.h>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <ranges>

using Clock = std::chrono::steady_clock;

float to_fps(Clock::duration duration)
{
    using Sec = std::chrono::duration<float>;
    auto sec  = std::chrono::duration_cast<Sec>(duration);
    return 1.0f / sec.count();
}

int main()
{
    auto glfw = glfw_cpp::init(glfw_cpp::api::OpenGL{
        .major   = 3,
        .minor   = 3,
        .profile = glfw_cpp::api::gl::Profile::Core,
        .loader  = [](auto handle,
                     auto proc) { glbinding::initialize((glbinding::ContextHandle)handle, proc); },
    });

    auto wm = glfw->create_window_manager();

    std::array<glfw_cpp::Window, 3> windows;
    for (auto i : std::views::iota(0u, windows.size())) {
        windows[i] = wm->create_window({}, fmt::format("Learn glfw-cpp {}", i), 800, 600);
        windows[i].set_vsync(false);
        windows[i].unbind();
    }

    auto now = Clock::now();

    while (wm->has_window_opened()) {
        for (auto& win : windows) {
            auto delta = win.use([&](const auto& events) {
                std::ranges::for_each(events, [&](const glfw_cpp::Event& ev) {
                    if (auto* e = ev.get_if<glfw_cpp::event::KeyPressed>()) {
                        if (e->key == glfw_cpp::KeyCode::Q) {
                            win.request_close();
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

        using glfw_cpp::operator""_fps;
        wm->poll_events(120_fps);

        auto current = Clock::now();
        auto elapsed = current - now;
        now          = current;

        fmt::println("elapsed: {} fps", to_fps(elapsed));
    }
}
