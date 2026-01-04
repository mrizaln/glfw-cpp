#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glfw_cpp/glfw_cpp.hpp>
#include <glfw_cpp/imgui.hpp>
#include <imgui_impl_opengl3.h>

using namespace gl;    // from <glbinding/gl/gl.h>

auto render_thread(glfw_cpp::Window& window)
{
    glfw_cpp::make_current(window.handle());
    glbinding::initialize(glfw_cpp::get_proc_address);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // replacing `ImGui_ImplGlfw_InitForOpenGL()`
    auto imgui_glfw = glfw_cpp::imgui::init_for_opengl(window.handle());
    ImGui_ImplOpenGL3_Init();

    glClearColor(0.1f, 0.1f, 0.11f, 1.0f);

    while (not window.should_close()) {
        namespace ev = glfw_cpp::event;

        // swap events queued by `glfw_cpp::poll_events()` or `glfw_cpp::wait_events()`
        const auto& events = window.swap_events();

        // process events manually.
        // NOTE: imgui should not install callback because there is no synchronization between the event
        //       pushing on `glfwPollEvents` thread and general imgui usage in this thread.
        imgui_glfw.process_events(events);

        // // or you can use the single event one
        // for (const auto& event : events) {
        //     imgui_glfw.process_event(event);
        // }

        events.visit(ev::Overload{
            [&](const ev::KeyPressed& k) {
                using K = glfw_cpp::KeyCode;
                using S = glfw_cpp::KeyState;
                if (k.key == K::Q and k.state == S::Press) {
                    window.request_close();
                }
            },
            [&](const auto&) { /* do nothing */ },
        });

        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        imgui_glfw.new_frame();    // replacing `ImGui_ImplGlfw_NewFrame()`
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.swap_buffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    imgui_glfw.shutdown();    // replacing `ImGui_ImplGlfw_Shutdown()`
}

int main()
{

    auto glfw = glfw_cpp::init({});

    glfw->set_error_callback([](glfw_cpp::ErrorCode code, std::string_view msg) {
        fprintf(stderr, "glfw-cpp [%20s]: %s\n", to_string(code).data(), msg.data());
    });

    glfw->apply_hints({ 
        .api = glfw_cpp::api::OpenGL{
            .version_major   = 3,
            .version_minor   = 3,
            .profile         = glfw_cpp::gl::Profile::Core,
        },
    });

    auto window        = glfw->create_window(1280, 720, "Hello ImGui from glfw-cpp", nullptr, nullptr);
    auto window_thread = std::jthread{ render_thread, std::ref(window) };

    while (glfw->has_window_opened()) {
        using glfw_cpp::operator""_fps;
        glfw->poll_events(120_fps);
    }
}
