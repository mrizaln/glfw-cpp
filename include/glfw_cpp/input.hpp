#ifndef INPUT_HPP_354TKEJR8H
#define INPUT_HPP_354TKEJR8H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <concepts>
#include <utility>

namespace glfw_cpp
{
    enum class KeyCode
    {
        UNKNOWN         = GLFW_KEY_UNKNOWN,
        SPACE           = GLFW_KEY_SPACE,
        APOSTROPHE      = GLFW_KEY_APOSTROPHE,
        COMMA           = GLFW_KEY_COMMA,
        MINUS           = GLFW_KEY_MINUS,
        PERIOD          = GLFW_KEY_PERIOD,
        SLASH           = GLFW_KEY_SLASH,
        ZERO            = GLFW_KEY_0,
        ONE             = GLFW_KEY_1,
        TWO             = GLFW_KEY_2,
        THREE           = GLFW_KEY_3,
        FOUR            = GLFW_KEY_4,
        FIVE            = GLFW_KEY_5,
        SIX             = GLFW_KEY_6,
        SEVEN           = GLFW_KEY_7,
        EIGHT           = GLFW_KEY_8,
        NINE            = GLFW_KEY_9,
        SEMICOLON       = GLFW_KEY_SEMICOLON,
        EQUAL           = GLFW_KEY_EQUAL,
        A               = GLFW_KEY_A,
        B               = GLFW_KEY_B,
        C               = GLFW_KEY_C,
        D               = GLFW_KEY_D,
        E               = GLFW_KEY_E,
        F               = GLFW_KEY_F,
        G               = GLFW_KEY_G,
        H               = GLFW_KEY_H,
        I               = GLFW_KEY_I,
        J               = GLFW_KEY_J,
        K               = GLFW_KEY_K,
        L               = GLFW_KEY_L,
        M               = GLFW_KEY_M,
        N               = GLFW_KEY_N,
        O               = GLFW_KEY_O,
        P               = GLFW_KEY_P,
        Q               = GLFW_KEY_Q,
        R               = GLFW_KEY_R,
        S               = GLFW_KEY_S,
        T               = GLFW_KEY_T,
        U               = GLFW_KEY_U,
        V               = GLFW_KEY_V,
        W               = GLFW_KEY_W,
        X               = GLFW_KEY_X,
        Y               = GLFW_KEY_Y,
        Z               = GLFW_KEY_Z,
        LEFT_BRACKET    = GLFW_KEY_LEFT_BRACKET,
        BACKSLASH       = GLFW_KEY_BACKSLASH,
        RIGHT_BRACKET   = GLFW_KEY_RIGHT_BRACKET,
        GRAVE_ACCENT    = GLFW_KEY_GRAVE_ACCENT,
        WORLD_1         = GLFW_KEY_WORLD_1,
        WORLD_2         = GLFW_KEY_WORLD_2,
        ESCAPE          = GLFW_KEY_ESCAPE,
        ENTER           = GLFW_KEY_ENTER,
        TAB             = GLFW_KEY_TAB,
        BACKSPACE       = GLFW_KEY_BACKSPACE,
        INSERT          = GLFW_KEY_INSERT,
        DELETE          = GLFW_KEY_DELETE,
        RIGHT           = GLFW_KEY_RIGHT,
        LEFT            = GLFW_KEY_LEFT,
        DOWN            = GLFW_KEY_DOWN,
        UP              = GLFW_KEY_UP,
        PAGE_UP         = GLFW_KEY_PAGE_UP,
        PAGE_DOWN       = GLFW_KEY_PAGE_DOWN,
        HOME            = GLFW_KEY_HOME,
        END             = GLFW_KEY_END,
        CAPS_LOCK       = GLFW_KEY_CAPS_LOCK,
        SCROL_LLOCK     = GLFW_KEY_SCROLL_LOCK,
        NUM_LOCK        = GLFW_KEY_NUM_LOCK,
        PRINT_SCREEN    = GLFW_KEY_PRINT_SCREEN,
        PAUSE           = GLFW_KEY_PAUSE,
        F1              = GLFW_KEY_F1,
        F2              = GLFW_KEY_F2,
        F3              = GLFW_KEY_F3,
        F4              = GLFW_KEY_F4,
        F5              = GLFW_KEY_F5,
        F6              = GLFW_KEY_F6,
        F7              = GLFW_KEY_F7,
        F8              = GLFW_KEY_F8,
        F9              = GLFW_KEY_F9,
        F10             = GLFW_KEY_F10,
        F11             = GLFW_KEY_F11,
        F12             = GLFW_KEY_F12,
        F13             = GLFW_KEY_F13,
        F14             = GLFW_KEY_F14,
        F15             = GLFW_KEY_F15,
        F16             = GLFW_KEY_F16,
        F17             = GLFW_KEY_F17,
        F18             = GLFW_KEY_F18,
        F19             = GLFW_KEY_F19,
        F20             = GLFW_KEY_F20,
        F21             = GLFW_KEY_F21,
        F22             = GLFW_KEY_F22,
        F23             = GLFW_KEY_F23,
        F24             = GLFW_KEY_F24,
        F25             = GLFW_KEY_F25,
        KEYPAD_0        = GLFW_KEY_KP_0,
        KEYPAD_1        = GLFW_KEY_KP_1,
        KEYPAD_2        = GLFW_KEY_KP_2,
        KEYPAD_3        = GLFW_KEY_KP_3,
        KEYPAD_4        = GLFW_KEY_KP_4,
        KEYPAD_5        = GLFW_KEY_KP_5,
        KEYPAD_6        = GLFW_KEY_KP_6,
        KEYPAD_7        = GLFW_KEY_KP_7,
        KEYPAD_8        = GLFW_KEY_KP_8,
        KEYPAD_9        = GLFW_KEY_KP_9,
        KEYPAD_DECIMAL  = GLFW_KEY_KP_DECIMAL,
        KEYPAD_DIVIDE   = GLFW_KEY_KP_DIVIDE,
        KEYPAD_MULTIPLY = GLFW_KEY_KP_MULTIPLY,
        KEYPAD_SUBTRACT = GLFW_KEY_KP_SUBTRACT,
        KEYPAD_ADD      = GLFW_KEY_KP_ADD,
        KEYPAD_ENTER    = GLFW_KEY_KP_ENTER,
        KEYPAD_EQUAL    = GLFW_KEY_KP_EQUAL,
        LEFT_SHIFT      = GLFW_KEY_LEFT_SHIFT,
        LEFT_CONTROL    = GLFW_KEY_LEFT_CONTROL,
        LEFT_ALT        = GLFW_KEY_LEFT_ALT,
        LEFT_SUPER      = GLFW_KEY_LEFT_SUPER,
        RIGHT_SHIFT     = GLFW_KEY_RIGHT_SHIFT,
        RIGHT_CONTROL   = GLFW_KEY_RIGHT_CONTROL,
        RIGHT_ALT       = GLFW_KEY_RIGHT_ALT,
        RIGHT_SUPER     = GLFW_KEY_RIGHT_SUPER,
        MENU            = GLFW_KEY_MENU,
        MAXVALUE        = GLFW_KEY_LAST
    };

