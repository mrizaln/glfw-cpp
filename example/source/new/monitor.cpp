// To use glfw_cpp::VideoMode, you need the full definition which glfw_cpp didn't provide, so including glfw C
// header is necessary. I plan on creating a new type on glfw_cpp that prevents inclusion of this header.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <fmt/core.h>
#include <glfw_cpp/instance.hpp>
#include <glfw_cpp/monitor.hpp>

int main()
{
    auto instance = glfw_cpp::init(glfw_cpp::api::NoApi{});
    auto monitors = glfw_cpp::get_monitors();

    for (int count = 1; auto monitor : monitors) {
        fmt::println(">>> monitor {}", count++);
        fmt::println("name     : {}", monitor.name());

        auto [w, h] = monitor.physical_size();
        fmt::println("size     : {}mm x {}mm", w, h);

        auto [xpos, ypos] = monitor.position();
        fmt::println("position : {} x {}", xpos, ypos);

        auto [xscale, yscale] = monitor.content_scale();
        fmt::println("scale    : {} x {}", xscale, yscale);

        auto [x, y, ww, wh] = monitor.work_area();
        fmt::println("work area: ({}, {}) ({}x{})", x, y, ww, wh);

        auto currentMode = monitor.current_video_mode();
        auto videoModes  = monitor.available_video_modes();
        fmt::println("available video modes:");

        bool isCurrent = false;
        for (const auto& mode : videoModes) {
            fmt::print("\t\t{}x{} \tat {}Hz", mode.width, mode.height, mode.refresh_rate);
            if (isCurrent = (mode == currentMode); isCurrent) {
                fmt::println("\t(current)");
            } else {
                fmt::println("");
            }
        }

        if (!isCurrent) {
            fmt::println("current video mode:");
            fmt::println(
                "\t\t{}x{} \tat {}Hz", currentMode.width, currentMode.height, currentMode.refresh_rate
            );
        }

        fmt::println("");
    }
}
