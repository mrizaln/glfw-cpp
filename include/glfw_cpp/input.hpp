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
    /**
     * @enum KeyCode
     * @brief Key codes for keyboard input.
     *
     * This enum is a replacement of macros used in GLFW C header file. The integer values are directly
     * translated from the source [here](https://www.glfw.org/docs/3.3/group__keys.html)
     */
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

    /**
     * @enum KeyState
     * @brief Key states for keyboard input.
     *
     * This enum is a replacement of macros used in GLFW C header file. The integer values are directly
     * translated from the source [here](https://www.glfw.org/docs/3.3/group__input.html)
     */
    enum class KeyState : int
    {
        RELEASE = 0,
        PRESS   = 1,
        REPEAT  = 2
    };

    /**
     * @enum MouseButton
     * @brief Mouse button codes for mouse input.
     *
     * This enum is a replacement of macros used in GLFW C header file. The integer values are directly
     * translated from the source [here](https://www.glfw.org/docs/3.3/group__buttons.html)
     */
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

    /**
     * @enum MouseButtonState
     * @brief Mouse button states for mouse input.
     *
     * This enum is a replacement of macros used in GLFW C header file. The integer values are directly
     * translated from the source [here](https://www.glfw.org/docs/3.3/group__input.html)
     */
    enum class MouseButtonState : int
    {
        RELEASE = 0,
        PRESS   = 1,
    };

    /**
     * @class ModifierKey
     * @brief Modifier key codes for keyboard input.
     *
     * This class is a replacement of macros used in GLFW C header file. The integer values are directly
     * translated from the source [here](https://www.glfw.org/docs/3.3/group__mods.html)
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
         * translated from the source [here](https://www.glfw.org/docs/3.3/group__mods.html)
         */
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

        ModifierKey(std::convertible_to<Bit> auto... e) noexcept
            : m_mods{ static_cast<Base>((0 | ... | e)) }
        {
        }

        ModifierKey(Base mods) noexcept
            : m_mods{ mods }
        {
        }

        /**
         * @brief Set the modifier key(s).
         * @param e The modifier key(s) to set.
         */
        auto& set(std::convertible_to<Bit> auto... e) noexcept
            requires(sizeof...(e) > 0)
        {
            m_mods |= (NONE | ... | e);
            return *this;
        }

        /**
         * @brief Unset the modifier key(s).
         * @param e The modifier key(s) to unset.
         */
        auto& unset(std::convertible_to<Bit> auto... e) noexcept
            requires(sizeof...(e) > 0)
        {
            m_mods &= ~(NONE | ... | e);
            return *this;
        }

        /**
         * @brief Toggle the modifier key(s).
         * @param e The modifier key(s) to toggle.
         */
        auto& toggle(std::convertible_to<Bit> auto... e) noexcept
            requires(sizeof...(e) > 0)
        {
            m_mods ^= (NONE | ... | e);
            return *this;
        }

        /**
         * @brief Set the modifier key(s) to a specific value.
         * @param value The value to set the modifier key(s) to.
         * @param e The modifier key(s) to be set.
         */
        auto& setToValue(bool value, std::convertible_to<Bit> auto... e) noexcept
            requires(sizeof...(e) > 0)
        {
            if (value) {
                return set(std::forward<Bit>(e)...);
            } else {
                return unset(std::forward<Bit>(e)...);
            }
        }

        /**
         * @brief Test if the modifier key(s) are set.
         * @param e The modifier key(s) to test.
         */
        bool test(std::convertible_to<Bit> auto... e) const noexcept
            requires(sizeof...(e) > 0)
        {
            auto flags{ static_cast<Base>((NONE | ... | e)) };
            if (flags == NONE) {
                return m_mods == NONE;
            } else {
                return (m_mods & flags) == flags;
            }
        }

        /**
         * @brief Test if any of the modifier key(s) are set.
         * @param e The modifier key(s) to test.
         */
        bool testSome(std::convertible_to<Bit> auto... e) const noexcept
            requires(sizeof...(e) > 0)
        {
            auto flags{ static_cast<Base>((NONE | ... | e)) };
            if (flags == NONE) {
                return test(static_cast<Bit>(flags));
            } else {
                return (m_mods & flags) != NONE;
            }
        }

        /**
         * @brief Reset the modifier key(s).
         */
        auto& reset() noexcept
        {
            m_mods = NONE;
            return *this;
        }

        /**
         * @brief Get the modifier key(s) value.
         */
        explicit operator int() const noexcept { return static_cast<Base>(m_mods); }

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
         * @param keyCode The key code to check.
         */
        bool isPressed(KeyCode keyCode) const noexcept { return getBit(bitPos(keyCode)); }

        /**
         * @brief Check if all keys are pressed.
         * @param keyCodes The key codes to check.
         */
        bool allPressed(std::initializer_list<KeyCode> keyCodes) const noexcept
        {
            return std::all_of(keyCodes.begin(), keyCodes.end(), [this](auto k) { return isPressed(k); });
        }

        /**
         * @brief Check if any key is pressed.
         * @param keyCodes The key codes to check.
         */
        bool anyPressed(std::initializer_list<KeyCode> keyCodes) const noexcept
        {
            return std::any_of(keyCodes.begin(), keyCodes.end(), [this](auto k) { return isPressed(k); });
        }

        /**
         * @brief Get the pressed keys as a vector.
         *
         * This function is expensive since it creates a new vector every time it is called. Use sparingly.
         */
        std::vector<KeyCode> pressedKeys() const noexcept;

        /**
         * @brief Get the released keys as a vector.
         *
         * This function is expensive since it creates a new vector every time it is called. Use sparingly.
         */
        std::vector<KeyCode> releasedKeys() const noexcept;

    private:
        using Element = std::uint64_t;
        using State   = std::array<Element, 2>;

        // for friends
        void setValue(KeyCode keyCode, bool value) noexcept { setBit(bitPos(keyCode), value); }
        void set(KeyCode keyCode) noexcept { setValue(keyCode, true); }
        void unset(KeyCode keyCode) noexcept { setValue(keyCode, false); }
        void clear() noexcept { m_state.fill(0); };

        // impl detail
        std::size_t bitPos(KeyCode keyCode) const noexcept;
        void        setBit(std::size_t pos, bool value) noexcept;
        bool        getBit(std::size_t pos) const noexcept;

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
        bool isPressed(MouseButton button) const noexcept { return getBit(bitPos(button)); }

        /**
         * @brief Check if all mouse buttons are pressed.
         * @param buttons The mouse buttons to check.
         */
        bool allPressed(std::initializer_list<MouseButton> buttons) const noexcept
        {
            return std::all_of(buttons.begin(), buttons.end(), [this](auto k) { return isPressed(k); });
        }

        /**
         * @brief Check if any mouse button is pressed.
         * @param buttons The mouse buttons to check.
         */
        bool anyPressed(std::initializer_list<MouseButton> buttons) const noexcept
        {
            return std::any_of(buttons.begin(), buttons.end(), [this](auto k) { return isPressed(k); });
        }

        /**
         * @brief Get the pressed mouse buttons as a vector.
         *
         * This function is expensive since it creates a new vector every time it is called. Use sparingly.
         */
        std::vector<MouseButton> pressedButtons() const;

        /**
         * @brief Get the released mouse buttons as a vector.
         *
         * This function is expensive since it creates a new vector every time it is called. Use sparingly.
         */
        std::vector<MouseButton> releasedButtons() const;

    private:
        using State = std::uint8_t;

        // for friends
        void setValue(MouseButton button, bool value) noexcept { setBit(bitPos(button), value); }
        void set(MouseButton button) noexcept { setValue(button, true); }
        void unset(MouseButton button) noexcept { setValue(button, false); }
        void clear() noexcept { m_state = 0; };

        // impl detail
        std::size_t bitPos(MouseButton button) const noexcept;
        void        setBit(std::size_t pos, bool value) noexcept;
        bool        getBit(std::size_t pos) const noexcept;

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

    /**
     * @brief Set the clipboard string.
     * @param string The string to set (must be null-terminated).
     *
     * TODO: move this function to `glfw_cpp::Instance` class
     */
    void setClipboardString(const char* string);

    /**
     * @brief Get the clipboard string.
     * @return The clipboard string.
     *
     * TODO: move this function to `glfw_cpp::Instance` class
     */
    std::string_view getClipboardString();

    /**
     * @brief Get time in seconds since the GLFW library was initialized.
     * @return The time in seconds.
     *
     * TODO: move this function to `glfw_cpp::Instance` class
     */
    double getTime();

    /**
     * @brief Set the current GLFW time.
     * @param time The time to set, in seconds.
     *
     * The time value must be a positive finite number less than or equal to 18446744073.0.
     * TODO: move this function to `glfw_cpp::Instance` class
     */
    void setTime(double time);

    /**
     * @brief Get the current value of the raw timer, measured in 1 / frequency seconds.
     * @return The timer value in microseconds.
     *
     * TODO: move this function to `glfw_cpp::Instance` class
     */
    uint64_t getTimerValue();

    /**
     * @brief Get the frequency, in Hz, of the raw timer.
     * @return The timer frequency.
     *
     * TODO: move this function to `glfw_cpp::Instance` class
     */
    uint64_t getTimerFrequency();
}

#endif /* end of include guard: INPUT_HPP_354TKEJR8H */
