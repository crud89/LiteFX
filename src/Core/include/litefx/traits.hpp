#pragma once

#include <type_traits>
#include <concepts>

namespace LiteFX::rtti {

    /// <summary>
    /// Trait that is evaluated, if a class does not have an builder member type defined.
    /// </summary>
    template <class, class = std::void_t<>>
    struct has_builder : std::false_type { };

    /// <summary>
    /// Trait that is evaluated, if a class does have an builder member type defined.
    /// </summary>
    template<class T>
    struct has_builder<T, std::void_t<typename T::builder>> : std::true_type { };

    /// <summary>
    /// Evaluates to either `true` or `false`, if <typeparamref name="T" /> contains an builder member definition.
    /// </summary>
    /// <example>
    /// <code>
    /// template <typename T, std::enable_if_t<rtti::has_builder_v<T>, int> = 0, typename TBuilder = T::builder>
    /// TBuilder makeBuilder() { return TBuilder(); }
    ///
    /// template <typename T, typename TBuilder, std::enable_if_t<!rtti::has_builder_v<T>, int> = 0, typename TBuilder = Builder<T, TBuilder>>
    /// TBuilder makeBuilder() { return TBuilder(); }
    /// </code>
    /// </example>
    /// <typeparam name="T">The type to check for an builder.</typeparam>
    template <class T>
    constexpr bool has_builder_v = has_builder<T>::value;

    /// <summary>
    /// 
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <typeparam name="TArg"></typeparam>
    /// <typeparam name="...TArgs"></typeparam>
    template <typename T, typename TArg, typename ...TArgs>
    struct is_explicitly_constructible : std::bool_constant<std::is_constructible_v<T, TArg, TArgs...> && !std::is_convertible_v<TArg, T>> { };

    /// <summary>
    /// Evalues to `true` or `false`, depending if <typeparamref name="T" /> contains an explicit constructor that takes <typeparamref name="TArg" /> and <typeparamref name="TArgs" /> as arguments.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <typeparam name="TArg"></typeparam>
    /// <typeparam name="...TArgs"></typeparam>
    template <typename T, typename TArg, typename ...TArgs>
    constexpr bool is_explicitly_constructible_v = is_explicitly_constructible<T, TArg, TArgs...>::value;

    /// <summary>
    /// 
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <typeparam name="TArg"></typeparam>
    /// <typeparam name="...TArgs"></typeparam>
    template <typename T, typename TArg, typename ...TArgs>
    struct is_implicitly_constructible : std::bool_constant<std::is_constructible_v<T, TArg, TArgs...> && std::is_convertible_v<TArg, T>> { };

    /// <summary>
    /// Evalues to `true` or `false`, depending if <typeparamref name="T" /> contains an implicit constructor that takes <typeparamref name="TArg" /> and <typeparamref name="TArgs" /> as arguments.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <typeparam name="TArg"></typeparam>
    /// <typeparam name="...TArgs"></typeparam>
    template <typename T, typename TArg, typename ...TArgs>
    constexpr bool is_implicitly_constructible_v = is_explicitly_constructible<T, TArg, TArgs...>::value;
}