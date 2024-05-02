#include "glfw_cpp/context.hpp"
#include "glfw_cpp/window_manager.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <format>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace
{
    int toGLFWProfile(glfw_cpp::Api::OpenGL::Profile profile)
    {
        using P = glfw_cpp::Api::OpenGL::Profile;
        switch (profile) {
        case P::CORE: return GLFW_OPENGL_CORE_PROFILE;
        case P::COMPAT: return GLFW_OPENGL_COMPAT_PROFILE;
        case P::ANY: return GLFW_OPENGL_ANY_PROFILE;
        default: [[unlikely]] return GLFW_OPENGL_CORE_PROFILE;
        }
    }
}

namespace glfw_cpp
{
    Context::~Context()
    {
        if (m_initialized) {
            glfwTerminate();
        }
    }

    void Context::reset()
    {
        glfwTerminate();
        m_initialized = false;
    }

    Context& Context::get()
    {
        return Context::s_instance;
    }

    void Context::log(LogLevel level, std::string msg)
    {
        auto& ctx = Context::get();
        assert(ctx.m_initialized && "Context not initialized!");

        if (ctx.m_logCallback) {
            ctx.m_logCallback(level, std::move(msg));
        }
    }

    Context::Handle init(Api::Variant&& api, Context::LogFun&& logCallback)
    {
        auto& ctx = Context::get();

        if (ctx.m_initialized) {
            throw std::runtime_error{ "Context already initialized!" };
        }

        ctx.m_api         = std::move(api);
        ctx.m_logCallback = std::move(logCallback);
        ctx.m_initialized = true;

        if (glfwInit() != GLFW_TRUE) {
            throw std::runtime_error{ "Failed to initialize GLFW!" };
        }

        glfwSetErrorCallback([](int err, const char* msg) {
            Context::log(Context::LogLevel::ERROR, std::format("Internal error ({}) {}", err, msg));
        });

        std::visit(
            [](auto& api) {
                using A = std::remove_reference_t<decltype(api)>;
                if constexpr (std::same_as<A, Api::OpenGL>) {
                    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.m_major);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.m_minor);
                    glfwWindowHint(GLFW_OPENGL_PROFILE, toGLFWProfile(api.m_profile));
                } else if constexpr (std::same_as<A, Api::OpenGLES>) {
                    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.m_major);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.m_minor);
                    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
                } else {
                    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                }
            },
            ctx.m_api
        );

        return { &Context::s_instance, [](Context* instance) { instance->reset(); } };
    }

    WindowManager Context::createWindowManager()
    {
        return WindowManager{ std::this_thread::get_id() };
    }
}
