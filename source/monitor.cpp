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

    Monitor::Position Monitor::position() const noexcept
    {
        auto position = Position{};
        glfwGetMonitorPos(m_handle, &position.m_x, &position.m_y);
        return position;
    }

    Monitor::WorkArea Monitor::work_area() const noexcept
    {
        auto area = WorkArea{};
        glfwGetMonitorWorkarea(m_handle, &area.m_x, &area.m_y, &area.m_width, &area.m_height);
        return area;
    }

    Monitor::PhysicalSize Monitor::physical_size() const noexcept
    {
        auto phys_size = PhysicalSize{};
        glfwGetMonitorPhysicalSize(m_handle, &phys_size.m_width_mm, &phys_size.m_height_mm);
        return phys_size;
    }

    Monitor::ContentScale Monitor::content_scale() const noexcept
    {
        auto scale = ContentScale{};
        glfwGetMonitorContentScale(m_handle, &scale.m_x, &scale.m_y);
        return scale;
    }

    Monitor::VideoMode Monitor::current_video_mode() const noexcept
    {
        auto* vid_mode = glfwGetVideoMode(m_handle);
        assert(vid_mode != nullptr && "video mode should not be empty");
        return {
            .m_width        = vid_mode->width,
            .m_height       = vid_mode->height,
            .m_red_bits     = vid_mode->redBits,
            .m_green_bits   = vid_mode->greenBits,
            .m_blue_bits    = vid_mode->blueBits,
            .m_refresh_rate = vid_mode->refreshRate,
        };
    }

    std::vector<Monitor::VideoMode> Monitor::available_video_modes() const noexcept
    {
        auto count     = 0;
        auto modes_ptr = glfwGetVideoModes(m_handle, &count);
        assert(modes_ptr != nullptr && "video modes should not be empty");

        auto modes = std::vector<Monitor::VideoMode>{};
        modes.reserve(static_cast<std::size_t>(count));
        for (auto i = 0; i < count; ++i) {
            auto mode = VideoMode{
                .m_width        = modes_ptr[i].width,
                .m_height       = modes_ptr[i].height,
                .m_red_bits     = modes_ptr[i].redBits,
                .m_green_bits   = modes_ptr[i].greenBits,
                .m_blue_bits    = modes_ptr[i].blueBits,
                .m_refresh_rate = modes_ptr[i].refreshRate,
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
        assert(ramp.m_red.size() == ramp.m_size);
        assert(ramp.m_green.size() == ramp.m_size);
        assert(ramp.m_blue.size() == ramp.m_size);

        // NOTE: const cast should be fine here since the data later will be copied by GLFW
        auto gamma = GLFWgammaramp{
            .red   = const_cast<unsigned short*>(ramp.m_red.data()),
            .green = const_cast<unsigned short*>(ramp.m_green.data()),
            .blue  = const_cast<unsigned short*>(ramp.m_blue.data()),
            .size  = static_cast<unsigned int>(ramp.m_size),
        };

        glfwSetGammaRamp(m_handle, &gamma);
    }

    Monitor::GammaRamp Monitor::get_gamma_ramp() const noexcept
    {
        auto ramp_ptr = glfwGetGammaRamp(m_handle);
        assert(ramp_ptr != nullptr && "gamma ramp should not be empty");

        return {
            .m_red   = { ramp_ptr->red, ramp_ptr->size },
            .m_green = { ramp_ptr->green, ramp_ptr->size },
            .m_blue  = { ramp_ptr->blue, ramp_ptr->size },
            .m_size  = ramp_ptr->size,
        };
    }
}
