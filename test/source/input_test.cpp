#include <boost/ut.hpp>

#include <glfw_cpp/input.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <ostream>

namespace ut = boost::ut;

namespace glfw_cpp
{
    std::ostream& operator<<(std::ostream& os, const KeyCode& code)
    {
        return os << "KeyCode{" << underlying(code) << "}";
    }

    std::ostream& operator<<(std::ostream& os, const MouseButton& button)
    {
        return os << "MouseButton{" << underlying(button) << "}";
    }

    std::ostream& operator<<(std::ostream& os, const ModifierKey& mods)
    {
        using ModBit = ModifierKey::Bit;

        if (mods.none()) {
            return os << "Mods[None]";
        }

        os << "Mods{";

        auto ss   = std::stringstream{};
        auto test = [&](ModBit mod, std::string_view name) { mods.test(mod) ? ss << name << ", " : ss; };

        test(ModBit::Shift, "Shift");
        test(ModBit::Control, "Control");
        test(ModBit::Alt, "Alt");
        test(ModBit::Super, "Super");
        test(ModBit::CapsLock, "CapsLock");
        test(ModBit::NumLock, "NumLock");

        auto str = ss.str();
        str.erase(str.end() - 2, str.end());

        return os << str << "}";
    }
}

// Just a fake class implementation to test KeyStateRecord and ButtonStateRecord since some operations
// are private that can only be accessed by the friend classes on which Window is one of them.
class glfw_cpp::Window
{
public:
    void test() const
    {
        test_key_state_record();
        test_button_state_record();
    }

    void test_key_state_record() const
    {
        using ut::expect, ut::that;
        using namespace ut::literals;
        using namespace ut::operators;

        [[maybe_unused]] ut::suite key_state_record_tests = [] {
            using Key   = glfw_cpp::KeyCode;
            using State = glfw_cpp::KeyStateRecord;

            "a default constructed KeyStateRecord should have no keys"_test = [] {
                auto state = State{};
                expect(that % not state.is_pressed(Key::A));
                expect(that % not state.is_pressed(Key::B));
                expect(that % not state.is_pressed(Key::C));

                expect(that % not state.is_pressed(Key::Equal));
                expect(that % not state.is_pressed(Key::Keypad4));
                expect(that % not state.is_pressed(Key::GraveAccent));
            };

            "KeyStateRecord should be able to set the key states"_test = [] {
                auto state = State{};
                state.set(Key::A).set(Key::B).set(Key::C);

                expect(that % state.is_pressed(Key::A));
                expect(that % state.is_pressed(Key::B));
                expect(that % state.is_pressed(Key::C));

                state.set(Key::Equal).set(Key::Keypad4).set(Key::GraveAccent);

                expect(that % state.is_pressed(Key::Equal));
                expect(that % state.is_pressed(Key::Keypad4));
                expect(that % state.is_pressed(Key::GraveAccent));
            };

            "KeyStateRecord should be able to unset the key states"_test = [] {
                auto state = State{};
                state.set(Key::A).set(Key::B).set(Key::C);
                state.unset(Key::A).unset(Key::B).unset(Key::C);

                expect(that % not state.is_pressed(Key::A));
                expect(that % not state.is_pressed(Key::B));
                expect(that % not state.is_pressed(Key::C));

                state.set(Key::Equal).set(Key::Keypad4).set(Key::GraveAccent);
                state.unset(Key::Equal).unset(Key::Keypad4).unset(Key::GraveAccent);

                expect(that % not state.is_pressed(Key::Equal));
                expect(that % not state.is_pressed(Key::Keypad4));
                expect(that % not state.is_pressed(Key::GraveAccent));
            };

            "KeyStateRecord should be able to check multiple key states"_test = [] {
                auto state = State{};
                state.set(Key::A).set(Key::B).set(Key::C);

                expect(that % state.all_pressed({ Key::A, Key::B, Key::C }));
                expect(that % state.any_pressed({ Key::A, Key::B, Key::C }));

                expect(that % not state.all_pressed({ Key::A, Key::B, Key::C, Key::D }));
                expect(that % state.any_pressed({ Key::A, Key::B, Key::C, Key::D }));

                state.unset(Key::A).unset(Key::B).unset(Key::C);

                expect(not state.all_pressed({ Key::A, Key::B, Key::C }));
                expect(not state.any_pressed({ Key::A, Key::B, Key::C }));
                expect(not state.all_pressed({ Key::A, Key::B, Key::C, Key::D }));
                expect(not state.any_pressed({ Key::A, Key::B, Key::C, Key::D }));
            };

            "KeyStateRecord should be able to generate a list of pressed keys"_test = [] {
                auto state = State{};
                expect(that % state.pressed_keys().empty());

                state.set(Key::A).set(Key::B).set(Key::C);
                auto keys = state.pressed_keys();

                expect(that % keys.size() == 3);
                expect(that % keys[0] == Key::A);
                expect(that % keys[1] == Key::B);
                expect(that % keys[2] == Key::C);
            };

            "KeyStateRecord should be able to generate a list of released keys"_test = [] {
                auto state = State{};

                state.set(Key::A).set(Key::B).set(Key::C);

                auto keys = state.released_keys();

                // NOTE: this number might change if the underlying GLFW decided to add more keys
                expect(that % keys.size() == 120 - 3);
            };
        };
    }

