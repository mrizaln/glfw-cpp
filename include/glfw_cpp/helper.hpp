#ifndef GLFW_CPP_HELPER_HPP
#define GLFW_CPP_HELPER_HPP

#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

/*
 * @brief Bitwise operators helper for enum classes
 *
 * To use this you need to enable the operators by specializing the `EnableOperators` struct to derive
 * from `std::true_type` for your enum class. After that, import all the operators from this namespace
 * either by `using namespace` the whole namespace or by `using` each operator.
 */
namespace glfw_cpp::helper::enums
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

namespace glfw_cpp::helper::variant
{
    template <typename>
    struct VariantTraits;

    template <template <typename...> typename VT, typename... Ts>
    struct VariantTraits<VT<Ts...>>
    {
        static consteval std::size_t size() { return sizeof...(Ts); }

        template <typename T>
        static consteval bool has_type()
        {
            return (std::same_as<T, Ts> || ...);
        }

        template <typename T>
            requires (has_type<T>())
        static consteval std::size_t type_index()
        {
            return []<std::size_t... Is>(std::index_sequence<Is...>) {
                return ((std::same_as<T, Ts> ? Is : 0) + ...);
            }(std::make_index_sequence<size()>{});
        }

        template <typename Visitor>
        static constexpr bool visitor_complete()
        {
            return (std::invocable<Visitor, Ts&> and ...);
        }

        template <typename Visitor>
        static constexpr bool move_visitor_complete()
        {
            return (std::invocable<Visitor, Ts&&> and ...);
        }

        template <typename Visitor>
        static constexpr bool const_visitor_complete()
        {
            return (std::invocable<Visitor, const Ts&> and ...);
        }

        template <std::size_t I>
        using TypeAt = std::variant_alternative_t<I, std::variant<Ts...>>;

        template <typename T, typename Var>
        using Swap = VariantTraits<Var>::template TypeAt<type_index<T>()>;
    };

    template <typename Visit, typename Var>
    concept VisitorComplete = VariantTraits<Var>::template visitor_complete<Visit>();

    template <typename Visit, typename Var>
    concept MoveVisitorComplete = VariantTraits<Var>::template move_visitor_complete<Visit>();

    template <typename Visit, typename Var>
    concept ConstVisitorComplete = VariantTraits<Var>::template const_visitor_complete<Visit>();

    template <typename T, typename Var>
    concept VariantMember = VariantTraits<Var>::template has_type<T>();

    template <typename Var>
    struct VariantWrapper
    {
        template <typename... Args>
            requires std::constructible_from<Var, Args...>
        VariantWrapper(Args&&... args)
            : variant{ std::forward<Args>(args)... }
        {
        }

        template <VisitorComplete<Var> T>
        decltype(auto) visit(T&& visitor) &
        {
            return std::visit(std::forward<T>(visitor), variant);
        }

        template <MoveVisitorComplete<Var> T>
        decltype(auto) visit(T&& visitor) &&
        {
            return std::visit(std::forward<T>(visitor), std::move(variant));
        }

        template <ConstVisitorComplete<Var> T>
        // template <typename T>
        decltype(auto) visit(T&& visitor) const&
        {
            return std::visit(std::forward<T>(visitor), std::as_const(variant));
        }

        template <VariantMember<Var> T>
        T& get()
        {
            return std::get<T>(variant);
        }

        template <VariantMember<Var> T>
        const T& get() const
        {
            return std::get<T>(variant);
        }

        template <VariantMember<Var> T>
        T* get_if() noexcept
        {
            return std::get_if<T>(&variant);
        }

        template <VariantMember<Var> T>
        const T* get_if() const noexcept
        {
            return std::get_if<T>(&variant);
        }

        template <VariantMember<Var> T>
        bool is() const noexcept
        {
            return std::holds_alternative<T>(variant);
        }

        Var variant;
    };
}

namespace glfw_cpp::helper::meta
{
    template <bool Opt, typename T>
    using MayOpt = std::conditional_t<Opt, std::optional<T>, T>;

    template <bool Opt, std::move_constructible T>
    constexpr MayOpt<Opt, T> may_opt(T t)
    {
        if constexpr (Opt) {
            return std::nullopt;
        } else {
            return std::move(t);
        }
    }
}

#endif /* end of include guard: GLFW_CPP_HELPER_HPP */
