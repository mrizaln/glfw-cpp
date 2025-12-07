#ifndef GLFW_CPP_INPUT_HPP
#define GLFW_CPP_INPUT_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <vector>

namespace glfw_cpp
{
    /**
     * @enum KeyCode
     * @brief Key codes for keyboard input.
     *
     * This enum is a replacement of macros used in GLFW C header file. The integer values are directly
     * translated from the source [here](https://www.glfw.org/docs/3.4/group__keys.html)
     */
    enum class KeyCode : int
    {
        // clang-format off
        Unknown         = -1,
        Space           = 32,
        Apostrophe      = 39,
        Comma           = 44,
        Minus,
        Period,
        Slash,
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine,
        Semicolon       = 59,
        Equal           = 61,
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
        LeftBracket,
        BackSlash,
        RightBracket,
        GraveAccent     = 96,
        World1          = 161,
        World2,
        Escape          = 256,
        Enter,
        Tab,
        Backspace,
        Insert,
        Delete,
        Right,
        Left,
        Down,
        Up,
        PageUp,
        PageDown,
        Home,
        End,
        CapsLock        = 280,
        ScrollLock,
        NumLock,
        PrintScreen,
        Pause,
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
        Keypad0         = 320,
        Keypad1,
        Keypad2,
        Keypad3,
        Keypad4,
        Keypad5,
        Keypad6,
        Keypad7,
        Keypad8,
        Keypad9,
        KeypadDecimal,
        KeypadDivide,
        KeypadMultiply,
        KeypadSubtract,
        KeypadAdd,
        KeypadEnter,
        KeypadEqual,
        LeftShift       = 340,
        LeftControl,
        LeftAlt,
        LeftSuper,
        RightShift,
        RightControl,
        RightAlt,
        RightSuper,
        Menu,
        MaxValue        = Menu,
        // clang-format on
    };

    /**
     * @enum KeyState
     * @brief Key states for keyboard input.
     *
     * This enum is a replacement of macros used in GLFW C header file. The integer values are directly
     * translated from the source [here](https://www.glfw.org/docs/3.4/group__input.html)
     */
    enum class KeyState : int
    {
        Release = 0,
        Press   = 1,
        Repeat  = 2
    };

    /**
     * @enum MouseButton
     * @brief Mouse button codes for mouse input.
     *
     * This enum is a replacement of macros used in GLFW C header file. The integer values are directly
     * translated from the source [here](https://www.glfw.org/docs/3.4/group__buttons.html)
     */
    enum class MouseButton : int
    {
        // clang-format off
        One         = 0,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,

        Left        = One,
        Right       = Two,
        Middle      = Three,

        MaxValue    = Eight,
        // clang-format on
    };

    /**
     * @enum MouseButtonState
     * @brief Mouse button states for mouse input.
     *
     * This enum is a replacement of macros used in GLFW C header file. The integer values are directly
     * translated from the source [here](https://www.glfw.org/docs/3.4/group__input.html)
     */
    enum class MouseButtonState : int
    {
        Release = 0,
        Press   = 1,
    };

    /**
     * @class ModifierKey
     * @brief Modifier key codes for keyboard input.
     *
     * This class is a replacement of macros used in GLFW C header file. The integer values are directly
     * translated from the source [here](https://www.glfw.org/docs/3.4/group__mods.html)
     */
    class ModifierKey
    {
    public:
        using Base = int;

        /**
         * @enum Bit
         * @brief Modifier key codes.
         *
         * This enum is a replacement of macros used in GLFW C header file. The integer values are directly
         * translated from the source [here](https://www.glfw.org/docs/3.4/group__mods.html)
         */
        enum Bit : Base
        {
            Shift    = 0x01,
            Control  = 0x02,
            Alt      = 0x04,
            Super    = 0x08,
            CapsLock = 0x10,
            NumLock  = 0x20,
        };

        ModifierKey() = default;

