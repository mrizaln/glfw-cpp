#ifndef EVENT_HPP_Q439GUKLHFWE
#define EVENT_HPP_Q439GUKLHFWE

#include "glfw_cpp/input.hpp"
#include "glfw_cpp/monitor.hpp"
#include "glfw_cpp/detail/helper.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

namespace glfw_cpp
{
    class Event
    {
    public:
        // visit helper using overloaded lambda
        template <typename... Ts>
        struct Overloaded : Ts...
        {
            using Ts::operator()...;
        };

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
            double m_xDelta;    // delta of current to previous x
            double m_yDelta;    // delta of current to previous y
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

    class EventQueue
    {
    public:
        template <bool isConst = false>
        class Iterator;

        friend class Iterator<false>;
        friend class Iterator<true>;

        enum class ResizePolicy
        {
            DISCARD_OLD,
            DISCARD_NEW,
        };

        static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

        explicit EventQueue(std::size_t capacity) noexcept;

        EventQueue(EventQueue&&) noexcept              = default;
        EventQueue& operator=(EventQueue&&) noexcept   = default;
        EventQueue(const EventQueue& other)            = delete;
        EventQueue& operator=(const EventQueue& other) = delete;

        std::span<const Event> buf() const { return { m_buffer.get(), capacity() }; }

        std::size_t capacity() const noexcept;
        std::size_t size() const noexcept;

        void swap(EventQueue& other) noexcept;
        void reset() noexcept;
        void clear() noexcept;

        Iterator<>           push(Event&& event) noexcept;
        std::optional<Event> pop() noexcept;

        void resize(std::size_t newCapacity, ResizePolicy policy = ResizePolicy::DISCARD_OLD) noexcept;

        Iterator<>     begin() noexcept;
        Iterator<>     end() noexcept;
        Iterator<true> begin() const noexcept;
        Iterator<true> end() const noexcept;
        Iterator<true> cbegin() const noexcept;
        Iterator<true> cend() const noexcept;

    private:
        std::unique_ptr<Event[]> m_buffer   = nullptr;
        std::size_t              m_capacity = 0;
        std::size_t              m_begin    = 0;
        std::size_t              m_end      = 0;
    };

    template <bool IsConst>
    class EventQueue::Iterator
    {
    public:
        // STL compliance
        using iterator_category = std::forward_iterator_tag;
        using value_type        = Event;
        using difference_type   = std::ptrdiff_t;
        using pointer           = Event*;
        using const_pointer     = const Event*;
        using reference         = Event&;
        using const_reference   = const Event&;

        // internal use
        using Buffer = std::conditional_t<IsConst, const EventQueue, EventQueue>;
        using Value  = std::conditional_t<IsConst, const Event, Event>;

        Iterator(Buffer* bufferPtr, std::size_t index)
            : m_bufferPtr{ bufferPtr }
            , m_index{ index }
        {
        }

        // special constructor for const iterator from non-const iterator
        Iterator(Iterator<false>& other)
            requires IsConst
            : m_bufferPtr{ other.m_bufferPtr }
            , m_index{ other.m_index }
        {
        }

        Iterator() noexcept                      = default;
        Iterator(const Iterator&)                = default;
        Iterator& operator=(const Iterator&)     = default;
        Iterator(Iterator&&) noexcept            = default;
        Iterator& operator=(Iterator&&) noexcept = default;

        Iterator& operator++()
        {
            if (m_index == Buffer::npos) {
                return *this;
            }

            if (++m_index == m_bufferPtr->capacity()) {
                m_index = 0;
            }

            if (m_index == m_index_original) {
                m_index = Buffer::npos;
            }

            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Value& operator*() const { return m_bufferPtr->m_buffer[m_index]; };
        Value* operator->() const { return &m_bufferPtr->m_buffer[m_index]; };

        template <bool IsConst2>
        bool operator==(const Iterator<IsConst2>& other) const
        {
            return m_bufferPtr == other.m_bufferPtr && m_index == other.m_index;
        }

        operator std::size_t() { return m_index; };

    private:
        Buffer*     m_bufferPtr = nullptr;
        std::size_t m_index     = npos;

        // workaround for when m_bufferPtr->m_begin == m_bufferPtr->m_end && size() == capacity()
        std::size_t m_index_original = m_index;
    };

    static_assert(std::forward_iterator<EventQueue::Iterator<false>>);
    static_assert(std::forward_iterator<EventQueue::Iterator<true>>);
}

#endif /* end of include guard: EVENT_HPP_Q439GUKLHFWE */
