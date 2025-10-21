#include "glfw_cpp/monitor.hpp"

#include "util.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace glfw_cpp
{
    std::vector<Monitor> get_monitors()
    {
        auto count   = 0;
        auto handles = glfwGetMonitors(&count);

        util::check_glfw_error();

        auto monitors = std::vector<Monitor>{};
        monitors.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            monitors.emplace_back(handles[i]);
        }

        return monitors;
    }

    Monitor get_primary_monitor()
    {
        auto monitor = Monitor{ glfwGetPrimaryMonitor() };
        util::check_glfw_error();
        return monitor;
    }

    std::string_view Monitor::name() const noexcept
    {
        return glfwGetMonitorName(m_handle);
    }

    Position Monitor::position() const noexcept
    {
        auto position = Position{};
        glfwGetMonitorPos(m_handle, &position.x, &position.y);
        return position;
    }

    WorkArea Monitor::work_area() const noexcept
    {
        auto area = WorkArea{};
        glfwGetMonitorWorkarea(m_handle, &area.x, &area.y, &area.width, &area.height);
        return area;
    }

    PhysicalSize Monitor::physical_size() const noexcept
    {
        auto phys_size = PhysicalSize{};
        glfwGetMonitorPhysicalSize(m_handle, &phys_size.width_mm, &phys_size.height_mm);
        return phys_size;
    }

    ContentScale Monitor::content_scale() const noexcept
    {
        auto scale = ContentScale{};
        glfwGetMonitorContentScale(m_handle, &scale.x, &scale.y);
        return scale;
    }

    VideoMode Monitor::current_video_mode() const noexcept
    {
        auto* vid_mode = glfwGetVideoMode(m_handle);
        assert(vid_mode != nullptr && "video mode should not be empty");
        return {
            .width        = vid_mode->width,
            .height       = vid_mode->height,
            .red_bits     = vid_mode->redBits,
            .green_bits   = vid_mode->greenBits,
            .blue_bits    = vid_mode->blueBits,
            .refresh_rate = vid_mode->refreshRate,
        };
    }

    std::vector<VideoMode> Monitor::available_video_modes() const noexcept
    {
        auto count     = 0;
        auto modes_ptr = glfwGetVideoModes(m_handle, &count);
        assert(modes_ptr != nullptr && "video modes should not be empty");

        auto modes = std::vector<VideoMode>{};
        modes.reserve(static_cast<std::size_t>(count));
        for (auto i = 0; i < count; ++i) {
            auto mode = VideoMode{
                .width        = modes_ptr[i].width,
                .height       = modes_ptr[i].height,
                .red_bits     = modes_ptr[i].redBits,
                .green_bits   = modes_ptr[i].greenBits,
                .blue_bits    = modes_ptr[i].blueBits,
                .refresh_rate = modes_ptr[i].refreshRate,
            };
            modes.push_back(mode);
        }

        return modes;
    }

    void Monitor::set_gamma(float gamma) noexcept
    {
        glfwSetGamma(glfwGetPrimaryMonitor(), gamma);
    }

    void Monitor::set_gamma_ramp(const GammaRamp& ramp) noexcept
    {
        assert(ramp.red.size() == ramp.size);
        assert(ramp.green.size() == ramp.size);
        assert(ramp.blue.size() == ramp.size);

        // NOTE: const cast should be fine here since the data later will be copied by GLFW
        auto gamma = GLFWgammaramp{
            .red   = const_cast<unsigned short*>(ramp.red.data()),
            .green = const_cast<unsigned short*>(ramp.green.data()),
            .blue  = const_cast<unsigned short*>(ramp.blue.data()),
            .size  = static_cast<unsigned int>(ramp.size),
        };

        glfwSetGammaRamp(m_handle, &gamma);
    }

    GammaRamp Monitor::get_gamma_ramp() const noexcept
    {
        auto ramp_ptr = glfwGetGammaRamp(m_handle);
        assert(ramp_ptr != nullptr && "gamma ramp should not be empty");

        return {
            .red   = { ramp_ptr->red, ramp_ptr->size },
            .green = { ramp_ptr->green, ramp_ptr->size },
            .blue  = { ramp_ptr->blue, ramp_ptr->size },
            .size  = ramp_ptr->size,
        };
    }
}