        /**
         * @brief Construct a new Modifier Key object from a list of modifier keys.
         * @param mods A list of modifier keys to set.
         */
        ModifierKey(std::initializer_list<Bit> mods) noexcept
            : m_mods{ 0 }
        {
            for (auto mod : mods) {
                m_mods |= mod;
            }
        }

        /**
         * @brief Construct a new Modifier Key object from a span of modifier keys.
         * @param mods A span of modifier keys to set.
         */
        ModifierKey(std::span<const Bit> mods) noexcept
            : m_mods{ 0 }
        {
            for (auto mod : mods) {
                m_mods |= mod;
            }
        }

        /**
         * @brief Construct a new Modifier Key object from an underlying integer value.
         * @param mods The underlying integer value to set.
         *
         * This constructor will ignore any invalid bits from the value.
         */
        explicit ModifierKey(Base mods) noexcept
            : m_mods{ mods }
        {
            // remove any invalid bits
            constexpr auto valid  = Shift | Control | Alt | Super | CapsLock | NumLock;
            m_mods               &= valid;
        }

        /**
         * @brief Set the modifier key(s).
         * @param mods The modifier key(s) to set.
         */
        auto& set(std::initializer_list<Bit> mods) noexcept
        {
            for (auto mod : mods) {
                m_mods |= mod;
            }
            return *this;
        }

        /**
         * @brief Set the modifier key(s).
         * @param mods The modifier key(s) to set.
         */
        auto& set(std::span<const Bit> mods) noexcept
        {
            for (auto mod : mods) {
                m_mods |= mod;
            }
            return *this;
        }

        /**
         * @brief Unset the modifier key(s).
         * @param mods The modifier key(s) to unset.
         */
        auto& unset(std::initializer_list<Bit> mods) noexcept
        {
            for (auto mod : mods) {
                m_mods &= ~mod;
            }
            return *this;
        }

        /**
         * @brief Unset the modifier key(s).
         * @param mods The modifier key(s) to unset.
         */
        auto& unset(std::span<const Bit> mods) noexcept
        {
            for (auto mod : mods) {
                m_mods &= ~mod;
            }
            return *this;
        }

        /**
         * @brief Toggle the modifier key(s).
         * @param mods The modifier key(s) to toggle.
         */
        auto& toggle(std::initializer_list<Bit> mods) noexcept
        {
            for (auto mod : mods) {
                m_mods ^= mod;
            }
            return *this;
        }

        /**
         * @brief Toggle the modifier key(s).
         * @param mods The modifier key(s) to toggle.
         */
        auto& toggle(std::span<const Bit> mods) noexcept
        {
            for (auto mod : mods) {
                m_mods ^= mod;
            }
            return *this;
        }

        /**
         * @brief Set the modifier key(s) to a specific value.
         * @param value The value to set the modifier key(s) to.
         * @param mods The modifier key(s) to be set.
         */
        auto& set_to_value(bool value, std::initializer_list<Bit> mods) noexcept
        {
            if (value) {
                return set(mods);
            } else {
                return unset(mods);
            }
        }

        /**
         * @brief Set the modifier key(s) to a specific value.
         * @param value The value to set the modifier key(s) to.
         * @param mods The modifier key(s) to be set.
         */
        auto& set_to_value(bool value, std::span<const Bit> mods) noexcept
        {
            if (value) {
                return set(mods);
            } else {
                return unset(mods);
            }
        }

        /**
         * @brief Check if none of the modifier keys are set.
         */
        bool none() const noexcept { return m_mods == 0; }

        /**
         * @brief Test if the modifier key is set.
         * @param mod The modifier key to test.
         */
        bool test(Bit mod) const noexcept { return (m_mods & mod) == mod; }

        /**
         * @brief Test if the modifier key(s) are set.
         * @param mods The modifier key(s) to test.
         */
        bool test_all(std::initializer_list<Bit> mods) const noexcept
        {
            auto flags = ModifierKey{ mods };
            return (m_mods & flags.m_mods) == flags.m_mods;
        }