    enum class KeyState
    {
        RELEASE = GLFW_RELEASE,
        PRESS   = GLFW_PRESS,
        REPEAT  = GLFW_REPEAT
    };

    enum class MouseButton
    {
        ONE      = GLFW_MOUSE_BUTTON_1,
        TWO      = GLFW_MOUSE_BUTTON_2,
        THREE    = GLFW_MOUSE_BUTTON_3,
        FOUR     = GLFW_MOUSE_BUTTON_4,
        FIVE     = GLFW_MOUSE_BUTTON_5,
        SIX      = GLFW_MOUSE_BUTTON_6,
        SEVEN    = GLFW_MOUSE_BUTTON_7,
        EIGHT    = GLFW_MOUSE_BUTTON_8,
        LEFT     = GLFW_MOUSE_BUTTON_LEFT,
        RIGHT    = GLFW_MOUSE_BUTTON_RIGHT,
        MIDDLE   = GLFW_MOUSE_BUTTON_MIDDLE,
        MAXVALUE = GLFW_MOUSE_BUTTON_LAST
    };

    enum class MouseButtonState
    {
        RELEASE = GLFW_RELEASE,
        PRESS   = GLFW_PRESS,
    };

    class ModifierKey
    {
    public:
        using Base = unsigned char;

        enum Bit : Base
        {
            NONE      = 0,
            SHIFT     = GLFW_MOD_SHIFT,
            CONTROL   = GLFW_MOD_CONTROL,
            ALT       = GLFW_MOD_ALT,
            SUPER     = GLFW_MOD_SUPER,
            CAPS_LOCK = GLFW_MOD_CAPS_LOCK,
            NUM_LOCK  = GLFW_MOD_NUM_LOCK
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

    private:
        Base m_mods = 0;
    };
}

#endif /* end of include guard: INPUT_HPP_354TKEJR8H */
