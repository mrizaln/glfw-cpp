#ifndef CONSTANT_HPP_UGI527S3R8
#define CONSTANT_HPP_UGI527S3R8

#include <concepts>

namespace glfw_cpp::inline constant
{
    template <std::integral Int>
    static constexpr auto dont_care_v = static_cast<Int>(-1);
    static constexpr auto dont_care   = dont_care_v<int>;

    template <std::integral Int>
    static constexpr auto any_position_v = static_cast<Int>(0x80000000);
    static constexpr auto any_position   = any_position_v<int>;
}

#endif /* end of include guard: CONSTANT_HPP_UGI527S3R8 */
