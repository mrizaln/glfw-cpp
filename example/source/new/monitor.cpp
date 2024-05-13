#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <fmt/core.h>
#include <glfw_cpp/instance.hpp>
#include <glfw_cpp/monitor.hpp>

auto operator==(const glfw_cpp::VideoMode& lhs, const glfw_cpp::VideoMode& rhs) -> bool
{
    return lhs.width == rhs.width            //
        && lhs.height == rhs.height          //
        && lhs.redBits == rhs.redBits        //
        && lhs.greenBits == rhs.greenBits    //
        && lhs.blueBits == rhs.blueBits      //
        && lhs.refreshRate == rhs.refreshRate;
}

int main()
{
    auto instance = glfw_cpp::init(glfw_cpp::Api::NoApi{});
    auto monitors = glfw_cpp::getMonitors();

    for (int count = 1; auto monitor : monitors) {
        fmt::println(">>> monitor {}", count++);
        fmt::println("name     : {}", monitor.name());

        auto [w, h] = monitor.physicalSize();
        fmt::println("size     : {}mm x {}mm", w, h);

        auto [xpos, ypos] = monitor.position();
        fmt::println("position : {} x {}", xpos, ypos);

        auto [xscale, yscale] = monitor.contentScale();
        fmt::println("scale    : {} x {}", xscale, yscale);

        auto [x, y, ww, wh] = monitor.workArea();
        fmt::println("work area: ({}, {}) ({}x{})", x, y, ww, wh);

        auto currentMode = *monitor.currentVideoMode();
        auto videoModes  = monitor.availableVideoModes();
        fmt::println("available video modes:");

        bool isCurrent = false;
        for (auto& mode : videoModes) {
            fmt::print("\t\t{}x{} \tat {}Hz", mode.width, mode.height, mode.refreshRate);
            if (isCurrent = (mode == currentMode); isCurrent) {
                fmt::println("\t(current)");
            } else {
                fmt::println("");
            }
        }

        if (!isCurrent) {
            fmt::println("current video mode:");
            fmt::println(
                "\t\t{}x{} \tat {}Hz",
                currentMode.width,
                currentMode.height,
                currentMode.refreshRate
            );
        }

        fmt::println("");
    }
}
