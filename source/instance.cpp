#include "glfw_cpp/instance.hpp"
#include "glfw_cpp/window_manager.hpp"

#include "util.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <format>
#include <stdexcept>
#include <utility>

namespace
{
    std::string_view errorToString(int glfwErrc)
    {
#define CASE_ENTRY(Val)                                                                                      \
    case Val: return #Val
        switch (glfwErrc) {
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
        default: return "Unknown error";
#undef CASE_ENTRY
        }
    }
}

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

        glfwSetErrorCallback([](int err, const char* msg) {
            Instance::logE("(Internal error [{}|{:#8x}]) {}", errorToString(err), err, msg);

            // throw on errors that is not caused by application programmer error
            switch (err) {
            case GLFW_OUT_OF_MEMORY: [[fallthrough]];
            case GLFW_API_UNAVAILABLE: [[fallthrough]];
            case GLFW_VERSION_UNAVAILABLE: [[fallthrough]];
            case GLFW_PLATFORM_ERROR: [[fallthrough]];
            case GLFW_FORMAT_UNAVAILABLE:
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 4
                [[fallthrough]];
            case GLFW_CURSOR_UNAVAILABLE: [[fallthrough]];
            case GLFW_FEATURE_UNAVAILABLE: [[fallthrough]];
            case GLFW_FEATURE_UNIMPLEMENTED: [[fallthrough]];
            case GLFW_PLATFORM_UNAVAILABLE:
#endif
                throw std::runtime_error{
                    std::format("GLFW error [{}|{:#8x}]: {}", errorToString(err), err, msg)
                };
            }
        });

        if (glfwInit() != GLFW_TRUE) {
            instance.reset();
            throw std::runtime_error{ "Failed to initialize GLFW!" };
        }

        const auto&& configureApi = util::VisitOverloaded{
            [](Api::OpenGL& api) {
                if (api.m_major < 0 || api.m_minor < 0) {
                    throw std::runtime_error{ std::format(
                        "OpenGL version can't be negative: major={}, minor={}", api.m_major, api.m_minor
                    ) };
                }

                if (api.m_loader == nullptr) {
                    throw std::runtime_error{ "OpengGL loader can't be empty" };
                }

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

                if (api.m_major >= 3 && api.m_minor >= 0) {
                    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, api.m_forwardCompat);
                }

                if (api.m_major >= 3 && api.m_minor >= 2) {
                    glfwWindowHint(GLFW_OPENGL_PROFILE, glProfile);
                }
            },
            [](Api::OpenGLES& api) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.m_major);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.m_minor);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
            },
            [](Api::NoApi&) { glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); },
        };
        std::visit(configureApi, instance.m_api);

        return { &Instance::s_instance, [](Instance* instance) { instance->reset(); } };
    }

    WindowManager Instance::createWindowManager()
    {
        return WindowManager{ std::this_thread::get_id() };
    }
}