        /**
         * @brief Test if the modifier key(s) are set.
         * @param mods The modifier key(s) to test.
         */
        bool test_all(std::span<const Bit> mods) const noexcept
        {
            auto flags = ModifierKey{ mods };
            return (m_mods & flags.m_mods) == flags.m_mods;
        }

        /**
         * @brief Test if any of the modifier key(s) are set.
         * @param mods The modifier key(s) to test.
         */
        bool test_any(std::initializer_list<Bit> mods) const noexcept
        {
            auto flags = ModifierKey{ mods };
            return (m_mods & flags.m_mods) != 0;
        }

        /**
         * @brief Test if any of the modifier key(s) are set.
         * @param mods The modifier key(s) to test.
         */
        bool test_any(std::span<const Bit> mods) const noexcept
        {
            auto flags = ModifierKey{ mods };
            return (m_mods & flags.m_mods) != 0;
        }

        /**
         * @brief Reset the modifier key(s).
         */
        auto& reset() noexcept
        {
            m_mods = {};
            return *this;
        }

        /**
         * @brief Get the modifier key(s) value.
         */
        explicit operator int() const noexcept { return static_cast<Base>(m_mods); }

        bool operator==(const ModifierKey&) const = default;

    private:
        Base m_mods = 0;
    };

    class Window;

    /**
     * @class KeyStateRecord
     * @brief A record of the key states.
     *
     * This class is used to record the key states of the keyboard.
     */
    class KeyStateRecord
    {
    public:
        friend Window;

        KeyStateRecord() = default;

        /**
         * @brief Check if a key is pressed.
         * @param key_code The key code to check.
         */
        bool is_pressed(KeyCode key_code) const noexcept { return get_bit(bit_pos(key_code)); }

        /**
         * @brief Check if all keys are pressed.
         * @param key_codes The key codes to check.
         */
        bool all_pressed(std::span<const KeyCode> key_codes) const noexcept
        {
            return std::all_of(key_codes.begin(), key_codes.end(), [this](auto k) { return is_pressed(k); });
        }

        /**
         * @brief Check if all keys are pressed.
         * @param key_codes The key codes to check.
         */
        bool all_pressed(std::initializer_list<KeyCode> key_codes) const noexcept
        {
            return std::all_of(key_codes.begin(), key_codes.end(), [this](auto k) { return is_pressed(k); });
        }

        /**
         * @brief Check if any key is pressed.
         * @param key_codes The key codes to check.
         */
        bool any_pressed(std::span<const KeyCode> key_codes) const noexcept
        {
            return std::any_of(key_codes.begin(), key_codes.end(), [this](auto k) { return is_pressed(k); });
        }

        /**
         * @brief Check if any key is pressed.
         * @param key_codes The key codes to check.
         */
        bool any_pressed(std::initializer_list<KeyCode> key_codes) const noexcept
        {
            return std::any_of(key_codes.begin(), key_codes.end(), [this](auto k) { return is_pressed(k); });
        }

        /**
         * @brief Get the pressed keys as a vector.
         *
         * This function is expensive since it creates a new vector every time it is called. Use sparingly.
         */
        std::vector<KeyCode> pressed_keys() const noexcept;

        /**
         * @brief Get the released keys as a vector.
         *
         * This function is expensive since it creates a new vector every time it is called. Use sparingly.
         */
        std::vector<KeyCode> released_keys() const noexcept;

    private:
        using Element = std::uint64_t;
        using State   = std::array<Element, 2>;

        // for friends
        KeyStateRecord& set_value(KeyCode key_code, bool value) noexcept
        {
            set_bit(bit_pos(key_code), value);
            return *this;
        }

        KeyStateRecord& set(KeyCode key_code) noexcept
        {
            set_value(key_code, true);
            return *this;
        }

        KeyStateRecord& unset(KeyCode key_code) noexcept
        {
            set_value(key_code, false);
            return *this;
        }

