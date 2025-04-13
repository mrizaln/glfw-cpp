#ifndef HELPER_HPP_3479TGFOWUEA6R
#define HELPER_HPP_3479TGFOWUEA6R

#include <concepts>
#include <variant>

namespace glfw_cpp::detail
{
    template <typename>
    struct VarTrait
    {
        static_assert(false, "not a variant");
    };

    template <typename... Ts>
    struct VarTrait<std::variant<Ts...>>
    {
        template <typename U>
        static constexpr bool is_elem()
        {
            return (std::same_as<U, Ts> or ...);
        }

        template <typename U>
        static constexpr bool const_overload_exhaustive()
        {
            return (std::invocable<U, const Ts&> and ...);
        }

        template <typename U>
        static constexpr bool overload_exhaustive()
        {
            return (std::invocable<U, Ts&> and ...);
        }
    };

    /*
     * @brief Bitwise operators helper for enum classes
     *
     * To use this you need to enable the operators by specializing the `EnableOperators` struct to derive
     * from `std::true_type` for your enum class. After that, import all the operators from this namespace
     * either by `using namespace` the whole namespace or by `using` each operator.
     */
    namespace enum_helper
    {
        template <typename T>
        concept Enum = std::is_enum_v<T>;

        template <Enum>
        struct EnableOperators : std::false_type
        {
        };

        template <typename T>
        concept EnabledEnum = Enum<T> && EnableOperators<T>::value;

        template <EnabledEnum E>
        auto to_underlying(E e) noexcept
        {
            return static_cast<std::underlying_type_t<E>>(e);
        }

        namespace operators
        {
            template <EnabledEnum E>
            E operator~(E lhs) noexcept
            {
                return static_cast<E>(~to_underlying(lhs));
            }

            template <EnabledEnum E>
            E operator|(E lhs, E rhs) noexcept
            {
                return static_cast<E>(to_underlying(lhs) | to_underlying(rhs));
            }

            template <EnabledEnum E>
            E operator&(E lhs, E rhs) noexcept
            {
                return static_cast<E>(to_underlying(lhs) & to_underlying(rhs));
            }

            template <EnabledEnum E>
            E operator^(E lhs, E rhs) noexcept
            {
                return static_cast<E>(to_underlying(lhs) ^ to_underlying(rhs));
            }

            template <EnabledEnum E>
            E& operator|=(E& lhs, E rhs) noexcept
            {
                return lhs = lhs | rhs;
            }

            template <EnabledEnum E>
            E& operator&=(E& lhs, E rhs) noexcept
            {
                return lhs = lhs & rhs;
            }

            template <EnabledEnum E>
            E& operator^=(E& lhs, E rhs) noexcept
            {
                return lhs = lhs ^ rhs;
            }
        }
    }
}

#endif /* end of include guard: HELPER_HPP_3479TGFOWUEA6R */