    void test_button_state_record() const
    {
        using ut::expect, ut::that;
        using namespace ut::literals;
        using namespace ut::operators;

        [[maybe_unused]] ut::suite button_state_record_tests = [] {
            using Btn   = glfw_cpp::MouseButton;
            using State = glfw_cpp::MouseButtonStateRecord;

            "a default constructed MouseButtonStateRecord should have no buttons"_test = [] {
                auto state = State{};
                expect(that % not state.is_pressed(Btn::Left));
                expect(that % not state.is_pressed(Btn::Right));
                expect(that % not state.is_pressed(Btn::Middle));

                expect(that % not state.is_pressed(Btn::Four));
                expect(that % not state.is_pressed(Btn::Five));
                expect(that % not state.is_pressed(Btn::Six));
            };

            "MouseButtonStateRecord should be able to set the button states"_test = [] {
                auto state = State{};
                state.set(Btn::Left).set(Btn::Right).set(Btn::Middle);

                expect(that % state.is_pressed(Btn::Left));
                expect(that % state.is_pressed(Btn::Right));
                expect(that % state.is_pressed(Btn::Middle));

                state.set(Btn::Four).set(Btn::Five).set(Btn::Six);

                expect(that % state.is_pressed(Btn::Four));
                expect(that % state.is_pressed(Btn::Five));
                expect(that % state.is_pressed(Btn::Six));
            };

            "MouseButtonStateRecord should be able to unset the button states"_test = [] {
                auto state = State{};
                state.set(Btn::Left).set(Btn::Right).set(Btn::Middle);
                state.unset(Btn::Left).unset(Btn::Right).unset(Btn::Middle);

                expect(that % not state.is_pressed(Btn::Left));
                expect(that % not state.is_pressed(Btn::Right));
                expect(that % not state.is_pressed(Btn::Middle));

                state.set(Btn::Four).set(Btn::Five).set(Btn::Six);
                state.unset(Btn::Four).unset(Btn::Five).unset(Btn::Six);

                expect(that % not state.is_pressed(Btn::Four));
                expect(that % not state.is_pressed(Btn::Five));
                expect(that % not state.is_pressed(Btn::Six));
            };

            "MouseButtonStateRecord should be able to check multiple button states"_test = [] {
                auto state = State{};
                state.set(Btn::Left).set(Btn::Right).set(Btn::Middle);

                expect(that % state.all_pressed({ Btn::Left, Btn::Right, Btn::Middle }));
                expect(that % state.any_pressed({ Btn::Left, Btn::Right, Btn::Middle }));

                expect(that % not state.all_pressed({ Btn::Left, Btn::Right, Btn::Middle, Btn::Four }));
                expect(that % state.any_pressed({ Btn::Left, Btn::Right, Btn::Middle, Btn::Four }));

                state.unset(Btn::Left).unset(Btn::Right).unset(Btn::Middle);

                expect(not state.all_pressed({ Btn::Left, Btn::Right, Btn::Middle }));
                expect(not state.any_pressed({ Btn::Left, Btn::Right, Btn::Middle }));
                expect(not state.all_pressed({ Btn::Left, Btn::Right, Btn::Middle, Btn::Four }));
                expect(not state.any_pressed({ Btn::Left, Btn::Right, Btn::Middle, Btn::Four }));
            };

            "MouseButtonStateRecord should be able to generate a list of pressed buttons"_test = [] {
                auto state = State{};
                expect(that % state.pressed_buttons().empty());

                state.set(Btn::Left).set(Btn::Right).set(Btn::Middle);
                auto buttons = state.pressed_buttons();

                expect(that % buttons.size() == 3);
                expect(that % buttons[0] == Btn::Left);
                expect(that % buttons[1] == Btn::Right);
                expect(that % buttons[2] == Btn::Middle);
            };

            "MouseButtonStateRecord should be able to generate a list of released buttons"_test = [] {
                auto state = State{};

                state.set(Btn::Left).set(Btn::Right).set(Btn::Middle);

                auto buttons = state.released_buttons();

                // NOTE: this number might change if the underlying GLFW decided to add more buttons
                expect(that % buttons.size() == 8 - 3);
            };
        };
    }
};

