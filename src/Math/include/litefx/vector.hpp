#pragma once

#include <cassert>
#include <algorithm>
#include <array>
#include <vector>
#include <ranges>

namespace LiteFX::Math {

	/// <summary>
	/// An algebraic vector type.
	/// </summary>
    /// <remarks>
    /// The value type of a vector must be in standard layout (i.e., `std::is_standard_layout_v<T>` must evaluate to `true`). This constraint is enforced at compile time and ensures that
    /// vector types can be binary marshalled. For example, the <seealso cref="LiteFX::Graphics::Vertex" /> type stores nothing more than a series of vectors. The standard layout constraint
    /// ensures that a set of vertices can be converted into a plain byte array and back.
    /// </remarks>
	/// <typeparam name="T">The type of the vector scalar elements. Must be in standard layout (i.e., `std::is_standard_layout_v<T>` must evaluate to `true`).</typeparam>
	/// <typeparam name="DIM">The number of dimensions of the vector.</typeparam>
    template <typename T, unsigned DIM> requires 
        std::is_standard_layout_v<T>
    struct Vector {
    public:
        /// <summary>
        /// Stores the size of the vector.
        /// </summary>
        static constexpr size_t vec_size = DIM;

        /// <summary>
        /// The type of the vector elements.
        /// </summary>
        using scalar_type = T;

        /// <summary>
        /// The type of the vector itself.
        /// </summary>
        using vec_type = Vector<scalar_type, vec_size>;

    protected:
        using array_type = std::array<scalar_type, vec_size>;
        array_type m_elements = { }; // NOLINT

    public:
        /// <summary>
        /// Initializes an empty vector.
        /// </summary>
        constexpr Vector() noexcept = default;

        /// <summary>
        /// Initializes a vector where all elements take the value provided by <paramref name="val" />.
        /// </summary>
        /// <param name="val">The value to initialize all elements of the vector with.</param>
        constexpr Vector(T val) noexcept {
            std::fill(std::begin(m_elements), std::end(m_elements), val);
        }

        /// <summary>
        /// Initializes a vector with the values provided by another vector.
        /// </summary>
        /// <param name="_other">The other vector to copy the values from.</param>
        constexpr Vector(const Vector<T, DIM>& _other) noexcept {
            std::ranges::copy(_other.m_elements, std::begin(m_elements));
        }

        /// <summary>
        /// Initializes a vector by taking over another vector.
        /// </summary>
        /// <param name="_other">The vector to take over.</param>
        constexpr Vector(Vector<T, DIM>&& _other) noexcept :
            m_elements(std::move(_other.m_elements)) { }

        /// <summary>
        /// Destroys the vector.
        /// </summary>
        constexpr ~Vector() noexcept = default;

