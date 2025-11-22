#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glfw_cpp/extra/imgui.hpp>
#include <glfw_cpp/glfw_cpp.hpp>
#include <imgui_impl_opengl3.h>

auto render_thread(glfw_cpp::Window& window, glfw_cpp::extra::ImguiInterceptor& interceptor)
{
    window.bind();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // replacing `ImGui_ImplGlfw_InitForOpenGL()`
    auto imgui_glfw = glfw_cpp::extra::init_imgui_for_opengl(interceptor, window.handle());
    ImGui_ImplOpenGL3_Init();

    gl::glClearColor(0.1f, 0.1f, 0.11f, 1.0f);

    while (not window.should_close()) {
        namespace ev = glfw_cpp::event;
        window.poll().visit(ev::Overload{
            [&](const ev::KeyPressed& k) {
                using K = glfw_cpp::KeyCode;
                using S = glfw_cpp::KeyState;
                if (k.key == K::Q and k.state == S::Press) {
                    window.request_close();
                }
            },
            [&](const auto&) { /* do nothing */ },
        });

        gl::glClear(gl::GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        imgui_glfw.new_frame();    // replacing `ImGui_ImplGlfw_NewFrame()`
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.display();
    }

    ImGui_ImplOpenGL3_Shutdown();
    imgui_glfw.shutdown();    // replacing `ImGui_ImplGlfw_Shutdown()`
}

int main()
{
    auto api = glfw_cpp::api::OpenGL{
        .major   = 3,
        .minor   = 3,
        .profile = glfw_cpp::api::gl::Profile::Core,
        .loader  = [](auto, auto proc) { glbinding::initialize(proc); },
    };

    auto glfw        = glfw_cpp::init(api);
    auto interceptor = glfw_cpp::extra::ImguiInterceptor{};
    glfw->set_event_interceptor(&interceptor);

    auto window        = glfw->create_window({}, "Hello ImGui from glfw-cpp", 1280, 720, false);
    auto window_thread = std::jthread{ render_thread, std::ref(window), std::ref(interceptor) };

    while (glfw->has_window_opened()) {
        using glfw_cpp::operator""_fps;
        glfw->poll_events(120_fps);
    }
}
