#include "glfw_cpp/input.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>
#include <climits>
#include <vector>

namespace glfw_cpp
{
    // NOTE: every time KeyCode is updated, this function must be updated as well
    std::size_t KeyStateRecord::bitPos(KeyCode keyCode) const noexcept
    {
        switch (keyCode) {
            // clang-format off
            case KeyCode::UNKNOWN         : return 0;
            case KeyCode::SPACE           : return 1;
            case KeyCode::APOSTROPHE      : return 2;
            case KeyCode::COMMA           : return 3;
            case KeyCode::MINUS           : return 4;
            case KeyCode::PERIOD          : return 5;
            case KeyCode::SLASH           : return 6;
            case KeyCode::ZERO            : return 7;
            case KeyCode::ONE             : return 8;
            case KeyCode::TWO             : return 9;
            case KeyCode::THREE           : return 10;
            case KeyCode::FOUR            : return 11;
            case KeyCode::FIVE            : return 12;
            case KeyCode::SIX             : return 13;
            case KeyCode::SEVEN           : return 14;
            case KeyCode::EIGHT           : return 15;
            case KeyCode::NINE            : return 16;
            case KeyCode::SEMICOLON       : return 17;
            case KeyCode::EQUAL           : return 18;
            case KeyCode::A               : return 19;
            case KeyCode::B               : return 20;
            case KeyCode::C               : return 21;
            case KeyCode::D               : return 22;
            case KeyCode::E               : return 23;
            case KeyCode::F               : return 24;
            case KeyCode::G               : return 25;
            case KeyCode::H               : return 26;
            case KeyCode::I               : return 27;
            case KeyCode::J               : return 28;
            case KeyCode::K               : return 29;
            case KeyCode::L               : return 30;
            case KeyCode::M               : return 31;
            case KeyCode::N               : return 32;
            case KeyCode::O               : return 33;
            case KeyCode::P               : return 34;
            case KeyCode::Q               : return 35;
            case KeyCode::R               : return 36;
            case KeyCode::S               : return 37;
            case KeyCode::T               : return 38;
            case KeyCode::U               : return 39;
            case KeyCode::V               : return 40;
            case KeyCode::W               : return 41;
            case KeyCode::X               : return 42;
            case KeyCode::Y               : return 43;
            case KeyCode::Z               : return 44;
            case KeyCode::LEFT_BRACKET    : return 45;
            case KeyCode::BACKSLASH       : return 46;
            case KeyCode::RIGHT_BRACKET   : return 47;
            case KeyCode::GRAVE_ACCENT    : return 48;
            case KeyCode::WORLD_1         : return 49;
            case KeyCode::WORLD_2         : return 50;
            case KeyCode::ESCAPE          : return 51;
            case KeyCode::ENTER           : return 52;
            case KeyCode::TAB             : return 53;
            case KeyCode::BACKSPACE       : return 54;
            case KeyCode::INSERT          : return 55;
            case KeyCode::DELETE          : return 56;
            case KeyCode::RIGHT           : return 57;
            case KeyCode::LEFT            : return 58;
            case KeyCode::DOWN            : return 59;
            case KeyCode::UP              : return 60;
            case KeyCode::PAGE_UP         : return 61;
            case KeyCode::PAGE_DOWN       : return 62;
            case KeyCode::HOME            : return 63;
            case KeyCode::END             : return 64;
            case KeyCode::CAPS_LOCK       : return 65;
            case KeyCode::SCROLL_LOCK     : return 66;
            case KeyCode::NUM_LOCK        : return 67;
            case KeyCode::PRINT_SCREEN    : return 68;
            case KeyCode::PAUSE           : return 69;
            case KeyCode::F1              : return 70;
            case KeyCode::F2              : return 71;
            case KeyCode::F3              : return 72;
            case KeyCode::F4              : return 73;
            case KeyCode::F5              : return 74;
            case KeyCode::F6              : return 75;
            case KeyCode::F7              : return 76;
            case KeyCode::F8              : return 77;
            case KeyCode::F9              : return 78;
            case KeyCode::F10             : return 79;
            case KeyCode::F11             : return 80;
            case KeyCode::F12             : return 81;
            case KeyCode::F13             : return 82;
            case KeyCode::F14             : return 83;
            case KeyCode::F15             : return 84;
            case KeyCode::F16             : return 85;
            case KeyCode::F17             : return 86;
            case KeyCode::F18             : return 87;
            case KeyCode::F19             : return 88;
            case KeyCode::F20             : return 89;
            case KeyCode::F21             : return 90;
            case KeyCode::F22             : return 91;
            case KeyCode::F23             : return 92;
            case KeyCode::F24             : return 93;
            case KeyCode::F25             : return 94;
            case KeyCode::KEYPAD_0        : return 95;
            case KeyCode::KEYPAD_1        : return 96;
            case KeyCode::KEYPAD_2        : return 97;
            case KeyCode::KEYPAD_3        : return 98;
            case KeyCode::KEYPAD_4        : return 99;
            case KeyCode::KEYPAD_5        : return 100;
            case KeyCode::KEYPAD_6        : return 101;
            case KeyCode::KEYPAD_7        : return 102;
            case KeyCode::KEYPAD_8        : return 103;
            case KeyCode::KEYPAD_9        : return 104;
            case KeyCode::KEYPAD_DECIMAL  : return 105;
            case KeyCode::KEYPAD_DIVIDE   : return 106;
            case KeyCode::KEYPAD_MULTIPLY : return 107;
            case KeyCode::KEYPAD_SUBTRACT : return 108;
            case KeyCode::KEYPAD_ADD      : return 109;
            case KeyCode::KEYPAD_ENTER    : return 110;
            case KeyCode::KEYPAD_EQUAL    : return 111;
            case KeyCode::LEFT_SHIFT      : return 112;
            case KeyCode::LEFT_CONTROL    : return 113;
            case KeyCode::LEFT_ALT        : return 114;
            case KeyCode::LEFT_SUPER      : return 115;
            case KeyCode::RIGHT_SHIFT     : return 116;
            case KeyCode::RIGHT_CONTROL   : return 117;
            case KeyCode::RIGHT_ALT       : return 118;
            case KeyCode::RIGHT_SUPER     : return 119;
            case KeyCode::MENU            : return 120;
            default:                        return 0;       // return 0 for any unhadled KeyCode
            // clang-format on
        }
    }

