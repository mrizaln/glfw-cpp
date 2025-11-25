#ifndef EVENT_HPP_Q439GUKLHFWE
#define EVENT_HPP_Q439GUKLHFWE

#include "glfw_cpp/helper.hpp"
#include "glfw_cpp/input.hpp"
#include "glfw_cpp/monitor.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

namespace glfw_cpp
{
    namespace event
    {
        /**
         * @struct Empty
         * @brief Empty event, currently unused
         */
        struct Empty
        {
            constexpr bool operator==(const Empty&) const = default;
        };

        /**
         * @struct WindowMoved
         * @brief Window move event, corresponds to event handled by callback sets by
         * `glfwSetWindowPosCallback`
         */
        struct WindowMoved
        {
            int  x;
            int  y;
            int  dx;
            int  dy;
            auto operator<=>(const WindowMoved&) const = default;
        };

        /**
         * @struct WindowResized
         * @brief Window resize event, corresponds to event handled by callback sets by
         * `glfwSetWindowSizeCallback`
         */
        struct WindowResized
        {
            int  width;
            int  height;
            int  width_change;
            int  height_change;
            auto operator<=>(const WindowResized&) const = default;
        };

        /**
         * @struct WindowClosed
         * @brief Window close event, corresponds to event handled by callback sets by
         * `glfwSetWindowCloseCallback`
         */
        struct WindowClosed
        {
            constexpr bool operator==(const WindowClosed&) const = default;
        };

        /**
         * @struct WindowRefreshed
         * @brief Window refresh event, corresponds to event handled by callback sets by
         * `glfwSetWindowRefreshCallback`
         */
        struct WindowRefreshed
        {
            constexpr bool operator==(const WindowRefreshed&) const = default;
        };

        /**
         * @struct WindowFocused
         * @brief Window focus event, corresponds to event handled by callback sets by
         * `glfwSetWindowFocusCallback`
         */
        struct WindowFocused
        {
            bool focused;
            auto operator<=>(const WindowFocused&) const = default;
        };

        /**
         * @struct WindowIconified
         * @brief Window iconify event, corresponds to event handled by callback sets by
         * `glfwSetWindowIconifyCallback`
         */
        struct WindowIconified
        {
            bool iconified;
            auto operator<=>(const WindowIconified&) const = default;
        };

        /**
         * @struct FramebufferResized
         * @brief Framebuffer resize event, corresponds to event handled by callback sets by
         * `glfwSetFramebufferSizeCallback`
         */
        struct FramebufferResized
        {
            int  width;
            int  height;
            int  width_change;
            int  height_change;
            auto operator<=>(const FramebufferResized&) const = default;
        };

        /**
         * @struct ButtonPressed
         * @brief Mouse button press event, corresponds to event handled by callback sets by
         * `glfwSetMouseButtonCallback`
         */
        struct ButtonPressed
        {
            MouseButton      button;
            MouseButtonState state;
            ModifierKey      mods;
            bool             operator==(const ButtonPressed&) const = default;
        };

        /**
         * @struct CursorMoved
         * @brief Mouse cursor move event, corresponds to event handled by callback sets by
         * `glfwSetCursorPosCallback`
         */
        struct CursorMoved
        {
            double x;
            double y;
            double dx;
            double dy;
            auto   operator<=>(const CursorMoved&) const = default;
        };

        /**
         * @struct CursorEntered
         * @brief Mouse cursor enter event, corresponds to event handled by callback sets by
         * `glfwSetCursorEnterCallback`
         */
        struct CursorEntered
        {
            bool entered;
            auto operator<=>(const CursorEntered&) const = default;
        };

        /**
         * @struct Scrolled
         * @brief Mouse scroll event, corresponds to event handled by callback sets by
         * `glfwSetScrollCallback`
         */
        struct Scrolled
        {
            double dx;
            double dy;
            auto   operator<=>(const Scrolled&) const = default;
        };

        /**
         * @struct KeyPressed
         * @brief Keyboard key press event, corresponds to event handled by callback sets by
         * `glfwSetKeyCallback`
         */
        struct KeyPressed
        {
            KeyCode     key;
            int         scancode;
            KeyState    state;
            ModifierKey mods;
            bool        operator==(const KeyPressed&) const = default;
        };

