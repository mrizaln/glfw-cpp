#ifndef WINDOW_HPP_IROQWEOX
#define WINDOW_HPP_IROQWEOX

#include <concepts>
#include <functional>
#include <optional>
#include <thread>
#include <queue>

struct GLFWwindow;

namespace glfw_cpp
{
    class WindowManager;

    struct MouseButton
    {
        enum class State
        {
            RELEASED,
            PRESSED,
        };

        enum class Button
        {
            LEFT,
            RIGHT,
            MIDDLE,
        };

        State m_right;
        State m_left;
        State m_middle;

        State& operator[](Button button)
        {
            switch (button) {
            case Button::LEFT: return m_left;
            case Button::RIGHT: return m_right;
            case Button::MIDDLE: return m_middle;
            default: [[unlikely]] return m_left;    // just to suppress the warning
            }
        }

        State operator[](Button button) const
        {
            switch (button) {
            case Button::LEFT: return m_left;
            case Button::RIGHT: return m_right;
            case Button::MIDDLE: return m_middle;
            default: [[unlikely]] return m_left;    // just to suppress the warning
            }
        }
    };

    struct WindowProperties
    {
        std::string               m_title;
        int                       m_width;
        int                       m_height;
        std::pair<double, double> m_cursorPos;
        MouseButton               m_mouseButton;
    };

    class Window
    {
    public:
        friend WindowManager;

        template <typename Sig>
        using Fun = std::function<Sig>;    // use std::move_only_function in the future

        using KeyEvent    = int;    // GLFW_KEY_*
        using KeyModifier = int;    // GLFW_MOD_*

        enum class KeyActionType
        {
            CALLBACK,
            CONTINUOUS,
        };

        struct KeyEventHandler
        {
            KeyModifier        mods;
            KeyActionType      action;
            Fun<void(Window&)> handler;
        };

        using KeyMap = std::unordered_multimap<KeyEvent, KeyEventHandler>;

        using CursorPosCallbackFun = Fun<void(Window& window, double xPos, double yPos)>;
        using ScrollCallbackFun    = Fun<void(Window& window, double xOffset, double yOffset)>;
        using FramebufferSizeCallbackFun = Fun<void(Window& window, int width, int height)>;
        using MouseButtonCallback        = Fun<void(
            Window&             window,
            MouseButton::Button button,
            MouseButton::State  state,
            KeyModifier         mods
        )>;

        Window(Window&&) noexcept;
        Window& operator=(Window&&) noexcept;
        ~Window();

        Window(const Window&)           = delete;
        Window operator=(const Window&) = delete;

        // use the context on current thread;
        void bind();
        void unbind();
        void use(std::invocable auto&& fn);
        void setWindowSize(int width, int height);
        void updateTitle(const std::string& title);

        // main rendering loop
        void    run(Fun<void()>&& func);
        void    enqueueTask(Fun<void()>&& func);
        void    requestClose();
        double  deltaTime() const;
        Window& setVsync(bool value);
        Window& setCaptureMouse(bool value);
        Window& setCursorPosCallback(CursorPosCallbackFun&& func);
        Window& setScrollCallback(ScrollCallbackFun&& func);
        Window& setFramebuffersizeCallback(FramebufferSizeCallbackFun&& func);
        Window& setMouseButtonCallback(MouseButtonCallback&& func);

        // The function added will be called from the window thread.
        Window& addKeyEventHandler(
            KeyEvent             key,
            KeyModifier          mods,
            KeyActionType        action,
            Fun<void(Window&)>&& func
        );
        Window& addKeyEventHandler(
            std::initializer_list<KeyEvent> keys,
            KeyModifier                     mods,
            KeyActionType                   action,
            Fun<void(Window&)>&&            func
        );

        bool                    isVsyncEnabled() const { return m_vsync; }
        bool                    isMouseCaptured() const { return m_captureMouse; }
        WindowProperties&       properties() { return m_properties; }
        const WindowProperties& properties() const { return m_properties; }
        GLFWwindow*             handle() const { return m_windowHandle; }

        const std::optional<std::thread::id>& attachedThreadId() const
        {
            return m_attachedThreadId;
        };

    private:
        Window() = default;

        Window(
            WindowManager&     manager,
            std::size_t        id,
            GLFWwindow*        handle,
            WindowProperties&& properties,
            bool               bindImmediately
        );

        static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void cursorPosCallback(GLFWwindow* window, double xPos, double yPos);
        static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

        void processInput();
        void processQueuedTasks();
        void updateDeltaTime();

        void swap(Window& other) noexcept;

        WindowManager* m_manager;

        // window stuff
        std::size_t      m_id;
        bool             m_contextInitialized{ false };
        GLFWwindow*      m_windowHandle;
        WindowProperties m_properties;
        bool             m_vsync{ true };

        // input
        KeyMap                     m_keyMap;
        CursorPosCallbackFun       m_cursorPosCallback;
        ScrollCallbackFun          m_scrollCallback;
        FramebufferSizeCallbackFun m_framebufferSizeCallback;
        MouseButtonCallback        m_mouseButtonCallback;

        std::queue<Fun<void()>> m_taskQueue;

        double m_lastFrameTime{ 0.0 };
        double m_deltaTime{ 0.0 };

        bool                           m_captureMouse{ false };
        std::optional<std::thread::id> m_attachedThreadId;

        mutable std::mutex m_windowMutex;
        mutable std::mutex m_queueMutex;
    };
}

#endif /* end of include guard: WINDOW_HPP_IROQWEOX */
