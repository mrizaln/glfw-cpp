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
            int m_width_mm;
            int m_height_mm;
        };

        struct ContentScale
        {
            float m_x;
            float m_y;
        };

        struct VideoMode
        {
            int m_width;
            int m_height;
            int m_red_bits;
            int m_green_bits;
            int m_blue_bits;
            int m_refresh_rate;
        };

        struct GammaRamp
        {
            std::span<const unsigned short> m_red;
            std::span<const unsigned short> m_green;
            std::span<const unsigned short> m_blue;
            std::size_t                     m_size;
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
        WorkArea         work_area() const noexcept;
        PhysicalSize     physical_size() const noexcept;
        ContentScale     content_scale() const noexcept;

        VideoMode              current_video_mode() const noexcept;
        std::vector<VideoMode> available_video_modes() const noexcept;

        void      set_gamma(float gamma) noexcept;
        void      set_gamma_ramp(const GammaRamp& ramp) noexcept;
        GammaRamp get_gamma_ramp() const noexcept;

    private:
        Handle m_handle;
    };

    std::vector<Monitor> get_monitors();
    Monitor              get_primary_monitor();
}

#endif /* end of include guard: MONITOR_HPP_F6RT4YUXEJ38H */