        /**
         * @struct CharInput
         * @brief Character input event, corresponds to event handled by callback sets by
         * `glfwSetCharCallback`
         */
        struct CharInput
        {
            unsigned int codepoint;
            auto         operator<=>(const CharInput&) const = default;
        };

        /**
         * @struct FileDropped
         * @brief File drop event, corresponds to event handled by callback sets by `glfwSetDropCallback`
         */
        struct FileDropped
        {
            std::vector<std::filesystem::path> files;
            bool                               operator==(const FileDropped&) const = default;
        };

        /**
         * @struct WindowMaximized
         * @brief Window maximize event, corresponds to event handled by callback sets by
         * `glfwSetWindowMaximizeCallback`
         */
        struct WindowMaximized
        {
            bool maximized;
            auto operator<=>(const WindowMaximized&) const = default;
        };

        /**
         * @struct WindowScaleChanged
         * @brief Window scale change event, corresponds to event handled by callback sets by
         * `glfwSetWindowContentScaleCallback`
         */
        struct WindowScaleChanged
        {
            float x;
            float y;
            auto  operator<=>(const WindowScaleChanged&) const = default;
        };

        /**
         * @struct MonitorConnected
         * @brief Monitor connect event, corresponds to event handled by callback sets by
         * `glfwSetMonitorCallback`
         *
         * TODO: implement logic surrounding this event
         */
        struct MonitorConnected
        {
            Monitor monitor;
            bool    connected;
        };

        /**
         * @struct JoystickConnected
         * @brief Joystick connect event, corresponds to event handled by callback sets by
         * `glfwSetJoystickCallback`
         *
         * TODO: implement logic surrounding this event
         */
        struct JoystickConnected
        {
            int  joystick_id;    // No dedicated wrapper class for joystick for now
            bool connected;
        };

        // NOTE: MonitorConnected and JoystickConnected events are not window events but global one, I still
        // don't know how to implement it yet, so I omit them from event list for now
        using Variant = std::variant<
            Empty,
            WindowMoved,
            WindowResized,
            WindowClosed,
            WindowRefreshed,
            WindowFocused,
            WindowIconified,
            FramebufferResized,
            ButtonPressed,
            CursorMoved,
            CursorEntered,
            Scrolled,
            KeyPressed,
            CharInput,
            FileDropped,
            WindowMaximized,
            WindowScaleChanged>;

        /**
         * @brief Check whether a type is part of event types
         */
        template <typename T>
        concept Event = helper::variant::VariantMember<T, Variant>;

        /**
         * @struct Overload
         * @tparam Ts List of types or lambdas that have `operator()` defined for each event variant in
         * `Event::Variant`
         *
         * Helper struct for overloading operator() for multiple event types, can be used to visit `Event`
         * through its `Event::visit` method
         */
        template <typename... Ts>
        struct Overload : Ts...
        {
            using Ts::operator()...;
        };
    }

    /**
     * @class Event
     * @brief Window events wrapper (a variant).
     *
     * Unlike GLFW that uses callback for its event handling, glfw_cpp uses an event queue to store events in
     * each Window.
     */
    struct Event : helper::variant::VariantWrapper<event::Variant>
    {
        using VariantWrapper::VariantWrapper;
    };

