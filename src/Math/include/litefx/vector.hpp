#pragma once

#include <cassert>
#include <algorithm>
#include <array>
#include <vector>
#include <ranges>

namespace LiteFX::Math {

	/// @brief An algebraic vector type.
    /// The value type of a vector must be in standard layout (i.e., `std::is_standard_layout_v<T>` must evaluate to `true`). This constraint is enforced at compile time and ensures that vector types can be
    /// binary marshaled. For example, the @ref LiteFX::Graphics::Vertex type stores nothing more than a series of vectors. The standard layout constraint ensures that a set of vertices can be converted into
    /// a plain byte array and back.
	/// @tparam T The type of the vector scalar elements. Must be in standard layout (i.e., `std::is_standard_layout_v<T>` must evaluate to `true`).
	/// @tparam DIM The number of dimensions of the vector.
    template <typename T, unsigned DIM> requires 
        (DIM > 0) && std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T>
    struct Vector {
    public:
        /// @brief Stores the size of the vector.
        static constexpr size_t vec_size = DIM;

        /// @brief The type of the vector elements.
        using scalar_type = T;

        /// @brief The type of the vector itself.
        using vec_type = Vector<scalar_type, vec_size>;

    protected:
        using array_type = std::array<scalar_type, vec_size>;
        array_type m_elements = { }; // NOLINT

    public:
        /// @brief Initializes an empty vector.
        constexpr Vector() noexcept = default;

        /// @brief Initializes a vector by taking over another vector.
        ///
        /// @param _other The vector to take over.
        constexpr Vector(Vector&& _other) noexcept = default;

        /// @brief Initializes a vector with the values provided by another vector.
        ///
        /// @param _other The other vector to copy the values from.
        constexpr Vector(const Vector& _other) = default;

        /// @brief Moves the elements of the other vector to the current vector.
        ///
        /// @param _other The vector to take over.
        /// @return A reference to the current vector instance.
        constexpr Vector& operator=(Vector&& _other) noexcept = default;

        /// @brief Copys the elements of another vector into the current vector.
        ///
        /// @param _other The vector to copy the elements from.
        /// @return A reference to the current vector instance.
        constexpr Vector& operator=(const Vector& _other) = default;

        /// @brief Destroys the vector.
        constexpr ~Vector() noexcept = default;

        /// @brief Initializes a vector where all elements take the value provided by @p val.
        ///
        /// @param val The value to initialize all elements of the vector with.
        constexpr Vector(T val) noexcept {
            std::fill(std::begin(m_elements), std::end(m_elements), val);
        }