int main()
{
    using ut::expect, ut::that;
    using namespace ut::literals;
    using namespace ut::operators;

    [[maybe_unused]] ut::suite modifier_key_tests = [] {
        using Mod    = glfw_cpp::ModifierKey;
        using ModBit = glfw_cpp::ModifierKey::Bit;

        "a default constructed ModifierKey should have no mods"_test = [] {
            auto mods = Mod{};
            expect(that % underlying(mods) == 0);

            auto mods2 = Mod{ ModBit::None };
            expect(that % mods == mods2);
        };

        "ModifierKey should be able to be constructed from multiple mod keys"_test = [] {
            auto mods_new = Mod{ ModBit::Shift, ModBit::Control, ModBit::Alt };
            auto mods_old = GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT;

            expect(that % underlying(mods_new) == mods_old);

            auto mods_new2 = Mod{ mods_old };
            expect(that % mods_new == mods_new2);

            auto mods      = std::array{ ModBit::Shift, ModBit::Control, ModBit::Alt };
            auto mods_new3 = Mod{ mods };
            expect(that % mods_new == mods_new3);
        };

        "ModifierKey can be constructed from an invalid value and ignore the invalid bits"_test = [] {
            auto valid = ModBit::Shift | ModBit::Control | ModBit::Alt | ModBit::Super | ModBit::CapsLock
                       | ModBit::NumLock;

            auto mods = Mod{ -1 };
            expect(that % underlying(mods) == valid);
        };

        "ModifierKey should be able to test single mod keys"_test = [] {
            auto mods = Mod{ ModBit::Shift, ModBit::Control, ModBit::Alt };

            expect(that % mods.test(ModBit::Shift));
            expect(that % mods.test(ModBit::Control));
            expect(that % mods.test(ModBit::Alt));
            expect(not mods.test(ModBit::Super));
            expect(not mods.test(ModBit::CapsLock));
            expect(not mods.test(ModBit::NumLock));
        };

        "ModifierKey should be able to test multiple mod keys"_test = [] {
            auto mods = Mod{ ModBit::Shift, ModBit::Control, ModBit::Alt };

            expect(that % mods.test_all({ ModBit::Shift, ModBit::Control, ModBit::Alt }));
            expect(not mods.test_all({ ModBit::Shift, ModBit::Control, ModBit::Alt, ModBit::Super }));
            expect(not mods.test_all({ ModBit::Shift, ModBit::Control, ModBit::Alt, ModBit::CapsLock }));
            expect(not mods.test_all({ ModBit::Shift, ModBit::Control, ModBit::Alt, ModBit::NumLock }));

            expect(that % mods.test_any({ ModBit::Shift, ModBit::Control, ModBit::Alt }));
            expect(that % mods.test_any({ ModBit::Shift, ModBit::Control, ModBit::Alt, ModBit::Super }));
            expect(not mods.test_any({ ModBit::Super, ModBit::CapsLock, ModBit::NumLock }));
        };

        "ModifierKey should be able to reset the mod keys"_test = [] {
            auto mods = Mod{ ModBit::Shift, ModBit::Control, ModBit::Alt };
            mods.reset();

            expect(that % underlying(mods) == 0);
        };

        "ModifierKey should be able to set the mod keys"_test = [] {
            auto mods = Mod{};
            mods.set({ ModBit::Shift, ModBit::Control, ModBit::Alt });

            expect(that % mods.test_all({ ModBit::Shift, ModBit::Control, ModBit::Alt }));
        };

        "ModifierKey should be able to unset the mod keys"_test = [] {
            auto mods = Mod{ ModBit::Shift, ModBit::Control, ModBit::Alt };
            mods.unset({ ModBit::Shift, ModBit::Control });

            expect(that % mods.test(ModBit::Alt));
            expect(not mods.test(ModBit::Shift));
            expect(not mods.test(ModBit::Control));
        };

        "ModifierKey should be able to set the mod keys to a value"_test = [] {
            auto mods = Mod{};
            mods.set_to_value(true, { ModBit::Shift, ModBit::Control, ModBit::Alt });

            expect(that % mods.test_all({ ModBit::Shift, ModBit::Control, ModBit::Alt }));

            mods.set_to_value(false, { ModBit::Shift, ModBit::Control, ModBit::Alt });

            expect(that % underlying(mods) == 0);
        };

        "ModifierKey should be able to toggle the mod keys"_test = [] {
            auto mods = Mod{};
            mods.toggle({ ModBit::Shift, ModBit::Control, ModBit::Alt });

            expect(that % mods.test_all({ ModBit::Shift, ModBit::Control, ModBit::Alt }));

            mods.toggle({ ModBit::Shift, ModBit::Control, ModBit::Alt });

            expect(that % underlying(mods) == 0);
        };
    };

    glfw_cpp::Window{}.test();
}
