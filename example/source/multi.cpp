#include "shader.hpp"
#include "plane.hpp"

// this header only includes the wrapper and does not include the GLFW header itself
#include <glbinding/gl/functions.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <fmt/core.h>
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>    // you include the GLFW header manually if you want to use it

#include <filesystem>
#include <thread>
#include <type_traits>

namespace glfw = glfw_cpp;

void logCallback(glfw::Context::LogLevel level, std::string message)
{
    auto levelStr = [&] {
        switch (level) {
        case glfw::Context::LogLevel::NONE: return "NONE";
        case glfw::Context::LogLevel::DEBUG: return "DEBUG";
        case glfw::Context::LogLevel::INFO: return "INFO";
        case glfw::Context::LogLevel::WARNING: return "WARNING";
        case glfw::Context::LogLevel::ERROR: return "ERROR";
        case glfw::Context::LogLevel::CRITICAL: return "CRITICAL";
        default: [[unlikely]] return "UNKNOWN";
        }
    };
    fmt::println(stderr, "[GLFW] [{}] {}", levelStr(), message);
}

void handleEvents(glfw::Window& window, std::deque<glfw::Event>&& events)
{
    for (const glfw::Event& event : events) {
        // Event is a std::variant, use visit to see its content. Internally it is using std::visit.
        // Not the best looking code IMO compared to unsafe manual tagged union (use getIf instead
        // if you think it would be better or use an overloaded set)

        event.visit([&](auto& e) {
            using E  = glfw::Event;
            using Ev = std::decay_t<decltype(e)>;

            if constexpr (std::same_as<Ev, E::WindowResized>) {
                gl::glViewport(0, 0, e.m_width, e.m_height);
            } else if constexpr (std::same_as<Ev, E::KeyPressed>) {
                using K = glfw::KeyCode;
                if (e.m_state != glfw::KeyState::PRESS) {
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

void threadFun(glfw::Window&& window, float side, float color)
{
    window.bind();

    auto shader = Shader{ "asset/shader/shader.vert", "asset/shader/shader.frag" };
    auto plane  = Plane{ side };

    window.run([&](auto&& eventQueue) {
        handleEvents(window, std::move(eventQueue));

        gl::glClearColor(0.1F * color, 0.1F * color, 0.11F * color, 1.0F);    // NOLINT
        gl::glClear(gl::GL_COLOR_BUFFER_BIT);

        shader.use();
        plane.draw();
    });
}

int main()
{
    // Hey, different loader from single.cpp. It's not GLAD anymore
    auto loader = [](auto handle, auto proc) {
        glbinding::initialize((glbinding::ContextHandle)handle, proc);
    };
    auto api = glfw::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw::Api::OpenGL::Profile::CORE,
        .m_loader  = loader,
    };

    auto context       = glfw::init(api, logCallback);
    auto windowManager = context->createWindowManager();

    auto window1 = windowManager.createWindow({}, "Learn glfw-cpp 1", 800, 600, false);

    using F      = glfw::WindowHint::FlagBit;
    auto hint    = glfw::WindowHint{ .m_flags = F::DEFAULT ^ F::RESIZABLE };
    auto window2 = windowManager.createWindow(
        hint, "Learn glfw-cpp 2 (not resizable)", 800, 600, false
    );

    auto thread1 = std::jthread{ threadFun, std::move(window1), 1.0F, 1 };
    auto thread2 = std::jthread{ threadFun, std::move(window2), -1.0F, 2 };

    while (windowManager.hasWindowOpened()) {
        using glfw::operator""_fps;
        windowManager.pollEvents(120_fps);    // NOLINT
    }
}