    /**
     * @brief Get the name of the event.
     *
     * @param event The event.
     *
     * The returned string is null-terminated.
     */
    template <event::Event E>
    std::string_view event_name() noexcept
    {
        // clang-format off
        if      constexpr (std::same_as<E, event::Empty>)              return "Empty";
        else if constexpr (std::same_as<E, event::WindowMoved>)        return "WindowMoved";
        else if constexpr (std::same_as<E, event::WindowResized>)      return "WindowResized";
        else if constexpr (std::same_as<E, event::WindowClosed>)       return "WindowClosed";
        else if constexpr (std::same_as<E, event::WindowRefreshed>)    return "WindowRefreshed";
        else if constexpr (std::same_as<E, event::WindowFocused>)      return "WindowFocused";
        else if constexpr (std::same_as<E, event::WindowIconified>)    return "WindowIconified";
        else if constexpr (std::same_as<E, event::FramebufferResized>) return "FramebufferResized";
        else if constexpr (std::same_as<E, event::ButtonPressed>)      return "ButtonPressed";
        else if constexpr (std::same_as<E, event::CursorMoved>)        return "CursorMoved";
        else if constexpr (std::same_as<E, event::CursorEntered>)      return "CursorEntered";
        else if constexpr (std::same_as<E, event::Scrolled>)           return "Scrolled";
        else if constexpr (std::same_as<E, event::KeyPressed>)         return "KeyPressed";
        else if constexpr (std::same_as<E, event::CharInput>)          return "CharInput";
        else if constexpr (std::same_as<E, event::FileDropped>)        return "FileDropped";
        else if constexpr (std::same_as<E, event::WindowMaximized>)    return "WindowMaximized";
        else if constexpr (std::same_as<E, event::WindowScaleChanged>) return "WindowScaleChanged";
        else    static_assert(false, "not an event");
        // clang-format on
    }

    /**
     * @brief Get the name of the event.
     *
     * @param event The event.
     *
     * The returned string is null-terminated.
     */
    template <event::Event E>
    std::string_view event_name(const E&) noexcept
    {
        return event_name<E>();
    }

    /**
     * @class IEventInterceptor
     * @brief An abstract class (interface) that is used to create an event interceptor.
     *
     * The class is used to intercept events before they are inserted into each `Window`'s `EventQueue`, and
     * before any `Window::Properties` is updated. The return value of each function determines whether the
     * event should be forwarded to the `Window` or not.
     */
    class IEventInterceptor
    {
    public:
        // allow deletion of derived class through this pointer
        virtual ~IEventInterceptor() = default;

        // clang-format off
        virtual bool on_window_moved        (Window& window, event::WindowMoved&        event) noexcept = 0;
        virtual bool on_window_resized      (Window& window, event::WindowResized&      event) noexcept = 0;
        virtual bool on_window_closed       (Window& window, event::WindowClosed&       event) noexcept = 0;
        virtual bool on_window_refreshed    (Window& window, event::WindowRefreshed&    event) noexcept = 0;
        virtual bool on_window_focused      (Window& window, event::WindowFocused&      event) noexcept = 0;
        virtual bool on_window_iconified    (Window& window, event::WindowIconified&    event) noexcept = 0;
        virtual bool on_window_maximized    (Window& window, event::WindowMaximized&    event) noexcept = 0;
        virtual bool on_window_scale_changed(Window& window, event::WindowScaleChanged& event) noexcept = 0;
        virtual bool on_framebuffer_resized (Window& window, event::FramebufferResized& event) noexcept = 0;
        virtual bool on_button_pressed      (Window& window, event::ButtonPressed&      event) noexcept = 0;
        virtual bool on_cursor_moved        (Window& window, event::CursorMoved&        event) noexcept = 0;
        virtual bool on_cursor_entered      (Window& window, event::CursorEntered&      event) noexcept = 0;
        virtual bool on_scrolled            (Window& window, event::Scrolled&           event) noexcept = 0;
        virtual bool on_key_pressed         (Window& window, event::KeyPressed&         event) noexcept = 0;
        virtual bool on_char_input          (Window& window, event::CharInput&          event) noexcept = 0;
        virtual bool on_file_dropped        (Window& window, event::FileDropped&        event) noexcept = 0;
        // clang-format on
    };

