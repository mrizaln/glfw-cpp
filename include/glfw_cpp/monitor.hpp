#ifndef GLFW_CPP_MONITOR_HPP
#define GLFW_CPP_MONITOR_HPP

#include <span>
#include <string_view>
#include <vector>

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

    /**
     * @class Monitor.
     * @brief Wrapper class for `GLFWmonitor`.
     *
     * Unlike `Window` this `Monitor` class is NOT a RAII wrapper around `GLFWmonitor` but rather just a thin
     * wrapper to allow for familiar dot notation syntax for retrieving or setting monitor parameters.
     */
    class Monitor
    {
    public:
        using Handle = GLFWmonitor*;

        Monitor() = default;
        Monitor(Handle handle)
            : m_handle{ handle }
        {
        }

        /**
         * @brief Get the name of the monitor (utf-8, null terminated).
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw error::NotInitialized If GLFW is not initialized yet.
         */
        std::string_view name() const;

        /**
         * @brief Get the position of the monitor's viewport on the virtual screen.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw error::NotInitialized If GLFW is not initialized yet.
         * @throw error::PlatformError If a platform-specific error occurred.
         */
        Position position() const;

        /**
         * @brief Get the work area of the monitor in screen coordinates.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw error::NotInitialized If GLFW is not initialized yet.
         * @throw error::PlatformError If a platform-specific error occurred.
         *
         * The work area is defined as the area of the monitor not occluded by the window system task bar
         * where present. If no task bar exists then the work area is the monitor resolution in screen
         * coordinates.
         */
        WorkArea work_area() const;

        /**
         * @brief Get the size of the display area of the monitor in milimeters.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw error::NotInitialized If GLFW is not initialized yet.
         *
         * Some platforms do not provide accurate monitor size information, either because the monitor EDID
         * data is incorrect or because the driver does not report it accurately.
         */
        PhysicalSize physical_size() const;

        /**
         * @brief Get the content scale of the monitor.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw error::NotInitialized If GLFW is not initialized yet.
         * @throw error::PlatformError If a platform-specific error occurred.
         *
         * The content scale is the ratio between the current DPI and the platform's default DPI. The content
         * scale may depend on both the monitor resolution and pixel density and on user settings. It may be
         * very different from the raw DPI calculated from the physical size and current resolution.
         */
        ContentScale content_scale() const;

        /**
         * @brief Get the current video mode of the monitor.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw error::NotInitialized If GLFW is not initialized yet.
         * @throw error::PlatformError If a platform-specific error occurred.
         *
         * If you have created a full screen window for that monitor, the return value will depend on whether
         * that window is iconified.
         */
        VideoMode current_video_mode() const;

        /**
         * @brief Get all of the video modes supported by the monitor.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw error::NotInitialized If GLFW is not initialized yet.
         * @throw error::PlatformError If a platform-specific error occurred.
         *
         * The returned array is sorted in ascending order, first by color bit depth (the sum of all channel
         * depths), then by resolution area (the product of width and height), then resolution width and
         * finally by refresh rate.
         */
        std::vector<VideoMode> available_video_modes() const;

        /**
         * @brief Generate an appropriately sized gamma ramp from the specified exponent and then set with it.
         *
         * @param gamma The desired component.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw error::NotInitialized If GLFW is not initialized yet.
         * @throw error::InvalidValue if the exponent is not greater than zero.
         * @throw error::PlatformError If a platform-specific error occurred.
         * @throw error::FeatureUnavailable If the feature is not implemented for the platform.
         *
         * @warn On Wayland gamma handling is a priviledged protocol thus not implemented and emits
         * error::FeatureUnavailable.
         *
         * The value must be a finite number greater than zero.
         *
         * The software controlled gamma ramp is applied in addition to the hardware gamma correction, which
         * today is usually an approximation of sRGB gamma. This means that setting a perfectly linear ramp,
         * or gamma 1.0, will produce the default (usually sRGB-like) behavior.
         */
        void set_gamma(float gamma);

        /**
         * @brief Set the current gamma ramp of the monitor.
         *
         * @param ramp The gamma ramp to use.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw error::NotInitialized If GLFW is not initialized yet.
         * @throw error::PlatformError If a platform-specific error occurred.
         * @throw error::FeatureUnavailable If the feature is not implemented for the platform.
         *
         * @warn The size of the ramp should match the size of the current ramp of the monitor.
         * @warn On Windows the gamma ramp size must be 256.
         * @warn On Wayland gamma handling is a priviledged protocol thus not implemented and emits
         * error::FeatureUnavailable.
         *
         * The original gamma ramp for that monitor is saved by GLFW the first time this function is called
         * and is restored on terminate (`glfwTerminate`, or in the case of this library, `Instance`
         * destructor).
         *
         * The software controlled gamma ramp is applied in addition to the hardware gamma correction, which
         * today is usually an approximation of sRGB gamma. This means that setting a perfectly linear ramp,
         * or gamma 1.0, will produce the default (usually sRGB-like) behavior.
         *
         * For gamma correct rendering with OpenGL or OpenGL ES, see `hint::Framebuffer::srgb_capable` hint.
         */
        void set_gamma_ramp(const GammaRamp& ramp);

        /**
         * @brief Get current gamma ramp of the monitor.
         *
         * @thread_safety This function must be called from the main thread.
         *
         * @throw error::NotInitialized If GLFW is not initialized yet.
         * @throw error::PlatformError If a platform-specific error occurred.
         * @throw error::FeatureUnavailable If the feature is not implemented for the platform.
         *
         * @warn On Wayland gamma handling is a priviledged protocol thus not implemented and emits
         * error::FeatureUnavailable.
         */
        GammaRamp get_gamma_ramp() const;

        /**
         * @brief Get the underlying `GLFWmonitor` handle.
         */
        Handle handle() const { return m_handle; }

        /**
         * @brief Boolean conversion to check whether the underlying handle is null or not.
         *
         * @return False on null, true otherwise.
         */
        explicit operator bool() { return m_handle != nullptr; }

    private:
        Handle m_handle;
    };

    /**
     * @brief Return all currently connected monitors.
     *
     * The primary monitor is always the first in the returned array.
     *
     * @thread_safety This function must be called from the main thread.
     *
     * @throw error::NotInitialized If GLFW is not initialized yet.
     */
    std::vector<Monitor> get_monitors();

    /**
     * @brief Return the primary monitor.
     *
     * This is usually the monitor where elements like the task bar or global menu bar ar located.
     *
     * @return The monitor or null if no monitor found.
     *
     * @thread_safety This function must be called from the main thread.
     *
     * @throw error::NotInitialized If GLFW is not initialized yet.
     */
    Monitor get_primary_monitor();
}

#endif /* end of include guard: GLFW_CPP_MONITOR_HPP */
