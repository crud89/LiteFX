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

/// <summary>
/// Contains type traits and meta-programming features for compile-time evaluation.
/// </summary>
namespace LiteFX::meta {

    /// <summary>
    /// Trait that is evaluated, if a class does not have an builder member type defined.
    /// </summary>
    template <class, class = std::void_t<>>
    struct has_builder_t : std::false_type { };

    /// <summary>
    /// Trait that is evaluated, if a class does have an builder member type defined.
    /// </summary>
    template<class T>
    struct has_builder_t<T, std::void_t<typename T::builder>> : std::true_type { };

    /// <summary>
    /// Evaluates to either `true` or `false`, if <typeparamref name="T" /> contains an builder member definition.
    /// </summary>
    /// <example>
    /// <code>
    /// template <typename T, std::enable_if_t<meta::has_builder_v<T>, int> = 0, typename TBuilder = T::builder>
    /// TBuilder makeBuilder() { return TBuilder(); }
    ///
    /// template <typename T, typename TBuilder, std::enable_if_t<!meta::has_builder_v<T>, int> = 0, typename TBuilder = Builder<T, TBuilder>>
    /// TBuilder makeBuilder() { return TBuilder(); }
    /// </code>
    /// </example>
    /// <typeparam name="T">The type to check for an builder.</typeparam>
    /// <seealso cref="Builder" />
    template <class T>
    constexpr bool has_builder_v = has_builder_t<T>::value;

    /// <summary>
    /// Checks if a type contains a builder.
    /// </summary>
    /// <typeparam name="T">The type to check for an builder.</typeparam>
    /// <seealso cref="Builder" />
    template <class T>
    concept has_builder = has_builder_v<T>;

    /// <summary>
    /// Evaluates to either `true` or `false`, if <typeparamref name="T" /> can be constructed using the provided arguments, whilst not being able to be converted from <typeparamref name="TArg" />.
    /// </summary>
    /// <typeparam name="T">The type to check.</typeparam>
    /// <typeparam name="TArg">The parameter to check for conversion support.</typeparam>
    /// <typeparam name="...TArgs">The remaining constructor parameters.</typeparam>
    template <typename T, typename TArg, typename ...TArgs>
    struct is_explicitly_constructible_t : std::bool_constant<std::is_constructible_v<T, TArg, TArgs...> && !std::is_convertible_v<TArg, T>> { };

    /// <summary>
    /// Evalues to `true` or `false`, depending if <typeparamref name="T" /> contains an explicit constructor that takes <typeparamref name="TArg" /> and <typeparamref name="TArgs" /> as arguments.
    /// </summary>
    /// <typeparam name="T">The type to check.</typeparam>
    /// <typeparam name="TArg">The parameter to check for conversion support.</typeparam>
    /// <typeparam name="...TArgs">The remaining constructor parameters.</typeparam>
    template <typename T, typename TArg, typename ...TArgs>
    constexpr bool is_explicitly_constructible_v = is_explicitly_constructible_t<T, TArg, TArgs...>::value;

    /// <summary>
    /// Checks if a type can be constructed using the provided arguments, whilst not being able to be converted from <typeparamref name="TArg" />.
    /// </summary>
    /// <typeparam name="T">The type to check.</typeparam>
    /// <typeparam name="TArg">The parameter to check for conversion support.</typeparam>
    /// <typeparam name="...TArgs">The remaining constructor parameters.</typeparam>
    template <typename T, typename TArg, typename ...TArgs>
    concept is_explicitly_constructible = is_explicitly_constructible_v<T, TArg, TArgs...>;

    /// <summary>
    /// Evaluates to either `true` or `false`, if <typeparamref name="T" /> can be constructed using the provided arguments and at the same time can also be converted from <typeparamref name="TArg" />.
    /// </summary>
    /// <typeparam name="T">The type to check.</typeparam>
    /// <typeparam name="TArg">The parameter to check for conversion support.</typeparam>
    /// <typeparam name="...TArgs">The remaining constructor parameters.</typeparam>
    template <typename T, typename TArg, typename ...TArgs>
    struct is_implicitly_constructible_t : std::bool_constant<std::is_constructible_v<T, TArg, TArgs...> && std::is_convertible_v<TArg, T>> { };

    /// <summary>
    /// Evalues to `true` or `false`, depending if <typeparamref name="T" /> contains an implicit constructor that takes <typeparamref name="TArg" /> and <typeparamref name="TArgs" /> as arguments.
    /// </summary>
    /// <typeparam name="T">The type to check.</typeparam>
    /// <typeparam name="TArg">The parameter to check for conversion support.</typeparam>
    /// <typeparam name="...TArgs">The remaining constructor parameters.</typeparam>
    template <typename T, typename TArg, typename ...TArgs>
    constexpr bool is_implicitly_constructible_v = is_explicitly_constructible_t<T, TArg, TArgs...>::value;

    /// <summary>
    /// Checks if a type can be constructed using the provided arguments and at the same time can also be converted from <typeparamref name="TArg" />.
    /// </summary>
    /// <typeparam name="T">The type to check.</typeparam>
    /// <typeparam name="TArg">The parameter to check for conversion support.</typeparam>
    /// <typeparam name="...TArgs">The remaining constructor parameters.</typeparam>
    template <typename T, typename TArg, typename ...TArgs>
    concept is_implicitly_constructible = is_explicitly_constructible_v<T, TArg, TArgs...>;

    /// <summary>
    /// Checks if a type <typeparamref name="TDerived" /> is derived from another type <typeparamref name="TBase" /> and is non-abstract.
    /// </summary>
    /// <typeparam name="TDerived">The type to check.</typeparam>
    /// <typeparam name="TBase">The base type to check against.</typeparam>
    template <typename TDerived, typename TBase>
    concept implements = !std::is_abstract_v<TDerived> && std::derived_from<TDerived, TBase>;

    /// <summary>
    /// Checks if a set of types are all equal to the type <typeparamref name="T" />.
    /// </summary>
    /// <typeparam name="T">The type to check against.</typeparam>
    /// <typeparam name="Ts">The types to check.</typeparam>
    template <typename T, typename... Ts>
    concept are_same = std::conjunction_v<std::is_same<T, Ts>...>;

}