    /**
     * @class DefaultEventInterceptor
     * @brief Default implementation of `IEventInterceptor` that does nothing.
     *
     * This class can be used as a base class for custom event interceptors. You can override only the
     * functions you need to intercept. The return value of each function determines whether the event should
     * be forwarded to the `Window` or not.
     */
    class DefaultEventInterceptor : public IEventInterceptor
    {
    public:
        // clang-format off
        bool on_window_moved        (Window&, event::WindowMoved&)        noexcept override { return true; }
        bool on_window_resized      (Window&, event::WindowResized&)      noexcept override { return true; }
        bool on_window_closed       (Window&, event::WindowClosed&)       noexcept override { return true; }
        bool on_window_refreshed    (Window&, event::WindowRefreshed&)    noexcept override { return true; }
        bool on_window_focused      (Window&, event::WindowFocused&)      noexcept override { return true; }
        bool on_window_iconified    (Window&, event::WindowIconified&)    noexcept override { return true; }
        bool on_window_maximized    (Window&, event::WindowMaximized&)    noexcept override { return true; }
        bool on_window_scale_changed(Window&, event::WindowScaleChanged&) noexcept override { return true; }
        bool on_framebuffer_resized (Window&, event::FramebufferResized&) noexcept override { return true; }
        bool on_button_pressed      (Window&, event::ButtonPressed&)      noexcept override { return true; }
        bool on_cursor_moved        (Window&, event::CursorMoved&)        noexcept override { return true; }
        bool on_cursor_entered      (Window&, event::CursorEntered&)      noexcept override { return true; }
        bool on_scrolled            (Window&, event::Scrolled&)           noexcept override { return true; }
        bool on_key_pressed         (Window&, event::KeyPressed&)         noexcept override { return true; }
        bool on_char_input          (Window&, event::CharInput&)          noexcept override { return true; }
        bool on_file_dropped        (Window&, event::FileDropped&)        noexcept override { return true; }
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
        template <bool IsConst = false>
        class Iterator;

        friend class Iterator<false>;
        friend class Iterator<true>;

        enum class ResizePolicy
        {
            DiscardOld,
            DiscardNew,
        };

        static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

        explicit EventQueue(std::size_t capacity) noexcept;

        EventQueue(EventQueue&&) noexcept              = default;
        EventQueue& operator=(EventQueue&&) noexcept   = default;
        EventQueue(const EventQueue& other)            = delete;
        EventQueue& operator=(const EventQueue& other) = delete;

        /**
         * @brief Visit all events.
         *
         * @param visitor The visitor.
         * @throw <exception> If the visitor throws an exception, the exception is propagated
         *
         * This function is functionally the same as calling `Event::visit` on each event in the queue.
         *
         * ```cpp
         * for (const auto& event : queue) { event.visit(visitor); }
         * ```
         *
         * Notice that this function returns nothing and will iterate all the events in the queue. If you
         * want to break from the loop or want the visitor to return something, then you better off writing
         * the loop yourself and use `Event::visit` traditionally.
         */
        template <helper::variant::ConstVisitorComplete<event::Variant> T>
        void visit(T&& visitor) const;

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
         * @param new_capacity New capacity of the queue
         * @param policy Resize policy to use
         */
        void resize(std::size_t new_capacity, ResizePolicy policy = ResizePolicy::DiscardOld) noexcept;

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

        Iterator(Buffer* buffer_ptr, std::size_t index) noexcept
            : m_buffer_ptr{ buffer_ptr }
            , m_index{ index }
        {
        }

        // special constructor for const iterator from non-const iterator
        Iterator(Iterator<false>& other) noexcept
            requires IsConst
            : m_buffer_ptr{ other.m_buffer_ptr }
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

            if (++m_index == m_buffer_ptr->capacity()) {
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

        Value& operator*() const noexcept { return m_buffer_ptr->m_buffer[m_index]; };
        Value* operator->() const noexcept { return &m_buffer_ptr->m_buffer[m_index]; };

        template <bool IsConst2>
        bool operator==(const Iterator<IsConst2>& other) const noexcept
        {
            return m_buffer_ptr == other.m_buffer_ptr && m_index == other.m_index;
        }

        operator std::size_t() noexcept { return m_index; };

    private:
        Buffer*     m_buffer_ptr = nullptr;
        std::size_t m_index      = npos;

        // workaround for when m_buffer_ptr->m_begin == m_buffer_ptr->m_end && size() == capacity()
        std::size_t m_index_original = m_index;
    };

    static_assert(std::forward_iterator<EventQueue::Iterator<false>>);
    static_assert(std::forward_iterator<EventQueue::Iterator<true>>);

    // NOTE: the visit function must be defined in the header since it's a template
    template <helper::variant::ConstVisitorComplete<event::Variant> T>
    void EventQueue::visit(T&& visitor) const
    {
        for (const auto& event : *this) {
            event.visit(visitor);
        }
    }
}

#endif /* end of include guard: EVENT_HPP_Q439GUKLHFWE */
