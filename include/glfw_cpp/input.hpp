#ifndef INPUT_HPP_354TKEJR8H
#define INPUT_HPP_354TKEJR8H

#if defined(_WIN32)
#    undef DELETE;
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <string_view>
#include <utility>
#include <vector>

namespace glfw_cpp
{
    // enum definitions replacing macros, reference from here:
    // https://www.glfw.org/docs/latest/group__keys.html
    enum class KeyCode : int
    {
        // clang-format off
        UNKNOWN         = -1,
        SPACE           = 32,
        APOSTROPHE      = 39,
        COMMA           = 44,
        MINUS,
        PERIOD,
        SLASH,
        ZERO,
        ONE,
        TWO,
        THREE,
        FOUR,
        FIVE,
        SIX,
        SEVEN,
        EIGHT,
        NINE,
        SEMICOLON       = 59,
        EQUAL           = 61,
        A               = 65,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        LEFT_BRACKET,
        BACKSLASH,
        RIGHT_BRACKET,
        GRAVE_ACCENT    = 96,
        WORLD_1         = 161,
        WORLD_2,
        ESCAPE          = 256,
        ENTER,
        TAB,
        BACKSPACE,
        INSERT,
        DELETE,
        RIGHT,
        LEFT,
        DOWN,
        UP,
        PAGE_UP,
        PAGE_DOWN,
        HOME,
        END,
        CAPS_LOCK       = 280,
        SCROLL_LOCK,
        NUM_LOCK,
        PRINT_SCREEN,
        PAUSE,
        F1              = 290,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,
        KEYPAD_0        = 320,
        KEYPAD_1,
        KEYPAD_2,
        KEYPAD_3,
        KEYPAD_4,
        KEYPAD_5,
        KEYPAD_6,
        KEYPAD_7,
        KEYPAD_8,
        KEYPAD_9,
        KEYPAD_DECIMAL,
        KEYPAD_DIVIDE,
        KEYPAD_MULTIPLY,
        KEYPAD_SUBTRACT,
        KEYPAD_ADD,
        KEYPAD_ENTER,
        KEYPAD_EQUAL,
        LEFT_SHIFT      = 340,
        LEFT_CONTROL,
        LEFT_ALT,
        LEFT_SUPER,
        RIGHT_SHIFT,
        RIGHT_CONTROL,
        RIGHT_ALT,
        RIGHT_SUPER,
        MENU,
        MAXVALUE        = MENU,
        // clang-format on
    };

    // enum definitions replacing macros, reference from here:
    // https://www.glfw.org/docs/latest/group__input.html
    enum class KeyState : int
    {
        RELEASE = 0,
        PRESS   = 1,
        REPEAT  = 2
    };

    // enum definitions replacing macros, reference from here:
    // https://www.glfw.org/docs/latest/group__buttons.html
    enum class MouseButton : int
    {
        // clang-format off
        ONE         = 0,
        TWO,
        THREE,
        FOUR,
        FIVE,
        SIX,
        SEVEN,
        EIGHT,
        LEFT        = ONE,
        RIGHT       = TWO,
        MIDDLE      = THREE,
        MAXVALUE    = EIGHT,
        // clang-format on
    };

    enum class MouseButtonState : int
    {
        RELEASE = 0,
        PRESS   = 1,
    };

    // enum definitions replacing macros, reference from here:
    // https://www.glfw.org/docs/latest/group__mods.html
    class ModifierKey
    {
    public:
        using Base = int;

        enum Bit : Base
        {
            NONE      = 0,
            SHIFT     = 0x01,
            CONTROL   = 0x02,
            ALT       = 0x04,
            SUPER     = 0x08,
            CAPS_LOCK = 0x10,
            NUM_LOCK  = 0x20,
        };

        ModifierKey(std::convertible_to<Bit> auto... e)
            : m_mods{ static_cast<Base>((0 | ... | e)) }
        {
        }

        ModifierKey(Base mods)
            : m_mods{ mods }
        {
        }

        auto& set(std::convertible_to<Bit> auto... e)
            requires(sizeof...(e) > 0)
        {
            m_mods |= (NONE | ... | e);
            return *this;
        }

        auto& unset(std::convertible_to<Bit> auto... e)
            requires(sizeof...(e) > 0)
        {
            m_mods &= ~(NONE | ... | e);
            return *this;
        }

