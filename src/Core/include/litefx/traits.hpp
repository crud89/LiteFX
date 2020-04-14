#pragma once

#include <type_traits>

namespace LiteFX::rtti {

    /// <summary>
    /// Trait that is evaluated, if a class does not have an initializer member type defined.
    /// </summary>
    template <class, class = std::void_t<>>
    struct has_initializer : std::false_type { };

    /// <summary>
    /// Trait that is evaluated, if a class does have an initializer member type defined.
    /// </summary>
    template<class T>
    struct has_initializer<T, std::void_t<typename T::initializer>> : std::true_type { };

    /// <summary>
    /// Evaluates to either `true` or `false`, if <typeparamref name="T" /> contains an initializer member definition.
    /// </summary>
    /// <example>
    /// <code>
    /// template <typename T, std::enable_if_t<rtti::has_initializer_v<T>, int> = 0, typename TBuilder = T::initializer>
    /// TBuilder makeBuilder() { return TBuilder(); }
    ///
    /// template <typename T, typename TBuilder, std::enable_if_t<!rtti::has_initializer<T>, int> = 0, typename TBuilder = Builder<T, TBuilder>>
    /// TBuilder makeBuilder() { return TBuilder(); }
    /// </code>
    /// </example>
    /// <typeparam name="T">The type to check for an initializer.</typeparam>
    template <class T>
    constexpr bool has_initializer_v = has_initializer<T>::value;
}