#include "print.hpp"

#include <glfw_cpp/instance.hpp>
#include <glfw_cpp/monitor.hpp>

int main()
{
    auto instance = glfw_cpp::init({});
    auto monitors = glfw_cpp::get_monitors();

    for (int count = 1; auto monitor : monitors) {
        println(">>> monitor {}", count++);
        println("name     : {}", monitor.name());

        auto [w, h] = monitor.physical_size();
        println("size     : {}mm x {}mm", w, h);

        auto [xpos, ypos] = monitor.position();
        println("position : {} x {}", xpos, ypos);

        auto [xscale, yscale] = monitor.content_scale();
        println("scale    : {} x {}", xscale, yscale);

        auto [x, y, ww, wh] = monitor.work_area();
        println("work area: ({}, {}) ({}x{})", x, y, ww, wh);

        auto currentMode = monitor.current_video_mode();
        auto videoModes  = monitor.available_video_modes();
        println("available video modes:");

        bool isCurrent = false;
        for (const auto& mode : videoModes) {
            print("\t\t{}x{} \tat {}Hz", mode.width, mode.height, mode.refresh_rate);
            if (isCurrent = (mode == currentMode); isCurrent) {
                println("\t(current)");
            } else {
                println("");
            }
        }

        if (!isCurrent) {
            println("current video mode:");
            println("\t\t{}x{} \tat {}Hz", currentMode.width, currentMode.height, currentMode.refresh_rate);
        }

        println("");
    }
}
