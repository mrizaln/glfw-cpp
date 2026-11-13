#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <cmath>
#include <iostream>

using namespace gl;    // from <glbinding/gl/gl.h>

template <>
struct std::formatter<glfw_cpp::Event> : std::formatter<std::string_view>
{

    auto format(const glfw_cpp::Event& event, auto& ctx) const
    {
        namespace ev = glfw_cpp::event;

        std::format_to(ctx.out(), "{}", event.name());

        auto files_to_str = [](auto&& r) {
            auto str = std::string{};
            auto out = std::back_inserter(str);

            *out = '[';
            for (auto first = true; auto&& v : r) {
                if (not std::exchange(first, false)) {
                    *out = ',';
                    *out = ' ';
                }
                std::format_to(out, "{}", v.c_str());
            }
            *out = ']';

            return str;
        };

        event.visit(glfw_cpp::event::Overload{
            // clang-format off
            [&](const ev::WindowMoved&        e) { std::format_to(ctx.out(), "{{ x: {}, y: {}, dx: {}, dy: {} }}", e.x, e.y, e.dx, e.dy); },
            [&](const ev::WindowResized&      e) { std::format_to(ctx.out(), "{{ w: {}, h: {}, dw: {}, dh: {} }}", e.width, e.height, e.width_change, e.height_change); },
            [&](const ev::WindowClosed&        ) { std::format_to(ctx.out(), "{{ }}"); },
            [&](const ev::WindowRefreshed&     ) { std::format_to(ctx.out(), "{{ }}"); },
            [&](const ev::WindowFocused&      e) { std::format_to(ctx.out(), "{{ {} }}", e.focused); },
            [&](const ev::WindowIconified&    e) { std::format_to(ctx.out(), "{{ {} }}", e.iconified); },
            [&](const ev::WindowMaximized&    e) { std::format_to(ctx.out(), "{{ {} }}", e.maximized); },
            [&](const ev::WindowScaleChanged& e) { std::format_to(ctx.out(), "{{ x: {}, y: {} }}", e.x, e.y); },
            [&](const ev::FramebufferResized& e) { std::format_to(ctx.out(), "{{ w: {}, h: {}, dw: {}, dh: {} }}", e.width, e.height, e.width_change, e.height_change); },
            [&](const ev::ButtonPressed&      e) { std::format_to(ctx.out(), "{{ button: {}, state: {}, mods: {} }}", underlying(e.button), underlying(e.state), underlying(e.mods)); },
            [&](const ev::CursorMoved&        e) { std::format_to(ctx.out(), "{{ x: {}, y: {}, dx: {}, dy: {} }}", e.x, e.y, e.dx, e.dy); },
            [&](const ev::CursorEntered&      e) { std::format_to(ctx.out(), "{{ {} }}", e.entered); },
            [&](const ev::Scrolled&           e) { std::format_to(ctx.out(), "{{ dx: {}, dy: {} }}", e.dx, e.dy); },
            [&](const ev::KeyPressed&         e) { std::format_to(ctx.out(), "{{ key: {}, state: {}, mods: {}, scancode: {} }}", underlying(e.key), underlying(e.state), underlying(e.mods), e.scancode); },
            [&](const ev::CharInput&          e) { std::format_to(ctx.out(), "{{ codepoint: {} }}", e.codepoint); },
            [&](const ev::FileDropped&        e) { std::format_to(ctx.out(), "{{ {} }}", files_to_str(e.files)); },
            // clang-format on
        });

        return ctx.out();
    }
};

int main()
{
    // Calls `glfwInit()` internally and returns an `std::unique_ptr<Instance>` that will call
    // `glfwTerminate()` on destruction. You can pass initialization hints here as argument.
    auto glfw = glfw_cpp::init({ .wayland_libdecor = glfw_cpp::hint::WaylandLibdecor::Prefer });

    // Set error callback for glfw functions.
    glfw->set_error_callback([](glfw_cpp::ErrorCode code, std::string_view msg) {
        fprintf(stderr, "glfw-cpp [%20s]: %s\n", to_string(code).data(), msg.data());
    });

    // Window creation hints are aggregated into one struct. the function will only set what you explicitly
    // set here.
    glfw->apply_hints({
        .api    = glfw_cpp::api::OpenGLES{ .version_major = 2, .version_minor = 0 },
        .window = { .focused = false },
    });

    // Window creation signature is the same but the last 2 arguments has its default value set to `nullptr`.
    auto window = glfw->create_window(800, 600, "Hello glfw-cpp");

    // make context current and load gl, use `get_current()` to get current context
    glfw_cpp::make_current(window.handle());
    glbinding::initialize(glfw_cpp::get_proc_address);

    // The `Window::run()` function is special function that loops over the lambda until close request
    // dispatched. Window will swap the buffer after calling the lambda in every iteration. Events from
    // `glfwPollEvents()` are aggregated into `EventQueue` that gets passed into the lambda which you can
    // process; no need for callbacks.

    // the underlying mechanism is just this:
    /*
        while (not window.should_close()) {
            const auto& evets = window.swap_events();    // swap events enqueued by `glfw_cpp::poll_events()`

            // event handling and rendering goes here...

            window.swap_buffers();    // swap buffers; may block until screen refresh if vsync is on
        }
    */

    auto elapsed = 0.0f;

    while (not window.should_close()) {
        using K      = glfw_cpp::KeyCode;
        using S      = glfw_cpp::KeyState;
        namespace ev = glfw_cpp::event;

        // handling events

        // `EventQueue` contains multiple `Event` which was just a variant you can visit. To make things
        // easier the class also have visit member function (which is what being used below). You can always
        // use range-based for loop if you want to.
        {
            const auto& events = window.swap_events();

            for (const auto& event : events) {
                std::cout << std::format("event: {}\n", event);
            }

            // clang-format off
            events.visit(ev::Overload{
                [&](const ev::KeyPressed& e) {
                    if (e.state != S::Press) return;
                    if (e.key == K::Q) window.request_close();
                    if (e.key == K::C) window.set_capture_mouse(not window.is_mouse_captured());
                },
                [&](const ev::FramebufferResized& e) { glViewport(0, 0, e.width, e.height);                 },
                [&](const auto&                   ) { /* std::cout << glfw_cpp::event_name(e) << '\n'; */ },  // catch-all case
            });
            // clang-format on
        }

        // `Window` keeps a copy of (almost) every properties of the window (like pressed keys) in
        // itself. You can query it for continuous key input (for movement) for example.
        {
            const auto& keys = window.properties().key_state;

            if (keys.all_pressed({ K::H, K::J, K::L, K::K })) {
                std::cout << "HJKL key pressed all at once\n";
            }

            // if (keys.is_pressed(K::LeftShift) && keys.any_pressed({ K::W, K::A, K::S, K::D })) {
            //     std::cout << "WASD key pressed with shift key being held\n";
            // } else if (keys.any_pressed({ K::W, K::A, K::S, K::D })) {
            //     std::cout << "WASD key pressed\n";
            // }
        }

        elapsed += static_cast<float>(window.delta_time());

        // funny color cycle
        const float r = (std::sin(23.0F / 8.0F * elapsed) + 1.0F) * 0.1F + 0.4F;
        const float g = (std::cos(13.0F / 8.0F * elapsed) + 1.0F) * 0.2F + 0.3F;
        const float b = (std::sin(41.0F / 8.0F * elapsed) + 1.5F) * 0.2F;

        glClearColor(r, g, b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        window.swap_buffers();

        // poll events from the OS
        glfw->poll_events();
    }
}
