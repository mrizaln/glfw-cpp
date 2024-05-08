#ifndef MONITOR_HPP_F6RT4YUXEJ38H
#define MONITOR_HPP_F6RT4YUXEJ38H

#include <span>
#include <string_view>
#include <vector>

struct GLFWvidmode;
struct GLFWgammaramp;
struct GLFWmonitor;

namespace glfw_cpp
{
    using VideoMode = GLFWvidmode;
    using GammaRamp = GLFWgammaramp;

    // Monitor is not a RAII class as it's not managing any resource. It's a simple wrapper of the
    // underlying GLFWmonitor.
    class Monitor
    {
    public:
        using Handle = GLFWmonitor*;

        struct Position
        {
            int m_x;
            int m_y;
        };

        struct WorkArea
        {
            int m_x;
            int m_y;
            int m_width;
            int m_height;
        };

        struct PhysicalSize
        {
            int m_widthMM;
            int m_heightMM;
        };

        struct ContentScale
        {
            float m_xScale;
            float m_yScale;
        };

        Monitor() = default;
        Monitor(Handle handle)
            : m_handle{ handle }
        {
        }

        explicit operator bool() { return m_handle != nullptr; }
        Handle   handle() const { return m_handle; }

        std::string_view name() const;
        Position         position() const;
        WorkArea         workArea() const;
        PhysicalSize     physicalSize() const;
        ContentScale     contentScale() const;

        const VideoMode*           currentVideoMode() const;
        std::span<const VideoMode> availableVideoModes() const;

        void      setGamma(float gamma);
        void      setGammaRamp(const GammaRamp& ramp);
        GammaRamp getGammaRamp() const;

    private:
        Handle m_handle;
    };

    std::vector<Monitor> getMonitors();
    Monitor              getPrimaryMonitor();
}

#endif /* end of include guard: MONITOR_HPP_F6RT4YUXEJ38H */
