#ifndef HELPER_HPP_3479TGFOWUEA6R
#define HELPER_HPP_3479TGFOWUEA6R

#include <type_traits>

namespace glfw_cpp::detail
{
    template <typename T, typename... Ts>
    concept IsAnyOf = (std::is_same_v<T, Ts> || ...);
}

#endif /* end of include guard: HELPER_HPP_3479TGFOWUEA6R */
