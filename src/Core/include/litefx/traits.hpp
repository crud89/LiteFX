#pragma once

#include <type_traits>
#include <typeindex>
#include <concepts>

#ifndef __cpp_size_t_suffix
// Implements C++23 P0330R8 for when compiler is missing support (MSVC only).
#pragma warning(suppress: 4455) // Ignore warning about reserved suffix, as this is guarded anyway.
inline constexpr std::size_t operator"" uz(unsigned long long int k)
{
    return static_cast<std::size_t>(k);
}
#endif

/// @brief Contains type traits and meta-programming features for compile-time evaluation.
namespace LiteFX::meta {

    /// @brief Trait that is evaluated, if a class does not have an builder member type defined.
    template <class, class = std::void_t<>>
    struct has_builder_t : std::false_type { };

    /// @brief Trait that is evaluated, if a class does have an builder member type defined.
    template<class T>
    struct has_builder_t<T, std::void_t<typename T::builder>> : std::true_type { };

    /// @brief Evaluates to either `true` or `false`, if @p T contains an builder member definition.
    ///
    /// @par Example
    /// @code
    /// template <typename T, std::enable_if_t<meta::has_builder_v<T>, int> = 0, typename TBuilder = T::builder>
    /// TBuilder makeBuilder() { return TBuilder(); }
    ///
    /// template <typename T, typename TBuilder, std::enable_if_t<!meta::has_builder_v<T>, int> = 0, typename TBuilder = Builder<T, TBuilder>>
    /// TBuilder makeBuilder() { return TBuilder(); }
    /// @endcode
    ///
    /// @tparam T The type to check for an builder.
    /// @see Builder
    template <class T>
    constexpr bool has_builder_v = has_builder_t<T>::value;

    /// @brief Checks if a type contains a builder.
    ///
    /// @tparam T The type to check for an builder.
    /// @see Builder
    template <class T>
    concept has_builder = has_builder_v<T>;

    /// @brief Evaluates to either `true` or `false`, if @p T can be constructed using the provided arguments, whilst not being able to be converted from @p TArg.
    ///
    /// @tparam T The type to check.
    /// @tparam TArg The parameter to check for conversion support.
    /// @tparam ...TArgs The remaining constructor parameters.
    template <typename T, typename TArg, typename ...TArgs>
    struct is_explicitly_constructible_t : std::bool_constant<std::is_constructible_v<T, TArg, TArgs...> && !std::is_convertible_v<TArg, T>> { };

    /// @brief Evalues to `true` or `false`, depending if @p T contains an explicit constructor that takes @p TArg and @p TArgs as arguments.
    ///
    /// @tparam T The type to check.
    /// @tparam TArg The parameter to check for conversion support.
    /// @tparam ...TArgs The remaining constructor parameters.
    template <typename T, typename TArg, typename ...TArgs>
    constexpr bool is_explicitly_constructible_v = is_explicitly_constructible_t<T, TArg, TArgs...>::value;

    /// @brief Checks if a type can be constructed using the provided arguments, whilst not being able to be converted from @p TArg.
    ///
    /// @tparam T The type to check.
    /// @tparam TArg The parameter to check for conversion support.
    /// @tparam ...TArgs The remaining constructor parameters.
    template <typename T, typename TArg, typename ...TArgs>
    concept is_explicitly_constructible = is_explicitly_constructible_v<T, TArg, TArgs...>;

    /// @brief Evaluates to either `true` or `false`, if @p T can be constructed using the provided arguments and at the same time can also be converted from @p TArg.
    ///
    /// @tparam T The type to check.
    /// @tparam TArg The parameter to check for conversion support.
    /// @tparam ...TArgs The remaining constructor parameters.
    template <typename T, typename TArg, typename ...TArgs>
    struct is_implicitly_constructible_t : std::bool_constant<std::is_constructible_v<T, TArg, TArgs...> && std::is_convertible_v<TArg, T>> { };

    /// @brief Evalues to `true` or `false`, depending if @p T contains an implicit constructor that takes @p TArg and @p TArgs as arguments.
    ///
    /// @tparam T The type to check.
    /// @tparam TArg The parameter to check for conversion support.
    /// @tparam ...TArgs The remaining constructor parameters.
    template <typename T, typename TArg, typename ...TArgs>
    constexpr bool is_implicitly_constructible_v = is_explicitly_constructible_t<T, TArg, TArgs...>::value;

    /// @brief Checks if a type can be constructed using the provided arguments and at the same time can also be converted from @p TArg.
    ///
    /// @tparam T The type to check.
    /// @tparam TArg The parameter to check for conversion support.
    /// @tparam ...TArgs The remaining constructor parameters.
    template <typename T, typename TArg, typename ...TArgs>
    concept is_implicitly_constructible = is_explicitly_constructible_v<T, TArg, TArgs...>;

    /// @brief Checks if a type @p TDerived is derived from another type @p TBase and is non-abstract.
    ///
    /// @tparam TDerived The type to check.
    /// @tparam TBase The base type to check against.
    template <typename TDerived, typename TBase>
    concept implements = !std::is_abstract_v<TDerived> && std::derived_from<TDerived, TBase>;

    /// @brief Checks if a set of types are all equal to the type @p T.
    ///
    /// @tparam T The type to check against.
    /// @tparam Ts The types to check.
    template <typename T, typename... Ts>
    concept are_same = std::conjunction_v<std::is_same<T, Ts>...>;

}