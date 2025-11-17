#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/window_manager.hpp"

#include "util.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <format>
#include <utility>

namespace
{
    std::string_view error_to_string(int glfw_errc)
    {
#define CASE_ENTRY(Val)                                                                                      \
    case Val: return #Val

        switch (glfw_errc) {
            CASE_ENTRY(GLFW_NO_ERROR);
            CASE_ENTRY(GLFW_NOT_INITIALIZED);
            CASE_ENTRY(GLFW_NO_CURRENT_CONTEXT);
            CASE_ENTRY(GLFW_INVALID_ENUM);
            CASE_ENTRY(GLFW_INVALID_VALUE);
            CASE_ENTRY(GLFW_OUT_OF_MEMORY);
            CASE_ENTRY(GLFW_API_UNAVAILABLE);
            CASE_ENTRY(GLFW_VERSION_UNAVAILABLE);
            CASE_ENTRY(GLFW_PLATFORM_ERROR);
            CASE_ENTRY(GLFW_FORMAT_UNAVAILABLE);
            CASE_ENTRY(GLFW_NO_WINDOW_CONTEXT);
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 4
            CASE_ENTRY(GLFW_CURSOR_UNAVAILABLE);
            CASE_ENTRY(GLFW_FEATURE_UNAVAILABLE);
            CASE_ENTRY(GLFW_FEATURE_UNIMPLEMENTED);
            CASE_ENTRY(GLFW_PLATFORM_UNAVAILABLE);
#endif
        default: return "UNKNOWN_ERROR";
#undef CASE_ENTRY
        }
    }
}

namespace glfw_cpp
{
    Instance::~Instance()
    {
        if (m_initialized) {
            // WARN:this might fail though
            glfwTerminate();
        }
    }

    void Instance::set_logger(LogFun&& logger) noexcept
    {
        m_loggger = logger;
    }

    void Instance::reset() noexcept
    {
        if (m_initialized) {
            // WARN: this might fail though
            glfwTerminate();
        }
        m_initialized = false;
    }

    Instance& Instance::get() noexcept
    {
        return Instance::s_instance;
    }

    void Instance::log(LogLevel level, std::string msg) noexcept
    {
        auto& instance = Instance::get();
        assert(instance.m_initialized && "Instance not initialized!");

        if (instance.m_loggger) {
            instance.m_loggger(level, std::move(msg));
        }
    }

    Instance::Unique init(Api&& api, Instance::LogFun&& logger)
    {
        auto& instance = Instance::get();

        if (instance.m_initialized) {
            throw AlreadyInitialized{};
        }

        instance.m_api         = std::move(api);
        instance.m_loggger     = std::move(logger);
        instance.m_initialized = true;

        glfwSetErrorCallback([](int err, const char* msg) {
            Instance::log_c("[{}|{:#010x}] {}", error_to_string(err), err, msg);
        });

        if (glfwInit() != GLFW_TRUE) {
            instance.reset();
            util::throw_glfw_error();
        }

        instance.m_api.visit(util::VisitOverloaded{
            [](api::OpenGL& api) {
                if (api.loader == nullptr) {
                    throw EmptyLoader{};
                }

                auto gl_profile = [&] {
                    using P = glfw_cpp::api::gl::Profile;
                    switch (api.profile) {
                    case P::Core: return GLFW_OPENGL_CORE_PROFILE;
                    case P::Compat: return GLFW_OPENGL_COMPAT_PROFILE;
                    case P::Any: return GLFW_OPENGL_ANY_PROFILE;
                    default: [[unlikely]] return GLFW_OPENGL_CORE_PROFILE;
                    }
                }();

                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.major);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.minor);

                if (api.major >= 3 && api.minor >= 0) {
                    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, api.forward_compat);
                }

                if (api.major >= 3 && api.minor >= 2) {
                    glfwWindowHint(GLFW_OPENGL_PROFILE, gl_profile);
                }
            },
            [](api::OpenGLES& api) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.major);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.minor);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
            },
            [](api::NoApi&) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);    //
            },
        });

        return { &Instance::s_instance, [](Instance* instance) { instance->reset(); } };
    }

    WindowManager::Shared Instance::create_window_manager(IEventInterceptor* event_interceptor) noexcept
    {
        // using new here instead of `std::make_shared` since `WindowManager` constructor is private and can
        // only be seen by itself and its friends (`Instance` is one of it, but `std::make_shared` is not).
        return WindowManager::Shared{ new WindowManager{ std::this_thread::get_id(), event_interceptor } };
    }
}
