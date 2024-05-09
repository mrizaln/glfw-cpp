#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/window_manager.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <format>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace glfw_cpp
{
    Instance::~Instance()
    {
        if (m_initialized) {
            glfwTerminate();
        }
    }

    void Instance::reset()
    {
        glfwTerminate();
        m_initialized = false;
    }

    Instance& Instance::get()
    {
        return Instance::s_instance;
    }

    void Instance::log(LogLevel level, std::string msg)
    {
        auto& instance = Instance::get();
        assert(instance.m_initialized && "Instance not initialized!");

        if (instance.m_loggger) {
            instance.m_loggger(level, std::move(msg));
        }
    }

    Instance::Handle init(Api::Variant&& api, Instance::LogFun&& logger)
    {
        auto& instance = Instance::get();

        if (instance.m_initialized) {
            throw std::runtime_error{ "Instance already initialized!" };
        }

        instance.m_api         = std::move(api);
        instance.m_loggger     = std::move(logger);
        instance.m_initialized = true;

        if (glfwInit() != GLFW_TRUE) {
            throw std::runtime_error{ "Failed to initialize GLFW!" };
        }

        glfwSetErrorCallback([](int err, const char* msg) {
            Instance::log(
                Instance::LogLevel::ERROR, std::format("Internal error ({}) {}", err, msg)
            );
        });

        std::visit(
            [](auto& api) {
                using A = std::decay_t<decltype(api)>;
                if constexpr (std::same_as<A, Api::OpenGL>) {
                    auto glProfile = [&] {
                        using P = glfw_cpp::Api::OpenGL::Profile;
                        switch (api.m_profile) {
                        case P::CORE: return GLFW_OPENGL_CORE_PROFILE;
                        case P::COMPAT: return GLFW_OPENGL_COMPAT_PROFILE;
                        case P::ANY: return GLFW_OPENGL_ANY_PROFILE;
                        default: [[unlikely]] return GLFW_OPENGL_CORE_PROFILE;
                        }
                    }();

                    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.m_major);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.m_minor);
                    glfwWindowHint(GLFW_OPENGL_PROFILE, glProfile);
                } else if constexpr (std::same_as<A, Api::OpenGLES>) {
                    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.m_major);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.m_minor);
                    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
                } else {
                    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                }
            },
            instance.m_api
        );

        return { &Instance::s_instance, [](Instance* instance) { instance->reset(); } };
    }

    WindowManager Instance::createWindowManager()
    {
        return WindowManager{ std::this_thread::get_id() };
    }
}