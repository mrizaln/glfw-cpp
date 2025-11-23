#include "plane.hpp"
#include "shader.hpp"

#include <fmt/core.h>
#include <fmt/std.h>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <filesystem>
#include <thread>

std::optional<std::string> read_file(const std::filesystem::path& path)
{
    auto file = std::ifstream{ path };

    if (not file) {
        fmt::println(stderr, "[Shader] Error reading vertex shader file '{}", path);
        return std::nullopt;
    }

    auto buffer = std::stringstream{};
    buffer << file.rdbuf();
    return buffer.str();
}

void handle_events(glfw_cpp::Window& window, const glfw_cpp::EventQueue& events)
{
    namespace ev = glfw_cpp::event;

    events.visit(ev::Overload{
        [&](const ev::WindowResized& e) { gl::glViewport(0, 0, e.width, e.height); },
        [&](const ev::KeyPressed& e) {
            using K = glfw_cpp::KeyCode;

            if (e.state != glfw_cpp::KeyState::Press) {
                return;
            }

            static int i = 0;

            switch (e.key) {
            case K::Q: window.request_close(); break;
            case K::A: window.update_title(std::format("hi: {}", i++)); break;
            default: /* nothing */;
            }
        },
        [&](const auto&) { /* do nothing on other events */ },
    });
}

void thread_fun(glfw_cpp::Window&& window, std::string_view vs, std::string_view fs, float side, float color)
{
    window.bind();

    auto shader = Shader{ vs, fs };
    auto plane  = Plane{ side };

    window.run([&](const glfw_cpp::EventQueue& event_queue) {
        handle_events(window, event_queue);

        gl::glClearColor(0.1F * color, 0.1F * color, 0.11F * color, 1.0F);    // NOLINT
        gl::glClear(gl::GL_COLOR_BUFFER_BIT);

        shader.use();
        plane.draw();
    });
}

int main()
{
    auto api = glfw_cpp::api::OpenGL{
        .major   = 3,
        .minor   = 3,
        .profile = glfw_cpp::api::gl::Profile::Core,
        .loader  = [](auto ctx, auto proc) { glbinding::initialize((glbinding::ContextHandle)ctx, proc); },
    };
    auto log = [](auto level, auto str) { fmt::println(stderr, "[GLFW: {:<7}] {}", to_string(level), str); };

    auto glfw = glfw_cpp::init(api, log);

    auto window1 = glfw->create_window({}, "Hello glfw-cpp 1", 800, 600, false);

    using F      = glfw_cpp::Flag;
    auto hint    = glfw_cpp::Hint{ .flags = F::Default & ~F::Resizable };
    auto window2 = glfw->create_window(hint, "Hello glfw-cpp 2 (not resizable)", 800, 600, false);

    auto vs_source = read_file("asset/shader/shader.vert").value();
    auto fs_source = read_file("asset/shader/shader.frag").value();

    auto thread1 = std::jthread{ thread_fun, std::move(window1), vs_source, fs_source, 1.0F, 1 };
    auto thread2 = std::jthread{ thread_fun, std::move(window2), vs_source, fs_source, -1.0F, 2 };

    while (glfw->has_window_opened()) {
        using glfw_cpp::operator""_fps;
        glfw->poll_events(120_fps);
    }
}
