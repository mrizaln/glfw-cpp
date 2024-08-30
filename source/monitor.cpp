#include "glfw_cpp/monitor.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace glfw_cpp
{
    std::vector<Monitor> getMonitors()
    {
        int  count;
        auto handles = glfwGetMonitors(&count);

        std::vector<Monitor> monitors;
        monitors.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            monitors.emplace_back(handles[i]);
        }

        return monitors;
    }

    Monitor getPrimaryMonitor()
    {
        return { glfwGetPrimaryMonitor() };
    }

    std::string_view Monitor::name() const
    {
        return glfwGetMonitorName(m_handle);
    }

    Monitor::Position Monitor::position() const
    {
        Position position;
        glfwGetMonitorPos(m_handle, &position.m_x, &position.m_y);
        return position;
    }

    Monitor::WorkArea Monitor::workArea() const
    {
        WorkArea workArea;
        glfwGetMonitorWorkarea(m_handle, &workArea.m_x, &workArea.m_y, &workArea.m_width, &workArea.m_height);
        return workArea;
    }

    Monitor::PhysicalSize Monitor::physicalSize() const
    {
        PhysicalSize physicalSize;
        glfwGetMonitorPhysicalSize(m_handle, &physicalSize.m_widthMM, &physicalSize.m_heightMM);
        return physicalSize;
    }

    Monitor::ContentScale Monitor::contentScale() const
    {
        ContentScale contentScale;
        glfwGetMonitorContentScale(m_handle, &contentScale.m_xScale, &contentScale.m_yScale);
        return contentScale;
    }

    const VideoMode* Monitor::currentVideoMode() const
    {
        return glfwGetVideoMode(m_handle);
    }

    std::span<const VideoMode> Monitor::availableVideoModes() const
    {
        int  count;
        auto modes = glfwGetVideoModes(m_handle, &count);
        return { modes, static_cast<std::size_t>(count) };
    }

    void Monitor::setGamma(float gamma)
    {
        glfwSetGamma(glfwGetPrimaryMonitor(), gamma);
    }

    void Monitor::setGammaRamp(const GammaRamp& gammaRamp)
    {
        glfwSetGammaRamp(m_handle, &gammaRamp);
    }

    GammaRamp Monitor::getGammaRamp() const
    {
        return *glfwGetGammaRamp(m_handle);
    }
}
