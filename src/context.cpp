#include "glfw_cpp/context.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <mutex>
#include <stdexcept>
#include <utility>
#include <type_traits>
#include <variant>

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
    Context::Context(Api::Variant api)
        : m_api{ std::move(api) }
    {
        if (s_hasInstance) {
            throw std::runtime_error{ "Context can only have one active instance" };
        }

        if (glfwInit() != GLFW_TRUE) {
            throw std::runtime_error{ "Failed to initialize GLFW!" };
        }
        s_hasInstance = true;
        m_initialized = true;

        // clang-format off
        std::visit([](auto& api) {
            using A = std::remove_reference_t<decltype(api)>;
            if constexpr (std::same_as<A, Api::OpenGL>) {
                glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.m_major);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.m_minor);
                glfwWindowHint(GLFW_OPENGL_PROFILE, toGLFWProfile(api.m_profile));
            } else if constexpr (std::same_as<A, Api::OpenGLES>) {
                glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_ES_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.m_major);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.m_minor);
                glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_ANY_PROFILE);
            } else {
                glfwWindowHint(GLFW_CLIENT_API,            GLFW_NO_API);
            }
        }, m_api);
        // clang-format on
    }

    Context::~Context()
    {
        if (m_initialized) {
            s_hasInstance = false;
            glfwTerminate();
        }
    }

    Context::Context(Context&& other) noexcept
        : m_initialized{ std::exchange(other.m_initialized, false) }
        , m_api{ std::exchange(other.m_api, {}) }
        , m_logCallback{ std::exchange(other.m_logCallback, {}) }
    {
    }

    Context& Context::operator=(Context&& other) noexcept
    {
        if (this != &other) {
            if (m_initialized) {
                glfwTerminate();
            }

            m_initialized = std::exchange(other.m_initialized, false);
            m_api         = std::exchange(other.m_api, {});
            m_logCallback = std::exchange(other.m_logCallback, {});
        }
        return *this;
    }

    void Context::setErrorCallback(ErrorCallback* callback)
    {
        glfwSetErrorCallback(callback);
    }

    void Context::setLogCallback(LogFun callback)
    {
        std::unique_lock lock{ m_mutex };
        m_logCallback = std::move(callback);
    }

    void Context::log(LogLevel level, std::string msg) const
    {
        std::shared_lock lock{ m_mutex };
        if (m_logCallback) {
            m_logCallback(level, std::move(msg));
        }
    }
}
