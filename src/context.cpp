#include "glfw_cpp/context.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <mutex>
#include <stdexcept>
#include <utility>

namespace
{
    int toGLFWProfile(glfw_cpp::Context::Profile profile)
    {
        switch (profile) {
        case glfw_cpp::Context::Profile::CORE: return GLFW_OPENGL_CORE_PROFILE;
        case glfw_cpp::Context::Profile::COMPAT: return GLFW_OPENGL_COMPAT_PROFILE;
        case glfw_cpp::Context::Profile::ANY: return GLFW_OPENGL_ANY_PROFILE;
        default: [[unlikely]] return GLFW_OPENGL_CORE_PROFILE;
        }
    }
}

namespace glfw_cpp
{
    Context::Context(Hint hint, GLLoaderFun glLoader)
        : m_hint{ hint }
        , m_loader{ std::move(glLoader) }
    {
        if (s_hasInstance) {
            throw std::runtime_error{ "Context can only have one active instance" };
        }

        if (glfwInit() != GLFW_TRUE) {
            throw std::runtime_error{ "Failed to initialize GLFW!" };
        }
        s_hasInstance = true;
        m_initialized = true;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, m_hint.m_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, m_hint.m_minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, toGLFWProfile(m_hint.m_profile));
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
        , m_hint{ std::exchange(other.m_hint, {}) }
        , m_loader{ std::exchange(other.m_loader, {}) }
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
            m_hint        = std::exchange(other.m_hint, {});
            m_loader      = std::exchange(other.m_loader, {});
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
