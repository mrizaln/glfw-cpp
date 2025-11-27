#ifndef GLFW_CPP_MONITOR_HPP
#define GLFW_CPP_MONITOR_HPP

#include <span>
#include <string_view>
#include <vector>

struct GLFWvidmode;
struct GLFWgammaramp;
struct GLFWmonitor;

namespace glfw_cpp
{
    struct Position
    {
        int  x;
        int  y;
        auto operator<=>(const Position&) const = default;
    };

    struct WorkArea
    {
        int  x;
        int  y;
        int  width;
        int  height;
        auto operator<=>(const WorkArea&) const = default;
    };

    struct PhysicalSize
    {
        int  width_mm;
        int  height_mm;
        auto operator<=>(const PhysicalSize&) const = default;
    };

    struct ContentScale
    {
        float x;
        float y;
        auto  operator<=>(const ContentScale&) const = default;
    };

    struct VideoMode
    {
        int width;
        int height;
        int red_bits;
        int green_bits;
        int blue_bits;
        int refresh_rate;

        auto operator<=>(const VideoMode&) const = default;
    };

    struct GammaRamp
    {
        std::span<const unsigned short> red;
        std::span<const unsigned short> green;
        std::span<const unsigned short> blue;
        std::size_t                     size;
    };

    // Monitor is not a RAII class as it's not managing any resource. It's a simple wrapper of the
    // underlying GLFWmonitor.
    class Monitor
    {
    public:
        using Handle = GLFWmonitor*;

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

#endif /* end of include guard: GLFW_CPP_MONITOR_HPP */