        /// <summary>
        /// Initializes a 2D vector using the values provided by <paramref name="x" /> and <paramref name="y" />.
        /// </summary>
        /// <param name="x">The value to initialize the x-component of the vector with.</param>
        /// <param name="y">The value to initialize the y-component of the vector with.</param>
        constexpr Vector(T x, T y) noexcept requires(DIM == 2)
        {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
            m_elements[0] = x;
            m_elements[1] = y;
            // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// <summary>
        /// Initializes a 3D vector using the values provided by <paramref name="x" />, <paramref name="y" /> and <paramref name="z" />.
        /// </summary>
        /// <param name="x">The value to initialize the x-component of the vector with.</param>
        /// <param name="y">The value to initialize the y-component of the vector with.</param>
        /// <param name="z">The value to initialize the z-component of the vector with.</param>
        constexpr Vector(T x, T y, T z) noexcept requires(DIM == 3)
        {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
            m_elements[0] = x;
            m_elements[1] = y;
            m_elements[2] = z;
            // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// <summary>
        /// Initializes a 4D vector using the values provided by <paramref name="x" />, <paramref name="y" />, <paramref name="z" /> and <paramref name="w" />.
        /// </summary>
        /// <param name="x">The value to initialize the x-component of the vector with.</param>
        /// <param name="y">The value to initialize the y-component of the vector with.</param>
        /// <param name="z">The value to initialize the z-component of the vector with.</param>
        /// <param name="w">The value to initialize the w-component of the vector with.</param>
        constexpr Vector(T x, T y, T z, T w) noexcept requires(DIM == 4)
        {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
            m_elements[0] = x;
            m_elements[1] = y;
            m_elements[2] = z;
            m_elements[3] = w;
            // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// <summary>
        /// Initializes the vector from an arbitrary input range.
        /// </summary>
        /// <param name="input">The range to initialize the vector with.</param>
        constexpr explicit Vector(std::ranges::input_range auto&& input) noexcept requires
            std::is_nothrow_convertible_v<std::ranges::range_value_t<decltype(input)>, T>
        {
            std::ranges::copy(input, std::begin(m_elements));
        }

        /// <summary>
        /// Copys the elements of another vector into the current vector.
        /// </summary>
        /// <param name="_other">The vector to copy the elements from.</param>
        /// <returns>A reference to the current vector instance.</returns>
        constexpr auto& operator=(const Vector<T, DIM>& _other) noexcept {
            std::ranges::copy(_other.m_elements, std::begin(m_elements));
            return *this;
        }

        /// <summary>
        /// Moves the elements of the other vector to the current vector.
        /// </summary>
        /// <param name="_other">The vector to take over.</param>
        /// <returns>A reference to the current vector instance.</returns>
        constexpr auto& operator=(Vector<T, DIM>&& _other) noexcept {
            m_elements = std::move(_other.m_elements);
            return *this;
        }

        /// <summary>
        /// Copies the values from an arbitrary input range into the current vector instance.
        /// </summary>
        /// <param name="input">The input range to copy the values from.</param>
        /// <returns>A reference to the current vector instance.</returns>
        constexpr auto& operator=(std::ranges::input_range auto&& input) noexcept requires
            std::is_nothrow_convertible_v<std::ranges::range_value_t<decltype(input)>, T>
        {
            std::ranges::copy(input, std::begin(m_elements));
            return *this;
        }

    public:
        /// <summary>
        /// Returns a value from the vector, indexed by the parameter <paramref name="i" />.
        /// </summary>
        /// <remarks>
        /// Note that this method wraps the index if it is out of range, i.e., calling the method with index `4` on a 4D vector will return the element at index `0`.
        /// </remarks>
        /// <param name="i">The index of the element to return.</param>
        /// <returns>The value of the element at the provided index.</returns>
        constexpr T operator[](unsigned int i) const noexcept {
            assert(i < DIM);

            return m_elements[i % DIM]; // NOLINT
        }

        /// <summary>
        /// Returns a reference to a value from the vector, indexed by the parameter <paramref name="i" />.
        /// </summary>
        /// <remarks>
        /// Note that this method wraps the index if it is out of range, i.e., calling the method with index `4` on a 4D vector will return the element at index `0`.
        /// </remarks>
        /// <param name="i">The index of the element to return.</param>
        /// <returns>A reference to a value of the element at the provided index.</returns>
        constexpr T& operator[](unsigned int i) noexcept {
            assert(i < DIM);

            return m_elements[i % DIM]; // NOLINT
        }

        /// <summary>
        /// Returns an interator for that addresses the begin of the vector elements.
        /// </summary>
        /// <returns>An interator for that addresses the begin of the vector elements.</returns>
        constexpr auto begin() noexcept {
            return m_elements.begin();
        }

        /// <summary>
        /// Returns an interator for that addresses the end of the vector elements.
        /// </summary>
        /// <returns>An interator for that addresses the end of the vector elements.</returns>
        constexpr auto end() noexcept {
            return m_elements.end();
        }

        /// <summary>
        /// Returns a constant interator for that addresses the begin of the vector elements.
        /// </summary>
        /// <returns>A constant interator for that addresses the begin of the vector elements.</returns>
        constexpr auto cbegin() const noexcept {
            return m_elements.cbegin();
        }

        /// <summary>
        /// Returns a constant interator for that addresses the end of the vector elements.
        /// </summary>
        /// <returns>A constant interator for that addresses the end of the vector elements.</returns>
        constexpr auto cend() const noexcept {
            return m_elements.cend();
        }

    public:
        /// <summary>
        /// Returns a pointer to the elements of the vector.
        /// </summary>
        /// <returns>A pointer to the elements of the vector.</returns>
        constexpr const scalar_type* elements() const noexcept {
            return m_elements.data();
        }

        /// <summary>
        /// Converts the vector to an instance of `std::array`.
        /// </summary>
        constexpr operator std::array<T, DIM>() const noexcept {
            return m_elements;
        }

        /// <summary>
        /// Converts the vector into an instance of type `std::vector`.
        /// </summary>
        constexpr operator std::vector<T>() const noexcept {
            return std::vector<T>(std::begin(m_elements), std::end(m_elements));
        }

        /// <summary>
        /// Returns the number of dimensions of the vector.
        /// </summary>
        /// <returns>The number of dimensions of the vector.</returns>
        constexpr int size() const noexcept {
            return vec_size;
        }

        /// <summary>
        /// Returns the value of the x component of the vector.
        /// </summary>
        /// <returns>The value of the x component of the vector.</returns>
        constexpr scalar_type x() const noexcept requires (DIM > 0) {
            return m_elements[0]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// <summary>
        /// Returns a reference of the value of the x component of the vector.
        /// </summary>
        /// <returns>The a reference of the value of the x component of the vector.</returns>
        constexpr scalar_type& x() noexcept requires (DIM > 0) {
            return m_elements[0]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// <summary>
        /// Returns the value of the y component of the vector.
        /// </summary>
        /// <returns>The value of the y component of the vector.</returns>
        constexpr scalar_type y() const noexcept requires (DIM > 1) {
            return m_elements[1]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// <summary>
        /// Returns a reference of the value of the y component of the vector.
        /// </summary>
        /// <returns>The a reference of the value of the y component of the vector.</returns>
        constexpr scalar_type& y() noexcept requires (DIM > 1) {
            return m_elements[1]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// <summary>
        /// Returns the value of the z component of the vector.
        /// </summary>
        /// <returns>The value of the z component of the vector.</returns>
        constexpr scalar_type z() const noexcept requires (DIM > 2) {
            return m_elements[2]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// <summary>
        /// Returns a reference of the value of the z component of the vector.
        /// </summary>
        /// <returns>The a reference of the value of the z component of the vector.</returns>
        constexpr scalar_type& z() noexcept requires (DIM > 2) {
            return m_elements[2]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// <summary>
        /// Returns the value of the w component of the vector.
        /// </summary>
        /// <returns>The value of the w component of the vector.</returns>
        constexpr scalar_type w() const noexcept requires (DIM > 3) {
            return m_elements[3]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// <summary>
        /// Returns a reference of the value of the w component of the vector.
        /// </summary>
        /// <returns>The a reference of the value of the w component of the vector.</returns>
        constexpr scalar_type& w() noexcept requires (DIM > 3) {
            return m_elements[3]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }
    };

	/// <summary>
	/// A generic vector with one component.
	/// </summary>
	/// <typeparam name="T">The type of the vector component.</typeparam>
	template<typename T> using TVector1 = Vector<T, 1>;

    /// <summary>
    /// A generic vector with two components.
    /// </summary>
    /// <typeparam name="T">The type of the vector components.</typeparam>
	template<typename T> using TVector2 = Vector<T, 2>;

    /// <summary>
    /// A generic vector with three components.
    /// </summary>
    /// <typeparam name="T">The type of the vector components.</typeparam>
	template<typename T> using TVector3 = Vector<T, 3>;

    /// <summary>
    /// A generic vector with four components.
    /// </summary>
    /// <typeparam name="T">The type of the vector components.</typeparam>
	template<typename T> using TVector4 = Vector<T, 4>;

}