    void KeyStateRecord::setBit(std::size_t pos, bool value) noexcept
    {
        constexpr auto ElementBitCount = sizeof(State::value_type) * CHAR_BIT;
        assert(pos < ElementBitCount * State{}.size());

        const auto bytePos = pos / ElementBitCount;
        const auto bitPos  = pos % ElementBitCount;

        const Element mask = 1ul << bitPos;
        m_state[bytePos]   = value ? m_state[bytePos] | mask : m_state[bytePos] & ~mask;
    }

    bool KeyStateRecord::getBit(std::size_t pos) const noexcept
    {
        constexpr auto ElementBitCount = sizeof(State::value_type) * CHAR_BIT;
        assert(pos < ElementBitCount * State{}.size());

        const auto bytePos = pos / ElementBitCount;
        const auto bitPos  = pos % ElementBitCount;

        const Element mask = 1ul << bitPos;
        return (m_state[bytePos] & mask) != 0;
    }

    std::vector<KeyCode> KeyStateRecord::pressedKeys() const
    {
        std::vector<KeyCode> keys;

        // kinda unsafe
        for (int k = 0; k < static_cast<int>(KeyCode::MAXVALUE); ++k) {
            // eliminate integer values that are not KeyCode and KeyCode::UNKNOWN
            if (bitPos(static_cast<KeyCode>(k)) == 0) {
                continue;
            }

            if (isPressed(static_cast<KeyCode>(k))) {
                keys.push_back(static_cast<KeyCode>(k));
            }
        }
        return keys;
    }

    std::vector<KeyCode> KeyStateRecord::releasedKeys() const
    {
        std::vector<KeyCode> keys;

        // kinda unsafe
        for (int k = 0; k < static_cast<int>(KeyCode::MAXVALUE); ++k) {
            // eliminate integer values that are not KeyCode and KeyCode::UNKNOWN
            if (bitPos(static_cast<KeyCode>(k)) == 0) {
                continue;
            }

            if (!isPressed(static_cast<KeyCode>(k))) {
                keys.push_back(static_cast<KeyCode>(k));
            }
        }
        return keys;
    }
}

namespace glfw_cpp
{
    std::size_t MouseButtonStateRecord::bitPos(MouseButton button) const noexcept
    {
        assert(static_cast<std::size_t>(MouseButton::MAXVALUE) < CHAR_BIT * sizeof(State));
        return static_cast<std::size_t>(button);
    }

    void MouseButtonStateRecord::setBit(std::size_t pos, bool value) noexcept
    {
        assert(pos < CHAR_BIT * sizeof(State));
        const State mask = static_cast<State>(1u << pos);
        m_state          = value ? m_state | mask : m_state & ~mask;
    }

    bool MouseButtonStateRecord::getBit(std::size_t pos) const noexcept
    {
        assert(pos < CHAR_BIT * sizeof(State));
        const State mask = static_cast<State>(1u << pos);
        return (m_state & mask) != 0;
    }

    std::vector<MouseButton> MouseButtonStateRecord::pressedButtons() const
    {
        std::vector<MouseButton> buttons;
        for (int b = 0; b < static_cast<int>(MouseButton::MAXVALUE); ++b) {
            if (isPressed(static_cast<MouseButton>(b))) {
                buttons.push_back(static_cast<MouseButton>(b));
            }
        }
        return buttons;
    }

    std::vector<MouseButton> MouseButtonStateRecord::releasedButtons() const
    {
        std::vector<MouseButton> buttons;
        for (int b = 0; b < static_cast<int>(MouseButton::MAXVALUE); ++b) {
            if (!isPressed(static_cast<MouseButton>(b))) {
                buttons.push_back(static_cast<MouseButton>(b));
            }
        }
        return buttons;
    }
}

namespace glfw_cpp
{
    void setClipboardString(const char* string)
    {
        glfwSetClipboardString(nullptr, string);
    }

    std::string_view getClipboardString()
    {
        return glfwGetClipboardString(nullptr);
    }

    double getTime()
    {
        return glfwGetTime();
    }

    void setTime(double time)
    {
        glfwSetTime(time);
    }

    uint64_t getTimerValue()
    {
        return glfwGetTimerValue();
    }

    uint64_t getTimerFrequency()
    {
        return glfwGetTimerFrequency();
    }
}