        /// @brief Initializes a 2D vector using the values provided by @p x and @p y.
        ///
        /// @param x The value to initialize the x-component of the vector with.
        /// @param y The value to initialize the y-component of the vector with.
        constexpr Vector(T x, T y) noexcept requires(DIM == 2)
        {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            m_elements[0] = x;
            m_elements[1] = y;
            // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// @brief Initializes a 3D vector using the values provided by @p x, @p y and @p z.
        ///
        /// @param x The value to initialize the x-component of the vector with.
        /// @param y The value to initialize the y-component of the vector with.
        /// @param z The value to initialize the z-component of the vector with.
        constexpr Vector(T x, T y, T z) noexcept requires(DIM == 3)
        {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            m_elements[0] = x;
            m_elements[1] = y;
            m_elements[2] = z;
            // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// @brief Initializes a 4D vector using the values provided by @p x, @p y, @p z and @p w.
        ///
        /// @param x The value to initialize the x-component of the vector with.
        /// @param y The value to initialize the y-component of the vector with.
        /// @param z The value to initialize the z-component of the vector with.
        /// @param w The value to initialize the w-component of the vector with.
        constexpr Vector(T x, T y, T z, T w) noexcept requires(DIM == 4)
        {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            m_elements[0] = x;
            m_elements[1] = y;
            m_elements[2] = z;
            m_elements[3] = w;
            // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        /// @brief Initializes the vector from an arbitrary input range.
        ///
        /// @param input The range to initialize the vector with.
        constexpr explicit Vector(std::ranges::input_range auto&& input) noexcept requires
            std::is_nothrow_convertible_v<std::ranges::range_value_t<decltype(input)>, T>
        {
            std::ranges::copy(input, std::begin(m_elements));
        }

        /// @brief Copies the values from an arbitrary input range into the current vector instance.
        ///
        /// @param input The input range to copy the values from.
        /// @return A reference to the current vector instance.
        constexpr auto& operator=(std::ranges::input_range auto&& input) noexcept requires
            std::is_nothrow_convertible_v<std::ranges::range_value_t<decltype(input)>, T>
        {
            std::ranges::copy(input, std::begin(m_elements));
            return *this;
        }

    public:
        /// @brief Returns a value from the vector, indexed by the parameter @p i.
        ///
        /// Note that this method wraps the index if it is out of range, i.e., calling the method with index `4` on a 4D vector will return the element at index `0`.
        ///
        /// @param i The index of the element to return.
        /// @return The value of the element at the provided index.
        constexpr T operator[](unsigned int i) const noexcept {
            assert(i < DIM);

            return m_elements[i % DIM]; // NOLINT
        }

        /// @brief Returns a reference to a value from the vector, indexed by the parameter @p i.
        ///
        /// Note that this method wraps the index if it is out of range, i.e., calling the method with index `4` on a 4D vector will return the element at index `0`.
        ///
        /// @param i The index of the element to return.
        /// @return A reference to a value of the element at the provided index.
        constexpr T& operator[](unsigned int i) noexcept {
            assert(i < DIM);

            return m_elements[i % DIM]; // NOLINT
        }

        /// @brief Returns an interator for that addresses the begin of the vector elements.
        ///
        /// @return An interator for that addresses the begin of the vector elements.
        constexpr auto begin() noexcept {
            return m_elements.begin();
        }

        /// @brief Returns an interator for that addresses the end of the vector elements.
        ///
        /// @return An interator for that addresses the end of the vector elements.
        constexpr auto end() noexcept {
            return m_elements.end();
        }

        /// @brief Returns a constant interator for that addresses the begin of the vector elements.
        ///
        /// @return A constant interator for that addresses the begin of the vector elements.
        constexpr auto cbegin() const noexcept {
            return m_elements.cbegin();
        }

        /// @brief Returns a constant interator for that addresses the end of the vector elements.
        ///
        /// @return A constant interator for that addresses the end of the vector elements.
        constexpr auto cend() const noexcept {
            return m_elements.cend();
        }

    public:
        /// @brief Returns a pointer to the elements of the vector.
        ///
        /// @return A pointer to the elements of the vector.
        constexpr const scalar_type* elements() const noexcept {
            return m_elements.data();
        }

        /// @brief Converts the vector to an instance of `std::array`.
        constexpr operator std::array<T, DIM>() const noexcept {
            return m_elements;
        }

        /// @brief Converts the vector into an instance of type `std::vector`.
        constexpr operator std::vector<T>() const {
            return std::vector<T>(std::begin(m_elements), std::end(m_elements));
        }

        /// @brief Returns the number of dimensions of the vector.
        ///
        /// @return The number of dimensions of the vector.
        constexpr int size() const noexcept {
            return vec_size;
        }

        /// @brief Returns the value of the x component of the vector.
        ///
        /// @return The value of the x component of the vector.
        constexpr scalar_type x() const noexcept requires (DIM > 0) {
            return m_elements[0]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        }

        /// @brief Returns a reference of the value of the x component of the vector.
        ///
        /// @return The a reference of the value of the x component of the vector.
        constexpr scalar_type& x() noexcept requires (DIM > 0) {
            return m_elements[0]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        }

        /// @brief Returns the value of the y component of the vector.
        ///
        /// @return The value of the y component of the vector.
        constexpr scalar_type y() const noexcept requires (DIM > 1) {
            return m_elements[1]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        }

        /// @brief Returns a reference of the value of the y component of the vector.
        ///
        /// @return The a reference of the value of the y component of the vector.
        constexpr scalar_type& y() noexcept requires (DIM > 1) {
            return m_elements[1]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        }

        /// @brief Returns the value of the z component of the vector.
        ///
        /// @return The value of the z component of the vector.
        constexpr scalar_type z() const noexcept requires (DIM > 2) {
            return m_elements[2]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        }

        /// @brief Returns a reference of the value of the z component of the vector.
        ///
        /// @return The a reference of the value of the z component of the vector.
        constexpr scalar_type& z() noexcept requires (DIM > 2) {
            return m_elements[2]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        }

        /// @brief Returns the value of the w component of the vector.
        ///
        /// @return The value of the w component of the vector.
        constexpr scalar_type w() const noexcept requires (DIM > 3) {
            return m_elements[3]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        }

        /// @brief Returns a reference of the value of the w component of the vector.
        ///
        /// @return The a reference of the value of the w component of the vector.
        constexpr scalar_type& w() noexcept requires (DIM > 3) {
            return m_elements[3]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        }
    };

	/// @brief A generic vector with one component.
	///
	/// @tparam T The type of the vector component.
	template<typename T> using TVector1 = Vector<T, 1>;

    /// @brief A generic vector with two components.
    ///
    /// @tparam T The type of the vector components.
	template<typename T> using TVector2 = Vector<T, 2>;

    /// @brief A generic vector with three components.
    ///
    /// @tparam T The type of the vector components.
	template<typename T> using TVector3 = Vector<T, 3>;

    /// @brief A generic vector with four components.
    ///
    /// @tparam T The type of the vector components.
	template<typename T> using TVector4 = Vector<T, 4>;

}