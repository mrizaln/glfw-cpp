#include "shader.hpp"
#include "plane.hpp"

#include <fmt/core.h>
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <filesystem>
#include <thread>
#include <type_traits>

void logCallback(glfw_cpp::Instance::LogLevel level, std::string message)
{
    using L  = glfw_cpp::Instance::LogLevel;
    auto str = [&] {
        switch (level) {
        case L::NONE: return "NONE";
        case L::DEBUG: return "DEBUG";
        case L::INFO: return "INFO";
        case L::WARNING: return "WARNING";
        case L::ERROR: return "ERROR";
        case L::CRITICAL: return "CRITICAL";
        default: [[unlikely]] return "UNKNOWN";
        }
    }();
    fmt::println(stderr, "[GLFW] [{}] {}", str, message);
}

void handleEvents(glfw_cpp::Window& window, const glfw_cpp::EventQueue& events)
{
    for (const glfw_cpp::Event& event : events) {
        // Event is a std::variant, use visit to see its content. Internally it is using std::visit.
        // Not the best looking code IMO compared to unsafe manual tagged union (use getIf instead
        // if you think it would be better or use an overloaded set)

        event.visit([&](auto& e) {
            using E  = glfw_cpp::Event;
            using Ev = std::decay_t<decltype(e)>;

            if constexpr (std::same_as<Ev, E::WindowResized>) {
                gl::glViewport(0, 0, e.m_width, e.m_height);
            } else if constexpr (std::same_as<Ev, E::KeyPressed>) {
                using K = glfw_cpp::KeyCode;
                if (e.m_state != glfw_cpp::KeyState::PRESS) {
                    return;
                }

                switch (e.m_key) {
                case K::Q: window.requestClose(); break;
                case K::A: {
                    static int i = 0;
                    window.updateTitle(std::format("hi: {}", i++));
                } break;
                default: /* nothing */;
                }
            } else {
                /* do nothing */
            }
        });
    }
}

void threadFun(glfw_cpp::Window&& window, float side, float color)
{
    window.bind();

    auto shader = Shader{ "asset/shader/shader.vert", "asset/shader/shader.frag" };
    auto plane  = Plane{ side };

    window.run([&](const glfw_cpp::EventQueue& eventQueue) {
        handleEvents(window, eventQueue);

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
        .m_profile = glfw_cpp::Api::OpenGL::Profile::CORE,
        .m_loader  = loader,
    };

    auto glfw          = glfw_cpp::init(api, logCallback);
    auto windowManager = glfw->createWindowManager();

    auto window1 = windowManager->createWindow({}, "Learn glfw-cpp 1", 800, 600, false);

    using F      = glfw_cpp::WindowHint::FlagBit;
    auto hint    = glfw_cpp::WindowHint{ .m_flags = F::DEFAULT ^ F::RESIZABLE };
    auto window2 = windowManager->createWindow(hint, "Learn glfw-cpp 2 (not resizable)", 800, 600, false);

    auto thread1 = std::jthread{ threadFun, std::move(window1), 1.0F, 1 };
    auto thread2 = std::jthread{ threadFun, std::move(window2), -1.0F, 2 };

    while (windowManager->hasWindowOpened()) {
        using glfw_cpp::operator""_fps;
        windowManager->pollEvents(120_fps);    // NOLINT
    }
}