        auto& toggle(std::convertible_to<Bit> auto... e)
            requires(sizeof...(e) > 0)
        {
            m_mods ^= (NONE | ... | e);
            return *this;
        }

        auto& setToValue(bool value, std::convertible_to<Bit> auto... e)
            requires(sizeof...(e) > 0)
        {
            if (value) {
                return set(std::forward<Bit>(e)...);
            } else {
                return unset(std::forward<Bit>(e)...);
            }
        }

        bool test(std::convertible_to<Bit> auto... e) const
            requires(sizeof...(e) > 0)
        {
            auto flags{ static_cast<Base>((NONE | ... | e)) };
            if (flags == NONE) {
                return m_mods == NONE;
            } else {
                return (m_mods & flags) == flags;
            }
        }

        bool testSome(std::convertible_to<Bit> auto... e) const
            requires(sizeof...(e) > 0)
        {
            auto flags{ static_cast<Base>((NONE | ... | e)) };
            if (flags == NONE) {
                return test(static_cast<Bit>(flags));
            } else {
                return (m_mods & flags) != NONE;
            }
        }

        auto& reset()
        {
            m_mods = NONE;
            return *this;
        }

        explicit operator int() const { return static_cast<Base>(m_mods); }

    private:
        Base m_mods = 0;
    };

    class Window;

    class KeyStateRecord
    {
    public:
        friend Window;

        KeyStateRecord() = default;

        bool isPressed(KeyCode keyCode) const { return getBit(bitPos(keyCode)); }

        bool allPressed(std::initializer_list<KeyCode> keyCodes) const
        {
            return std::all_of(keyCodes.begin(), keyCodes.end(), [this](auto k) { return isPressed(k); });
        }

        bool anyPressed(std::initializer_list<KeyCode> keyCodes) const
        {
            return std::any_of(keyCodes.begin(), keyCodes.end(), [this](auto k) { return isPressed(k); });
        }

        std::vector<KeyCode> pressedKeys() const;
        std::vector<KeyCode> releasedKeys() const;

    private:
        using Element = std::uint64_t;
        using State   = std::array<Element, 2>;

        // for friends
        void setValue(KeyCode keyCode, bool value) { setBit(bitPos(keyCode), value); }
        void set(KeyCode keyCode) { setValue(keyCode, true); }
        void unset(KeyCode keyCode) { setValue(keyCode, false); }
        void clear() { m_state.fill(0); };

        // impl detail
        std::size_t bitPos(KeyCode keyCode) const;
        void        setBit(std::size_t pos, bool value);
        bool        getBit(std::size_t pos) const;

        State m_state = {};
    };

    class MouseButtonStateRecord
    {
    public:
        friend Window;
        bool isPressed(MouseButton button) const { return getBit(bitPos(button)); }

        bool allPressed(std::initializer_list<MouseButton> buttons) const
        {
            return std::all_of(buttons.begin(), buttons.end(), [this](auto k) { return isPressed(k); });
        }

        bool anyPressed(std::initializer_list<MouseButton> buttons) const
        {
            return std::any_of(buttons.begin(), buttons.end(), [this](auto k) { return isPressed(k); });
        }

        std::vector<MouseButton> pressedButtons() const;
        std::vector<MouseButton> releasedButtons() const;

    private:
        using State = std::uint8_t;

        // for friends
        void setValue(MouseButton button, bool value) { setBit(bitPos(button), value); }
        void set(MouseButton button) { setValue(button, true); }
        void unset(MouseButton button) { setValue(button, false); }
        void clear() { m_state = 0; };

        // impl detail
        std::size_t bitPos(MouseButton button) const;
        void        setBit(std::size_t pos, bool value);
        bool        getBit(std::size_t pos) const;

        State m_state = {};
    };

    template <typename T>
        requires std::same_as<T, KeyCode>             //
              or std::same_as<T, KeyState>            //
              or std::same_as<T, MouseButton>         //
              or std::same_as<T, MouseButtonState>    //
              or std::same_as<T, ModifierKey>         //
              or std::same_as<T, ModifierKey::Bit>
    int underlying(const T& value)
    {
        return static_cast<int>(value);
    }

    // string must be null-terminated
    void setClipboardString(const char* string);

    std::string_view getClipboardString();
    double           getTime();
    void             setTime(double time);
    uint64_t         getTimerValue();
    uint64_t         getTimerFrequency();
}

#endif /* end of include guard: INPUT_HPP_354TKEJR8H */
