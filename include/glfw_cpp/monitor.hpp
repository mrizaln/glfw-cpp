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

        explicit operator bool() noexcept { return m_handle != nullptr; }
        Handle   handle() const noexcept { return m_handle; }

        std::string_view name() const noexcept;
        Position         position() const noexcept;
        WorkArea         workArea() const noexcept;
        PhysicalSize     physicalSize() const noexcept;
        ContentScale     contentScale() const noexcept;

        const VideoMode*           currentVideoMode() const noexcept;
        std::span<const VideoMode> availableVideoModes() const noexcept;

        void      setGamma(float gamma) noexcept;
        void      setGammaRamp(const GammaRamp& ramp) noexcept;
        GammaRamp getGammaRamp() const noexcept;

    private:
        Handle m_handle;
    };

    std::vector<Monitor> getMonitors();
    Monitor              getPrimaryMonitor();
}

#endif /* end of include guard: MONITOR_HPP_F6RT4YUXEJ38H */
