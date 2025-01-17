// To use glfw_cpp::VideoMode, you need the full definition which glfw_cpp didn't provide, so including glfw C
// header is necessary. I plan on creating a new type on glfw_cpp so there is no need to do this.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <fmt/core.h>
#include <glfw_cpp/instance.hpp>
#include <glfw_cpp/monitor.hpp>

auto operator==(const glfw_cpp::Monitor::VideoMode& lhs, const glfw_cpp::Monitor::VideoMode& rhs) -> bool
{
    return lhs.m_width == rhs.m_width              //
        && lhs.m_height == rhs.m_height            //
        && lhs.m_red_bits == rhs.m_red_bits        //
        && lhs.m_green_bits == rhs.m_green_bits    //
        && lhs.m_blue_bits == rhs.m_blue_bits      //
        && lhs.m_refresh_rate == rhs.m_refresh_rate;
}

int main()
{
    auto instance = glfw_cpp::init(glfw_cpp::Api::NoApi{});
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
        for (auto& mode : videoModes) {
            fmt::print("\t\t{}x{} \tat {}Hz", mode.m_width, mode.m_height, mode.m_refresh_rate);
            if (isCurrent = (mode == currentMode); isCurrent) {
                fmt::println("\t(current)");
            } else {
                fmt::println("");
            }
        }

        if (!isCurrent) {
            fmt::println("current video mode:");
            fmt::println(
                "\t\t{}x{} \tat {}Hz", currentMode.m_width, currentMode.m_height, currentMode.m_refresh_rate
            );
        }

        fmt::println("");
    }
}
