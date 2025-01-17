#include "shader.hpp"
#include "plane.hpp"

#include <fmt/core.h>
#include <fmt/std.h>
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <filesystem>
#include <thread>
#include <type_traits>

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
    // Event is a std::variant, use visit to see its content. Internally it is using std::visit.
    // Not the best looking code IMO compared to unsafe manual tagged union (use get_if instead
    // if you think it would be better or use an overloaded set)

    events.visit([&](auto& e) {
        using E  = glfw_cpp::Event;
        using Ev = std::decay_t<decltype(e)>;

        if constexpr (std::same_as<Ev, E::WindowResized>) {
            gl::glViewport(0, 0, e.m_width, e.m_height);
        } else if constexpr (std::same_as<Ev, E::KeyPressed>) {
            using K = glfw_cpp::KeyCode;
            if (e.m_state != glfw_cpp::KeyState::Press) {
                return;
            }

            switch (e.m_key) {
            case K::Q: window.request_close(); break;
            case K::A: {
                static int i = 0;
                window.update_title(std::format("hi: {}", i++));
            } break;
            default: /* nothing */;
            }
        } else {
            /* do nothing */
        }
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
    // different loader from single.cpp. It's not GLAD anymore
    auto loader = [](auto handle, auto proc) {
        glbinding::initialize((glbinding::ContextHandle)handle, proc);
    };
    auto api = glfw_cpp::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw_cpp::Api::OpenGL::Profile::Core,
        .m_loader  = loader,
    };
    auto log = [](auto level, auto str) { fmt::println(stderr, "[GLFW: {:<7}] {}", to_string(level), str); };

    auto glfw = glfw_cpp::init(api, log);
    auto wm   = glfw->create_window_manager();

    auto window1 = wm->create_window({}, "Learn glfw-cpp 1", 800, 600, false);

    using F      = glfw_cpp::WindowHint::FlagBit;
    auto hint    = glfw_cpp::WindowHint{ .m_flags = F::Default ^ F::Resizable };
    auto window2 = wm->create_window(hint, "Learn glfw-cpp 2 (not resizable)", 800, 600, false);

    auto vs_source = read_file("asset/shader/shader.vert").value();
    auto fs_source = read_file("asset/shader/shader.frag").value();

    auto thread1 = std::jthread{ thread_fun, std::move(window1), vs_source, fs_source, 1.0F, 1 };
    auto thread2 = std::jthread{ thread_fun, std::move(window2), vs_source, fs_source, -1.0F, 2 };

    while (wm->has_window_opened()) {
        using glfw_cpp::operator""_fps;
        wm->poll_events(120_fps);    // NOLINT
    }
}
