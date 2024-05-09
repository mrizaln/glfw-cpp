#ifndef EVENT_HPP_Q439GUKLHFWE
#define EVENT_HPP_Q439GUKLHFWE

#include "glfw_cpp/input.hpp"
#include "glfw_cpp/monitor.hpp"
#include "glfw_cpp/detail/helper.hpp"

#include <filesystem>
#include <type_traits>
#include <variant>

namespace glfw_cpp
{
    class Event
    {
    public:
        struct Empty
        {
        };

        struct WindowMoved
        {
            int m_xPos;
            int m_yPos;
        };

        struct WindowResized
        {
            int m_width;
            int m_height;
        };

        struct WindowClosed
        {
        };

        struct WindowRefreshed
        {
        };

        struct WindowFocused
        {
            bool m_focused;
        };

        struct WindowIconified
        {
            bool m_iconified;
        };

        struct FramebufferResized
        {
            int m_width;
            int m_height;
        };

        struct ButtonPressed
        {
            MouseButton      m_button;
            MouseButtonState m_state;
            ModifierKey      m_mods;
        };

        struct CursorMoved
        {
            double m_xPos;
            double m_yPos;
        };

        struct CursorEntered
        {
            bool m_entered;
        };

        struct Scrolled
        {
            double m_xOffset;
            double m_yOffset;
        };

        struct KeyPressed
        {
            KeyCode     m_key;
            int         m_scancode;
            KeyState    m_state;
            ModifierKey m_mods;
        };

        struct CharInput
        {
            unsigned int m_codepoint;
        };

        struct MonitorConnected
        {
            Monitor m_monitor;
            bool    m_connected;
        };

        struct FileDropped
        {
            std::vector<std::filesystem::path> m_files;
        };

        struct JoystickConnected
        {
            int  m_joystickId;    // No dedicated wrapper class for joystick for now
            bool m_connected;
        };

        struct WindowMaximized
        {
            bool m_maximized;
        };

        struct WindowScaleChanged
        {
            float m_xScale;
            float m_yScale;
        };

        // Normally I don't want to use macro, but this is a last resort (I hate repeating things)
        // clang-format off
#define GLFW_CPP_EVENT_TYPE_LIST    \
        Empty               , \
        WindowMoved         , \
        WindowResized       , \
        WindowClosed        , \
        WindowRefreshed     , \
        WindowFocused       , \
        WindowIconified     , \
        FramebufferResized  , \
        ButtonPressed       , \
        CursorMoved         , \
        CursorEntered       , \
        Scrolled            , \
        KeyPressed          , \
        CharInput           , \
        MonitorConnected    , \
        FileDropped         , \
        WindowMaximized     , \
        WindowScaleChanged

        // clang-format on

        using Variant = std::variant<GLFW_CPP_EVENT_TYPE_LIST>;

        template <typename T>
        static constexpr bool IsEventType_v = detail::IsAnyOf<T, GLFW_CPP_EVENT_TYPE_LIST>;

#undef GLFW_CPP_EVENT_TYPE_LIST

        Event()
            : m_event{ Empty{} }
        {
        }

        template <typename E>
            requires IsEventType_v<E>
        Event(E&& event)
            : m_event{ std::move(event) }
        {
        }

        // Unbounded visitor, I'm too tired trying to get the bound right...
        decltype(auto) visit(auto&& visitor)
        {
            return std::visit(std::forward<decltype(visitor)>(visitor), m_event);
        }

        // Unbounded visitor, I'm too tired trying to get the bound right...
        decltype(auto) visit(auto&& visitor) const
        {
            return std::visit(std::forward<decltype(visitor)>(visitor), m_event);
        }

        template <typename T>
            requires(not std::is_pointer_v<T>)
        T& get()
        {
            return std::get<T>(m_event);
        }

        template <typename T>
            requires(not std::is_pointer_v<T> && IsEventType_v<T>)
        const T& get() const
        {
            return std::get<T>(m_event);
        }

        template <typename T>
            requires(not std::is_pointer_v<T> && IsEventType_v<T>)
        T* getIf() noexcept
        {
            return std::get_if<T>(&m_event);
        }

        template <typename T>
            requires(not std::is_pointer_v<T> && IsEventType_v<T>)
        const T* getIf() const noexcept
        {
            return std::get_if<T>(&m_event);
        }

        template <typename T>
            requires(not std::is_pointer_v<T> && IsEventType_v<T>)
        void set(T&& event) noexcept
        {
            m_event = std::forward<T>(event);
        }

        template <typename T>
            requires(not std::is_pointer_v<T> && IsEventType_v<T>)
        bool holds() const noexcept
        {
            return std::holds_alternative<T>(m_event);
        }

    private:
        Variant m_event;
    };
}

#endif /* end of include guard: EVENT_HPP_Q439GUKLHFWE */
