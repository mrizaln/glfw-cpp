#include "shader.hpp"
#include "plane.hpp"

#include <fmt/core.h>
#include <glfw_cpp/glfw_cpp.hpp>
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>

#include <filesystem>
#include <thread>

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
        }
    };
    fmt::println(stderr, "[GLFW] [{}] {}", levelStr(), message);
}

void threadFun(glfw::Window&& window, float side, float color)
{
    window.bind();

    auto shader = Shader{ "asset/shader/shader.vert", "asset/shader/shader.frag" };
    auto plane  = Plane{ side };

    window.addKeyEventHandler(GLFW_KEY_Q, 0, glfw::Window::KeyActionType::CALLBACK, [](auto& win) {
        win.requestClose();
    });

    window.run([&] {
        using namespace gl;

        glClearColor(0.1F * color, 0.1F * color, 0.11F * color, 1.0F);    // NOLINT
        glClear(GL_COLOR_BUFFER_BIT);

        const auto& prop = window.properties();
        glViewport(0, 0, prop.m_width, prop.m_height);

        shader.use();
        plane.draw();
    });
}

int main()
{
    auto hint = glfw::Context::Hint{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw::Context::Profile::CORE,
    };

    auto context = glfw::Context{
        hint,
        [](auto glContext, auto proc) {
            glbinding::initialize((glbinding::ContextHandle)glContext, proc);
        },
    };
    context.setLogCallback(logCallback);

    auto windowManager = glfw::WindowManager{ context };
    auto window1 = windowManager.createWindow("Learn glfw-cpp 1", 800, 600, false);    // NOLINT
    auto window2 = windowManager.createWindow("Learn glfw-cpp 2", 800, 600, false);    // NOLINT

    auto thread1 = std::jthread{ threadFun, std::move(window1), 1.0F, 1 };
    auto thread2 = std::jthread{ threadFun, std::move(window2), -1.0F, 2 };

    while (windowManager.hasWindowOpened()) {
        using glfw::operator""_fps;
        windowManager.pollEvents(120_fps);    // NOLINT
    }
}
