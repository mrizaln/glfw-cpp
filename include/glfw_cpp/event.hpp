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
    /**
     * @class Event
     * @brief Window events wrapper (a variant).
     *
     * Unlike GLFW that uses callback for its event handling, glfw_cpp uses an event queue to store events in
     * each Window.
     */
    class Event
    {
    public:
        /**
         * @struct Template struct for overloading operator() for multiple event types, can be used with to
         * visit `Event` throught its `Event::visit` method
         *
         * @tparam Ts List of types or lambdas that have `operator()` defined for each event variant in
         * `Event::Variant`
         */
        template <typename... Ts>
        struct Overloaded : Ts...
        {
            using Ts::operator()...;
        };

        /**
         * @struct Empty
         * @brief Empty event, currently unused
         */
        struct Empty
        {
        };

        /**
         * @struct WindowMoved
         * @brief Window move event, corresponds to event handled by callback sets by
         * `glfwSetWindowPosCallback`
         */
        struct WindowMoved
        {
            int m_xPos;
            int m_yPos;
        };

        /**
         * @struct WindowResized
         * @brief Window resize event, corresponds to event handled by callback sets by
         * `glfwSetWindowSizeCallback`
         */
        struct WindowResized
        {
            int m_width;
            int m_height;
        };

        /**
         * @struct WindowClosed
         * @brief Window close event, corresponds to event handled by callback sets by
         * `glfwSetWindowCloseCallback`
         */
        struct WindowClosed
        {
        };

        /**
         * @struct WindowRefreshed
         * @brief Window refresh event, corresponds to event handled by callback sets by
         * `glfwSetWindowRefreshCallback`
         */
        struct WindowRefreshed
        {
        };

        /**
         * @struct WindowFocused
         * @brief Window focus event, corresponds to event handled by callback sets by
         * `glfwSetWindowFocusCallback`
         */
        struct WindowFocused
        {
            bool m_focused;
        };

        /**
         * @struct WindowIconified
         * @brief Window iconify event, corresponds to event handled by callback sets by
         * `glfwSetWindowIconifyCallback`
         */
        struct WindowIconified
        {
            bool m_iconified;
        };

        /**
         * @struct FramebufferResized
         * @brief Framebuffer resize event, corresponds to event handled by callback sets by
         * `glfwSetFramebufferSizeCallback`
         */
        struct FramebufferResized
        {
            int m_width;
            int m_height;
        };

        /**
         * @struct ButtonPressed
         * @brief Mouse button press event, corresponds to event handled by callback sets by
         * `glfwSetMouseButtonCallback`
         */
        struct ButtonPressed
        {
            MouseButton      m_button;
            MouseButtonState m_state;
            ModifierKey      m_mods;
        };

        /**
         * @struct CursorMoved
         * @brief Mouse cursor move event, corresponds to event handled by callback sets by
         * `glfwSetCursorPosCallback`
         */
        struct CursorMoved
        {
            double m_xPos;
            double m_yPos;
            double m_xDelta;    // delta of current to previous x
            double m_yDelta;    // delta of current to previous y
        };

        /**
         * @struct CursorEntered
         * @brief Mouse cursor enter event, corresponds to event handled by callback sets by
         * `glfwSetCursorEnterCallback`
         */
        struct CursorEntered
        {
            bool m_entered;
        };

        /**
         * @struct Scrolled
         * @brief Mouse scroll event, corresponds to event handled by callback sets by
         * `glfwSetScrollCallback`
         */
        struct Scrolled
        {
            double m_xOffset;
            double m_yOffset;
        };

        /**
         * @struct KeyPressed
         * @brief Keyboard key press event, corresponds to event handled by callback sets by
         * `glfwSetKeyCallback`
         */
        struct KeyPressed
        {
            KeyCode     m_key;
            int         m_scancode;
            KeyState    m_state;
            ModifierKey m_mods;
        };

        /**
         * @struct CharInput
         * @brief Character input event, corresponds to event handled by callback sets by
         * `glfwSetCharCallback`
         */
        struct CharInput
        {
            unsigned int m_codepoint;
        };

        /**
         * @struct FileDropped
         * @brief File drop event, corresponds to event handled by callback sets by `glfwSetDropCallback`
         */
        struct FileDropped
        {
            std::vector<std::filesystem::path> m_files;
        };

        /**
         * @struct WindowMaximized
         * @brief Window maximize event, corresponds to event handled by callback sets by
         * `glfwSetWindowMaximizeCallback`
         */
        struct WindowMaximized
        {
            bool m_maximized;
        };

        /**
         * @struct WindowScaleChanged
         * @brief Window scale change event, corresponds to event handled by callback sets by
         * `glfwSetWindowContentScaleCallback`
         */
        struct WindowScaleChanged
        {
            float m_xScale;
            float m_yScale;
        };

        /**
         * @struct MonitorConnected
         * @brief Monitor connect event, corresponds to event handled by callback sets by
         * `glfwSetMonitorCallback` (not implemented yet)
         */
        struct MonitorConnected
        {
            Monitor m_monitor;
            bool    m_connected;
        };

        /**
         * @struct JoystickConnected
         * @brief Joystick connect event, corresponds to event handled by callback sets by
         * `glfwSetJoystickCallback` (not implemented yet)
         */
        struct JoystickConnected
        {
            int  m_joystickId;    // No dedicated wrapper class for joystick for now
            bool m_connected;
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
        FileDropped         , \
        WindowMaximized     , \
        WindowScaleChanged

        // clang-format on

        using Variant = std::variant<GLFW_CPP_EVENT_TYPE_LIST>;

        template <typename T>
        static constexpr bool IsEventType_v = detail::IsAnyOf<T, GLFW_CPP_EVENT_TYPE_LIST>;

#undef GLFW_CPP_EVENT_TYPE_LIST

        Event() noexcept
            : m_event{ Empty{} }
        {
        }

        template <typename E>
            requires IsEventType_v<E>
        Event(E&& event) noexcept
            : m_event{ std::move(event) }
        {
        }

        /**
         * @brief Visit the event with the given visitor
         *
         * @param visitor Visitor to visit the event with (must comply with `std::visit` requirements)
         * @return The return value of the visitor
         *
         * @throw <exception> If the visitor throws an exception, the exception is propagated
         */
        decltype(auto) visit(auto&& visitor)
        {
            // Unbounded visitor, I'm too tired trying to get the bound right...
            return std::visit(std::forward<decltype(visitor)>(visitor), m_event);
        }

        /**
         * @brief Visit the event with the given visitor
         *
         * @param visitor Visitor to visit the event with (must comply with `std::visit` requirements)
         * @return The return value of the visitor
         *
         * @throw <exception> If the visitor throws an exception, the exception is propagated
         */
        decltype(auto) visit(auto&& visitor) const
        {
            // Unbounded visitor, I'm too tired trying to get the bound right...
            return std::visit(std::forward<decltype(visitor)>(visitor), m_event);
        }

        /**
         * @brief Get the event as the specified type
         *
         * @tparam T Type of the event to get
         * @return Reference to the event
         *
         * @throw std::bad_variant_access If the event is not of the specified type
         */
        template <typename T>
            requires(not std::is_pointer_v<T>) and IsEventType_v<T>
        T& get()
        {
            return std::get<T>(m_event);
        }

        /**
         * @brief Get the event as the specified type
         *
         * @tparam T Type of the event to get
         * @return Reference to the event
         *
         * @throw std::bad_variant_access If the event is not of the specified type
         */
        template <typename T>
            requires(not std::is_pointer_v<T> and IsEventType_v<T>)
        const T& get() const
        {
            return std::get<T>(m_event);
        }

        /**
         * @brief Get the event as the specified type
         *
         * @tparam T Type of the event to get
         * @return Pointer to the event, or nullptr if the event is not of the specified type
         */
        template <typename T>
            requires(not std::is_pointer_v<T> and IsEventType_v<T>)
        T* getIf() noexcept
        {
            return std::get_if<T>(&m_event);
        }

        /**
         * @brief Get the event as the specified type
         *
         * @tparam T Type of the event to get
         * @return Pointer to the event, or nullptr if the event is not of the specified type
         */
        template <typename T>
            requires(not std::is_pointer_v<T> and IsEventType_v<T>)
        const T* getIf() const noexcept
        {
            return std::get_if<T>(&m_event);
        }

        /**
         * @brief Set the event to the specified value
         *
         * @tparam T Type of the event to set
         * @param event Event to set
         */
        template <typename T>
            requires(not std::is_pointer_v<T> and IsEventType_v<T>)
        void set(T&& event) noexcept
        {
            m_event = std::forward<T>(event);
        }

        /**
         * @brief Check if the event holds the specified type
         *
         * @tparam T Type to check
         * @return true If the event holds the specified type
         */
        template <typename T>
            requires(not std::is_pointer_v<T> and IsEventType_v<T>)
        bool holds() const noexcept
        {
            return std::holds_alternative<T>(m_event);
        }

    private:
        Variant m_event;
    };

    /**
     * @class IEventInterceptor
     * @brief An abstract class (interface) that is used to create an event interceptor.
     *
     * The class is used to intercept events before they are inserted into each `Window`'s `EventQueue`, and
     * before any `Window::Properties` is updated. The return value of each function determines whether the
     * event should be forwarded to the `Window` or not.
     */
    struct IEventInterceptor
    {
    public:
        // clang-format off
        virtual bool onWindowMoved       (Window& window, Event::WindowMoved&        event) noexcept = 0;
        virtual bool onWindowResized     (Window& window, Event::WindowResized&      event) noexcept = 0;
        virtual bool onWindowClosed      (Window& window, Event::WindowClosed&       event) noexcept = 0;
        virtual bool onWindowRefreshed   (Window& window, Event::WindowRefreshed&    event) noexcept = 0;
        virtual bool onWindowFocused     (Window& window, Event::WindowFocused&      event) noexcept = 0;
        virtual bool onWindowIconified   (Window& window, Event::WindowIconified&    event) noexcept = 0;
        virtual bool onWindowMaximized   (Window& window, Event::WindowMaximized&    event) noexcept = 0;
        virtual bool onWindowScaleChanged(Window& window, Event::WindowScaleChanged& event) noexcept = 0;
        virtual bool onFramebufferResized(Window& window, Event::FramebufferResized& event) noexcept = 0;
        virtual bool onButtonPressed     (Window& window, Event::ButtonPressed&      event) noexcept = 0;
        virtual bool onCursorMoved       (Window& window, Event::CursorMoved&        event) noexcept = 0;
        virtual bool onCursorEntered     (Window& window, Event::CursorEntered&      event) noexcept = 0;
        virtual bool onScrolled          (Window& window, Event::Scrolled&           event) noexcept = 0;
        virtual bool onKeyPressed        (Window& window, Event::KeyPressed&         event) noexcept = 0;
        virtual bool onCharInput         (Window& window, Event::CharInput&          event) noexcept = 0;
        virtual bool onFileDropped       (Window& window, Event::FileDropped&        event) noexcept = 0;
        // clang-format on

    protected:
        // destructor is protected to prevent deletion through this interface
        ~IEventInterceptor() = default;
    };

    /**
     * @class DefaultEventInterceptor
     * @brief Default implementation of `IEventInterceptor` that does nothing.
     *
     * This class can be used as a base class for custom event interceptors. You can override only the
     * functions you need to intercept.
     */
    struct DefaultEventInterceptor : public IEventInterceptor
    {
        // clang-format off
        virtual bool onWindowMoved       (Window&, Event::WindowMoved&)        noexcept { return true; }
        virtual bool onWindowResized     (Window&, Event::WindowResized&)      noexcept { return true; }
        virtual bool onWindowClosed      (Window&, Event::WindowClosed&)       noexcept { return true; }
        virtual bool onWindowRefreshed   (Window&, Event::WindowRefreshed&)    noexcept { return true; }
        virtual bool onWindowFocused     (Window&, Event::WindowFocused&)      noexcept { return true; }
        virtual bool onWindowIconified   (Window&, Event::WindowIconified&)    noexcept { return true; }
        virtual bool onWindowMaximized   (Window&, Event::WindowMaximized&)    noexcept { return true; }
        virtual bool onWindowScaleChanged(Window&, Event::WindowScaleChanged&) noexcept { return true; }
        virtual bool onFramebufferResized(Window&, Event::FramebufferResized&) noexcept { return true; }
        virtual bool onButtonPressed     (Window&, Event::ButtonPressed&)      noexcept { return true; }
        virtual bool onCursorMoved       (Window&, Event::CursorMoved&)        noexcept { return true; }
        virtual bool onCursorEntered     (Window&, Event::CursorEntered&)      noexcept { return true; }
        virtual bool onScrolled          (Window&, Event::Scrolled&)           noexcept { return true; }
        virtual bool onKeyPressed        (Window&, Event::KeyPressed&)         noexcept { return true; }
        virtual bool onCharInput         (Window&, Event::CharInput&)          noexcept { return true; }
        virtual bool onFileDropped       (Window&, Event::FileDropped&)        noexcept { return true; }
        // clang-format on
    };

    /**
     * @class EventQueue
     * @brief A simple event queue that stores events in a circular buffer. The queue is used to store events
     * in each `Window` instance. The queue has a fixed capacity that can be resized at runtime.
     */
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

        /**
         * @brief Get a view of the underlying buffer as a `std::span`
         */
        std::span<const Event> buf() const noexcept { return { m_buffer.get(), capacity() }; }

        /**
         * @brief Get the capacity of the queue
         */
        std::size_t capacity() const noexcept;

        /**
         * @brief Get the size of the queue
         */
        std::size_t size() const noexcept;

        /**
         * @brief Check if the queue is empty
         */
        bool empty() const noexcept;

        /**
         * @brief Check if the queue is full
         */
        bool full() const noexcept;

        /**
         * @brief Swap the contents of this queue with another queue
         *
         * @param other Queue to swap with
         */
        void swap(EventQueue& other) noexcept;

        /**
         * @brief Reset the internal indices of the queue to the initial state without clearing the buffer
         */
        void reset() noexcept;

        /**
         * @brief Clear the queue by resetting the internal indices and filling the buffer with empty events
         */
        void clear() noexcept;

        /**
         * @brief Push an event to the queue
         *
         * @param event Event to push
         * @return Iterator to the pushed event
         */
        Iterator<> push(Event&& event) noexcept;

        /**
         * @brief Pop an event from the queue
         *
         * @return The popped event, or std::nullopt if the queue is empty
         */
        std::optional<Event> pop() noexcept;

        /**
         * @brief Resize the queue to the specified capacity
         *
         * @param newCapacity New capacity of the queue
         * @param policy Resize policy to use
         */
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

        Iterator(Buffer* bufferPtr, std::size_t index) noexcept
            : m_bufferPtr{ bufferPtr }
            , m_index{ index }
        {
        }

        // special constructor for const iterator from non-const iterator
        Iterator(Iterator<false>& other) noexcept
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

        Iterator& operator++() noexcept
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

        Iterator operator++(int) noexcept
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Value& operator*() const noexcept { return m_bufferPtr->m_buffer[m_index]; };
        Value* operator->() const noexcept { return &m_bufferPtr->m_buffer[m_index]; };

        template <bool IsConst2>
        bool operator==(const Iterator<IsConst2>& other) const noexcept
        {
            return m_bufferPtr == other.m_bufferPtr && m_index == other.m_index;
        }

        operator std::size_t() noexcept { return m_index; };

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
