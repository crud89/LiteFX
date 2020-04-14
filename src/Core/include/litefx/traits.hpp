#pragma once

#include <type_traits>

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
    /// template <typename T, typename TBuilder, std::enable_if_t<!rtti::has_builder<T>, int> = 0, typename TBuilder = Builder<T, TBuilder>>
    /// TBuilder makeBuilder() { return TBuilder(); }
    /// </code>
    /// </example>
    /// <typeparam name="T">The type to check for an builder.</typeparam>
    template <class T>
    constexpr bool has_builder_v = has_builder<T>::value;
}