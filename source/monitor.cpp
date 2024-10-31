#include "glfw_cpp/monitor.hpp"

#include "util.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace glfw_cpp
{
    std::vector<Monitor> getMonitors()
    {
        int  count;
        auto handles = glfwGetMonitors(&count);

        util::checkGlfwError();

        std::vector<Monitor> monitors;
        monitors.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            monitors.emplace_back(handles[i]);
        }

        return monitors;
    }

    Monitor getPrimaryMonitor()
    {
        auto monitor = Monitor{ glfwGetPrimaryMonitor() };
        util::checkGlfwError();
        return monitor;
    }

    std::string_view Monitor::name() const noexcept
    {
        return glfwGetMonitorName(m_handle);
    }

    Monitor::Position Monitor::position() const noexcept
    {
        Position position;
        glfwGetMonitorPos(m_handle, &position.m_x, &position.m_y);
        return position;
    }

    Monitor::WorkArea Monitor::workArea() const noexcept
    {
        WorkArea workArea;
        glfwGetMonitorWorkarea(m_handle, &workArea.m_x, &workArea.m_y, &workArea.m_width, &workArea.m_height);
        return workArea;
    }

    Monitor::PhysicalSize Monitor::physicalSize() const noexcept
    {
        PhysicalSize physicalSize;
        glfwGetMonitorPhysicalSize(m_handle, &physicalSize.m_widthMM, &physicalSize.m_heightMM);
        return physicalSize;
    }

    Monitor::ContentScale Monitor::contentScale() const noexcept
    {
        ContentScale contentScale;
        glfwGetMonitorContentScale(m_handle, &contentScale.m_xScale, &contentScale.m_yScale);
        return contentScale;
    }

    const VideoMode* Monitor::currentVideoMode() const noexcept
    {
        return glfwGetVideoMode(m_handle);
    }

    std::span<const VideoMode> Monitor::availableVideoModes() const noexcept
    {
        int  count;
        auto modes = glfwGetVideoModes(m_handle, &count);
        return { modes, static_cast<std::size_t>(count) };
    }

    void Monitor::setGamma(float gamma) noexcept
    {
        glfwSetGamma(glfwGetPrimaryMonitor(), gamma);
    }

    void Monitor::setGammaRamp(const GammaRamp& gammaRamp) noexcept
    {
        glfwSetGammaRamp(m_handle, &gammaRamp);
    }

    GammaRamp Monitor::getGammaRamp() const noexcept
    {
        return *glfwGetGammaRamp(m_handle);
    }
}
