#include "plane.hpp"
#include "shader.hpp"

#include <fmt/core.h>
#include <fmt/std.h>

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>

#include <glfw_cpp/glfw_cpp.hpp>

#include <filesystem>
#include <fstream>
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
    glfw_cpp::make_current(window.handle());
    glbinding::initialize(glfw_cpp::get_proc_address);

    auto shader = Shader{ vs, fs };
    auto plane  = Plane{ side };

    window.run([&](const glfw_cpp::EventQueue& event_queue) {
        handle_events(window, event_queue);

        gl::glClearColor(0.1F * color, 0.1F * color, 0.11F * color, 1.0F);    // NOLINT
        gl::glClear(gl::GL_COLOR_BUFFER_BIT);

        // fmt::println("delta time: {}", window.delta_time());

        shader.use();
        plane.draw();
    });
}

int main()
{
    auto glfw = glfw_cpp::init({});

    glfw->set_error_callback([](glfw_cpp::ErrorCode code, std::string_view message) {
        fmt::println(stderr, "glfw-cpp [{:<20}]: {}", to_string(code), message);
    });
    glfw->apply_hints({
        .api = glfw_cpp::api::OpenGL{
            .version_major = 3,
            .version_minor = 3,
            .profile       = glfw_cpp::gl::Profile::Core,
        },
    });

    auto window1 = glfw->create_window(800, 600, "Hello glfw-cpp 1");

    glfw->apply_hints({ .window = { .resizable = false } });
    auto window2 = glfw->create_window(800, 600, "Hello glfw-cpp 2 (not resizable)");

    auto vs_source = read_file("asset/shader/shader.vert").value();
    auto fs_source = read_file("asset/shader/shader.frag").value();

    auto thread1 = std::jthread{ thread_fun, std::move(window1), vs_source, fs_source, 1.0F, 1 };
    auto thread2 = std::jthread{ thread_fun, std::move(window2), vs_source, fs_source, -1.0F, 2 };

    while (glfw->has_window_opened()) {
        using glfw_cpp::operator""_fps;
        glfw->poll_events(120_fps);
    }
}
