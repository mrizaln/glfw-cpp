#ifndef GLFW_CPP_FWD_HPP
#define GLFW_CPP_FWD_HPP

struct GLFWwindow;
struct GLFWmonitor;

namespace glfw_cpp
{
    // error.hpp
    // ---------
    enum class ErrorCode : int;

    namespace error
    {
        class Error;
        class NotInitialized;
        class NoCurrentContext;
        class InvalidEnum;
        class InvalidValue;
        class OutOfMemory;
        class ApiUnavailable;
        class VersionUnavailable;
        class PlatformError;
        class FormatUnavailable;
        class NoWindowContext;
        class CursorUnavailable;
        class FeatureUnavailable;
        class FeatureUnimplemented;
        class PlatformUnavailable;
        class AlreadyInitialized;
        class WrongThreadAccess;
        class UnknownError;
    }
    // ---------

    // event.hpp
    // ---------
    namespace event
    {
        struct WindowMoved;
        struct WindowResized;
        struct WindowClosed;
        struct WindowRefreshed;
        struct WindowFocused;
        struct WindowIconified;
        struct FramebufferResized;
        struct ButtonPressed;
        struct CursorMoved;
        struct CursorEntered;
        struct Scrolled;
        struct KeyPressed;
        struct CharInput;
        struct FileDropped;
        struct WindowMaximized;
        struct WindowScaleChanged;
        struct MonitorConnected;
        struct JoystickConnected;
    }

    struct Event;
    class EventInterceptor;
    class EventQueue;
    // ---------

    // input.hpp
    // ---------
    enum class KeyCode : int;
    enum class KeyState : int;
    enum class MouseButton : int;
    enum class MouseButtonState : int;
    class ModifierKey;
    class KeyStateRecord;
    class MouseButtonStateRecord;
    // ---------

    // monitor.hpp
    // -----------
    struct Position;
    struct WorkArea;
    struct PhysicalSize;
    struct ContentScale;
    struct VideoMode;
    struct GammaRamp;
    class Monitor;
    // -----------

    // window.hpp
    // ----------
    struct Dimensions;
    struct FramebufferSize;
    struct CursorPosition;
    struct Attributes;
    struct Properties;
    class Window;
    // ----------

    // instance.hpp
    // ------------
    namespace gl
    {
        enum class Profile : int;
        enum class CreationApi : int;
        enum class Robustness : int;
        enum class ReleaseBehavior : int;
    }

    namespace api
    {
        // clang-format off
#if __EMSCRIPTEN__
        template <bool> struct WebGL;
#else
        template <bool> struct OpenGL;
        template <bool> struct OpenGLES;
#endif
        struct NoApi;
        // clang-format on
    }

    namespace hint
    {
        // clang-format off
        template <bool> struct Api;
        template <bool> struct Window;
        template <bool> struct Framebuffer;
        template <bool> struct Monitor;
        template <bool> struct Win32;
        template <bool> struct Cocoa;
        template <bool> struct Wayland;
        template <bool> struct X11;
        template <bool> struct Emscripten;
        // clang-format on

        enum class Platform : int;
        enum class AnglePlatform : int;
        enum class WaylandLibdecor : int;
    }

    template <bool>
    struct Hints;
    struct InitHints;
    class Instance;
    // ------------
}

#endif    // GLFW_CPP_FWD_HPP