        void clear() noexcept { m_state.fill(0); };

        // impl detail
        std::size_t bit_pos(KeyCode key_code) const noexcept;
        void        set_bit(std::size_t pos, bool value) noexcept;
        bool        get_bit(std::size_t pos) const noexcept;

        State m_state = {};
    };

    /**
     * @class MouseButtonStateRecord
     * @brief A record of the mouse button states.
     *
     * This class is used to record the mouse button states of the mouse.
     */
    class MouseButtonStateRecord
    {
    public:
        friend Window;

        MouseButtonStateRecord() = default;

        /**
         * @brief Check if a mouse button is pressed.
         * @param button The mouse button to check.
         */
        bool is_pressed(MouseButton button) const noexcept { return get_bit(bit_pos(button)); }

        /**
         * @brief Check if all mouse buttons are pressed.
         * @param buttons The mouse buttons to check.
         */
        bool all_pressed(std::span<const MouseButton> buttons) const noexcept
        {
            return std::all_of(buttons.begin(), buttons.end(), [this](auto k) { return is_pressed(k); });
        }

        /**
         * @brief Check if all mouse buttons are pressed.
         * @param buttons The mouse buttons to check.
         */
        bool all_pressed(std::initializer_list<MouseButton> buttons) const noexcept
        {
            return std::all_of(buttons.begin(), buttons.end(), [this](auto k) { return is_pressed(k); });
        }

        /**
         * @brief Check if any mouse button is pressed.
         * @param buttons The mouse buttons to check.
         */
        bool any_pressed(std::span<const MouseButton> buttons) const noexcept
        {
            return std::any_of(buttons.begin(), buttons.end(), [this](auto k) { return is_pressed(k); });
        }

        /**
         * @brief Check if any mouse button is pressed.
         * @param buttons The mouse buttons to check.
         */
        bool any_pressed(std::initializer_list<MouseButton> buttons) const noexcept
        {
            return std::any_of(buttons.begin(), buttons.end(), [this](auto k) { return is_pressed(k); });
        }

        /**
         * @brief Get the pressed mouse buttons as a vector.
         *
         * This function is expensive since it creates a new vector every time it is called. Use sparingly.
         */
        std::vector<MouseButton> pressed_buttons() const;

        /**
         * @brief Get the released mouse buttons as a vector.
         *
         * This function is expensive since it creates a new vector every time it is called. Use sparingly.
         */
        std::vector<MouseButton> released_buttons() const;

    private:
        using State = std::uint8_t;

        // for friends
        MouseButtonStateRecord& set_value(MouseButton button, bool value) noexcept
        {
            set_bit(bit_pos(button), value);
            return *this;
        }

        MouseButtonStateRecord& set(MouseButton button) noexcept
        {
            set_value(button, true);
            return *this;
        }

        MouseButtonStateRecord& unset(MouseButton button) noexcept
        {
            set_value(button, false);
            return *this;
        }

        void clear() noexcept { m_state = 0; };

        // impl detail
        std::size_t bit_pos(MouseButton button) const noexcept;
        void        set_bit(std::size_t pos, bool value) noexcept;
        bool        get_bit(std::size_t pos) const noexcept;

        State m_state = {};
    };

    /**
     * @brief Get the underlying value of object.
     *
     * @tparam T The type of the object (bound with concept).
     * @param value The object to get the underlying value from.
     * @return An integer representing the underlying value of the object.
     */
    template <typename T>
        requires std::same_as<T, KeyCode>             //
              or std::same_as<T, KeyState>            //
              or std::same_as<T, MouseButton>         //
              or std::same_as<T, MouseButtonState>    //
              or std::same_as<T, ModifierKey>         //
              or std::same_as<T, ModifierKey::Bit>
    int underlying(const T& value) noexcept
    {
        return static_cast<int>(value);
    }
}

#endif /* end of include guard: GLFW_CPP_INPUT_HPP */
