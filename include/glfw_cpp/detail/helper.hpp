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
}

#endif /* end of include guard: HELPER_HPP_3479TGFOWUEA6R */
