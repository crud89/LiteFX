#pragma once

#include <cassert>
#include <algorithm>
#include <utility>
#include <span>
#include <array>
#include <vector>
#include <ranges>
#include <initializer_list>

#ifdef __cpp_lib_mdspan
#include <mdspan>
#endif

#ifdef LITEFX_BUILD_WITH_GLM
#include <glm/matrix.hpp>
#endif

#ifdef LITEFX_BUILD_WITH_DIRECTX_MATH
#include <DirectXMath.h>
#endif

namespace LiteFX::Math {

	/// <summary>
	/// An algebraic matrix type.
	/// </summary>
	/// <remarks>
	/// Note that matrices in the engine are row-major by convention. 
	/// 
	/// Matrices act as optimized storage containers only. All algebraic operations are not part of the library itself, but rather covered by supported linear algebra libraries.
	/// </remarks>
	/// <typeparam name="T">The type of the matrix scalar elements. Must be in standard layout (i.e., `std::is_standard_layout_v<T>` must evaluate to `true`).</typeparam>
	/// <typeparam name="ROWS">The number of rows of the matrix. Must be greater than 1.</typeparam>
	/// <typeparam name="COLS">The number of columns of the matrix. Must be greater than 1.</typeparam>
	template <typename T, unsigned ROWS, unsigned COLS> requires 
		(ROWS >= 2 && COLS >= 2) && std::is_standard_layout_v<T>
	struct Matrix final {
	public:
		/// <summary>
		/// Stores the number of rows of the matrix.
		/// </summary>
		static constexpr size_t mat_rows = ROWS;

		/// <summary>
		/// Stores the number of columns of the matrix.
		/// </summary>
		static constexpr size_t mat_cols = COLS;

		/// <summary>
		/// The type of the matrix elements.
		/// </summary>
		using scalar_type = T;

		/// <summary>
		/// The type of the matrix itself.
		/// </summary>
		using mat_type = Matrix<scalar_type, mat_rows, mat_cols>;

		/// <summary>
		/// The type of the matrix, but without pre-defined dimensions.
		/// </summary>
		/// <typeparam name="rows">The number of rows of the matrix.</typeparam>
		/// <typeparam name="cols">The number of columns of the matrix.</typeparam>
		template <unsigned rows, unsigned cols>
		using generic_mat_type = Matrix<scalar_type, rows, cols>;

	protected:
		using array_type = std::array<scalar_type, mat_rows * mat_cols>;
		array_type m_elements = { }; // NOLINT

	public:
		/// <summary>
		/// Initializes an empty matrix.
		/// </summary>
		constexpr Matrix() noexcept = default;

		/// <summary>
		/// Initializes a matrix where all elements take the value provided by <paramref name="val" />.
		/// </summary>
		/// <param name="val">The value to initialize all elements of the matrix with.</param>
		constexpr Matrix(T val) noexcept {
			std::fill(std::begin(m_elements), std::end(m_elements), val);
		}

		/// <summary>
		/// Initializes a matrix with an array of values.
		/// </summary>
		/// <param name="array">The array of values to take over by the matrix.</param>
		constexpr Matrix(array_type&& array) noexcept :
			m_elements(std::move(array)) 
		{
		}

		/// <summary>
		/// Initializes the matrix with a set of values.
		/// </summary>
		/// <param name="elements">The values to initialize the matrix with.</param>
		constexpr Matrix(std::initializer_list<scalar_type> elements) noexcept {
			std::ranges::move(elements, std::begin(m_elements));
		};

		/// <summary>
		/// Initializes a copy from another matrix, that might have different dimensions.
		/// </summary>
		/// <typeparam name="rows">The rows of the other matrix.</typeparam>
		/// <typeparam name="cols">The columns of the other matrix.</typeparam>
		/// <param name="_other">The other matrix.</param>
		template <unsigned rows, unsigned cols>
		constexpr Matrix(const Matrix<scalar_type, rows, cols>& _other) noexcept {
			for (size_t r { 0 }; r < rows && r < mat_rows; ++r)
				std::ranges::copy(_other.row(r), std::begin(m_elements) + r * mat_cols);
		}

		/// <summary>
		/// Initializes a matrix with the values provided by another matrix.
		/// </summary>
		/// <param name="_other">The other matrix to copy the values from.</param>
		constexpr Matrix(const Matrix<scalar_type, ROWS, COLS>& _other) noexcept {
			std::ranges::copy(_other.m_elements, std::begin(m_elements));
		}

		/// <summary>
		/// Initializes a matrix by taking over another matrix.
		/// </summary>
		/// <param name="_other">The matrix to take over.</param>
		constexpr Matrix(Matrix<scalar_type, ROWS, COLS>&& _other) noexcept :
			m_elements{ std::move(_other.m_elements) } { }

		/// <summary>
		/// Destroys the matrix instance.
		/// </summary>
		constexpr ~Matrix() noexcept = default;

		/// <summary>
		/// Copies the elements of another matrix into the current matrix.
		/// </summary>
		/// <param name="_other">The matrix to copy the elements from.</param>
		/// <returns>A reference to the current matrix instance.</returns>
		constexpr auto& operator=(const Matrix<scalar_type, ROWS, COLS>& _other) noexcept {
			std::ranges::copy(_other.m_elements, std::begin(m_elements));
			return *this;
		}

		/// <summary>
		/// Moves the elements of the other matrix to the current matrix.
		/// </summary>
		/// <param name="_other">The matrix to take over.</param>
		/// <returns>A reference to the current matrix instance.</returns>
		constexpr auto& operator=(Matrix<scalar_type, ROWS, COLS>&& _other) noexcept {
			m_elements = std::move(_other.m_elements);
			return *this;
		}

		/// <summary>
		/// Returns an identity matrix.
		/// </summary>
		/// <returns>An identity matrix instance.</returns>
		constexpr static mat_type identity() noexcept {
			std::array<scalar_type, mat_rows * mat_cols> data { };

			for (size_t i = 0; i < mat_rows && i < mat_cols; ++i)
				data[i * mat_cols + i] = 1.0f; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)

			return mat_type(std::move(data));
		}

	public:
		/// <summary>
		/// Returns a pointer to the raw data of the matrix.
		/// </summary>
		/// <returns>A pointer to the raw data of the matrix.</returns>
		constexpr const scalar_type* elements() const noexcept {
			return m_elements.data();
		}

		/// <summary>
		/// Returns a pointer to the raw data of the matrix.
		/// </summary>
		/// <returns>A pointer to the raw data of the matrix.</returns>
		constexpr scalar_type* elements() noexcept {
			return m_elements.data();
		}

		/// <summary>
		/// Returns the number of elements of the matrix.
		/// </summary>
		/// <returns>The number of elements of the matrix.</returns>
		consteval size_t size() const noexcept {
			return mat_rows * mat_cols;
		}

		/// <summary>
		/// Returns an iterator for that addresses the begin of the matrix elements.
		/// </summary>
		/// <returns>An iterator for that addresses the begin of the matrix elements.</returns>
		constexpr auto begin() noexcept {
			return m_elements.begin();
		}

		/// <summary>
		/// Returns an iterator for that addresses the end of the matrix elements.
		/// </summary>
		/// <returns>An iterator for that addresses the end of the matrix elements.</returns>
		constexpr auto end() noexcept {
			return m_elements.end();
		}

		/// <summary>
		/// Returns a constant iterator for that addresses the begin of the matrix elements.
		/// </summary>
		/// <returns>A constant iterator for that addresses the begin of the matrix elements.</returns>
		constexpr auto cbegin() const noexcept {
			return m_elements.cbegin();
		}

		/// <summary>
		/// Returns a constant iterator for that addresses the end of the matrix elements.
		/// </summary>
		/// <returns>A constant iterator for that addresses the end of the matrix elements.</returns>
		constexpr auto cend() const noexcept {
			return m_elements.cend();
		}

		/// <summary>
		/// Returns the element at a specified position.
		/// </summary>
		/// <param name="row">The row of the element.</param>
		/// <param name="col">The column of the element.</param>
		/// <returns>The scalar value at the provided row and column.</returns>
		constexpr scalar_type at(size_t row, size_t col) const noexcept {
			assert(row < mat_rows && col < mat_cols);

			return m_elements[row * mat_cols + col];
		};

		/// <summary>
		/// Returns the element at a specified position.
		/// </summary>
		/// <param name="row">The row of the element.</param>
		/// <param name="col">The column of the element.</param>
		/// <returns>A reference of the scalar value at the provided row and column.</returns>
		constexpr scalar_type& at(size_t row, size_t col) noexcept {
			assert(row < mat_rows && col < mat_cols);

			return m_elements[row * mat_cols + col];
		};

		/// <summary>
		/// Returns a view over a row of the matrix.
		/// </summary>
		/// <param name="row">The index of the row to view.</param>
		/// <returns>A view over the specified matrix row.</returns>
		constexpr std::span<const scalar_type> row(size_t row) const noexcept {
			assert(row < mat_rows);

			return std::span(m_elements.begin() + row * mat_cols, mat_cols);
		}

		/// <summary>
		/// Returns a view over a row of the matrix.
		/// </summary>
		/// <param name="row">The index of the row to view.</param>
		/// <returns>A view over the specified matrix row.</returns>
		constexpr std::span<scalar_type> row(size_t row) noexcept {
			assert(row < mat_rows);

			return std::span(m_elements.begin() + row * mat_cols, mat_cols);
		}

		/// <summary>
		/// Returns a copy of a column over the matrix.
		/// </summary>
		/// <remarks>
		/// Note that this call involves a copy, which may be inefficient if done frequently. Prefer converting the matrix into an `std::mdspan`
		/// instead, if supported.
		/// </remarks>
		/// <param name="col">The index of the column of the matrix.</param>
		/// <returns>An array containing a copy of the specified column.</returns>
		constexpr std::array<scalar_type, mat_cols> column(size_t col) const noexcept {
			assert(col <= mat_cols);

			return m_elements | std::views::drop(col) | std::views::stride(mat_cols) | std::ranges::to<std::array<scalar_type, mat_cols>>();
		}

#ifdef __cpp_multidimensional_subscript
		/// <summary>
		/// Returns an element of the matrix.
		/// </summary>
		/// <param name="row">The row of the element.</param>
		/// <param name="col">The column of the element.</param>
		/// <returns>The scalar value at the provided row and column.</returns>
		constexpr scalar_type operator[](size_t row, size_t col) const noexcept {
			return this->at(row, col);
		}

		/// <summary>
		/// Returns an element of the matrix.
		/// </summary>
		/// <param name="row">The row of the element.</param>
		/// <param name="col">The column of the element.</param>
		/// <returns>A reference of the scalar value at the provided row and column.</returns>
		constexpr scalar_type& operator[](size_t row, size_t col) noexcept {
			return this->at(row, col);
		}
#endif

		/// <summary>
		/// Returns an element of the matrix.
		/// </summary>
		/// <param name="position">The row and column position of the matrix element.</param>
		/// <returns>The scalar value at the provided position.</returns>
		constexpr scalar_type operator[](std::pair<size_t, size_t> position) const noexcept {
			return this->at(position.first, position.second);
		}

		/// <summary>
		/// Returns an element of the matrix.
		/// </summary>
		/// <param name="position">The row and column position of the matrix element.</param>
		/// <returns>A reference of the scalar value at the provided position.</returns>
		constexpr scalar_type& operator[](std::pair<size_t, size_t> position) noexcept {
			return this->at(position.first, position.second);
		}

#ifdef __cpp_lib_mdspan
		/// <summary>
		/// Converts the matrix into a multi-dimensional view over the elements.
		/// </summary>
		constexpr operator std::mdspan<const scalar_type, std::extents<std::size_t, mat_rows, mat_cols>>() const noexcept {
			return std::mdspan<const scalar_type, std::extents<std::size_t, mat_rows, mat_cols>>(m_elements.data());
		}

		/// <summary>
		/// Converts the matrix into a multi-dimensional view over the elements.
		/// </summary>
		constexpr operator std::mdspan<scalar_type, std::extents<std::size_t, mat_rows, mat_cols>>() noexcept {
			return std::mdspan<scalar_type, std::extents<std::size_t, mat_rows, mat_cols>>(m_elements.data());
		}
#endif

		/// <summary>
		/// Converts the matrix to an instance of `std::array`.
		/// </summary>
		constexpr operator std::array<T, mat_rows * mat_cols>() const noexcept {
			return m_elements;
		}

		/// <summary>
		/// Converts the matrix into an instance of type `std::vector`.
		/// </summary>
		constexpr operator std::vector<T>() const noexcept {
			return std::vector<T>(std::begin(m_elements), std::end(m_elements));
		}

		/// <summary>
		/// Converts the matrix into a linear view over the elements.
		/// </summary>
		constexpr operator std::span<const scalar_type>() const noexcept {
			return std::span(m_elements.data(), m_elements.size());
		}

		/// <summary>
		/// Converts the matrix into a linear view over the elements.
		/// </summary>
		constexpr operator std::span<scalar_type>() noexcept {
			return std::span(m_elements.data(), m_elements.size());
		}

		/// <summary>
		/// Returns a copy of the matrix where the elements are transposed.
		/// </summary>
		/// <remarks>
		/// You can use this operations, if you want to iterate all columns of the matrix in a more efficient way. Transposing effectively turns a row-major 
		/// matrix into a column-major one.
		/// </remarks>
		/// <returns>A copy of the matrix where the elements are transposed.</returns>
		constexpr generic_mat_type<mat_cols, mat_rows> transpose() const noexcept {
			std::array<scalar_type, mat_cols * mat_rows> data { };

			for (int r{ 0 }; r < mat_rows; ++r)
			{
				auto row = this->row(r);

				for (int c{ 0 }; c < mat_cols; ++c)
					data[c * mat_rows + r] = row[c];
			}

			return generic_mat_type<mat_cols, mat_rows>(std::move(data));
		}

		/// <summary>
		/// Returns whether or not the matrix is symmetric, that is the number of rows and columns are equal.
		/// </summary>
		/// <returns>`true`, if the matrix is symmetric and `false` otherwise.</returns>
		consteval bool symmetric() const noexcept {
			return ROWS == COLS;
		}

#ifdef LITEFX_BUILD_WITH_GLM
		// NOTE: glm stores matrices in column-major order and also initializes them this way.
	public:
		/// <summary>
		/// Initializes a matrix from a glm matrix instance.
		/// </summary>
		/// <param name="mat">The glm matrix to initialize the matrix instance with.</param>
		constexpr Matrix(const glm::mat<mat_cols, mat_rows, scalar_type>& mat) noexcept {
			for (int r { 0 }; r < mat_rows; ++r)
				for (int c { 0 }; c < mat_cols; ++c)
					m_elements[r * mat_cols + c] = mat[c][r];
		}

		/// <summary>
		/// Initializes a matrix from a glm matrix instance.
		/// </summary>
		/// <param name="mat">The glm matrix to initialize the matrix instance with.</param>
		constexpr Matrix(glm::mat<mat_cols, mat_rows, scalar_type>&& mat) noexcept {
			for (size_t r { 0 }; r < mat_rows; ++r)
				for (size_t c { 0 }; c < mat_cols; ++c)
					m_elements[r * mat_cols + c] = std::move(mat[c][r]);
		}

		/// <summary>
		/// Converts the matrix into a glm matrix.
		/// </summary>
		/// <returns>The glm matrix instance.</returns>
		template <unsigned rows, unsigned cols>
		constexpr operator glm::mat<cols, rows, scalar_type>() const noexcept requires (mat_rows >= rows && mat_cols >= cols) {
			std::array<scalar_type, cols * rows> data;
			glm::mat<cols, rows, scalar_type> mat;

			for (size_t c { 0 }; c < cols; ++c)
				for (size_t r { 0 }; r < rows; ++r)
					data[c * mat_rows + r] = this->at(r, c);

			std::memcpy(&mat, data.data(), data.size() * sizeof(scalar_type));
			return mat;
		}

		/// <summary>
		/// Converts the matrix into a 2x2 glm matrix.
		/// </summary>
		/// <returns>The glm matrix instance.</returns>
		constexpr operator glm::mat<2, 2, scalar_type>() const noexcept requires (mat_rows >= 2 && mat_cols >= 2) {
			return glm::mat<2, 2, scalar_type>(at(0, 0), at(1, 0), at(0, 1), at(1, 1));
		}

		/// <summary>
		/// Converts the matrix into a 3x2 glm matrix.
		/// </summary>
		/// <returns>The glm matrix instance.</returns>
		constexpr operator glm::mat<2, 3, scalar_type>() const noexcept requires (mat_rows >= 3 && mat_cols >= 2) {
			return glm::mat<2, 3, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(0, 1), at(1, 1), at(2, 1));
		}

		/// <summary>
		/// Converts the matrix into a 4x2 glm matrix.
		/// </summary>
		/// <returns>The glm matrix instance.</returns>
		constexpr operator glm::mat<2, 4, scalar_type>() const noexcept requires (mat_rows >= 4 && mat_cols >= 2) {
			return glm::mat<2, 4, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(3, 0), at(0, 1), at(1, 1), at(2, 1), at(3, 1));
		}

		/// <summary>
		/// Converts the matrix into a 2x3 glm matrix.
		/// </summary>
		/// <returns>The glm matrix instance.</returns>
		constexpr operator glm::mat<3, 2, scalar_type>() const noexcept requires (mat_rows >= 2 && mat_cols >= 3) {
			return glm::mat<3, 2, scalar_type>(at(0, 0), at(1, 0), at(0, 1), at(1, 1), at(0, 2), at(1, 2));
		}

		/// <summary>
		/// Converts the matrix into a 2x4 glm matrix.
		/// </summary>
		/// <returns>The glm matrix instance.</returns>
		constexpr operator glm::mat<4, 2, scalar_type>() const noexcept requires (mat_rows >= 2 && mat_cols >= 4) {
			return glm::mat<4, 2, scalar_type>(at(0, 0), at(1, 0), at(0, 1), at(1, 1), at(0, 2), at(1, 2), at(0, 3), at(1, 3));
		}

		/// <summary>
		/// Converts the matrix into a 3x3 glm matrix.
		/// </summary>
		/// <returns>The glm matrix instance.</returns>
		constexpr operator glm::mat<3, 3, scalar_type>() const noexcept requires (mat_rows >= 3 && mat_cols >= 3) {
			return glm::mat<3, 3, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(0, 1), at(1, 1), at(2, 1), at(0, 2), at(1, 2), at(2, 2));
		}

		/// <summary>
		/// Converts the matrix into a 4x3 glm matrix.
		/// </summary>
		/// <returns>The glm matrix instance.</returns>
		constexpr operator glm::mat<3, 4, scalar_type>() const noexcept requires (mat_rows >= 4 && mat_cols >= 3) {
			return glm::mat<3, 4, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(3, 0), at(0, 1), at(1, 1), at(2, 1), at(3, 1), at(0, 2), at(1, 2), at(2, 2), at(3, 2));
		}

		/// <summary>
		/// Converts the matrix into a 3x4 glm matrix.
		/// </summary>
		/// <returns>The glm matrix instance.</returns>
		constexpr operator glm::mat<4, 3, scalar_type>() const noexcept requires (mat_rows >= 3 && mat_cols >= 4) {
			return glm::mat<4, 3, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(0, 1), at(1, 1), at(2, 1), at(0, 2), at(1, 2), at(2, 2), at(0, 3), at(1, 3), at(2, 3));
		}

		/// <summary>
		/// Converts the matrix into a 4x4 glm matrix.
		/// </summary>
		/// <returns>The glm matrix instance.</returns>
		constexpr operator glm::mat<4, 4, scalar_type>() const noexcept requires (mat_rows >= 4 && mat_cols >= 4) {
			return glm::mat<4, 4, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(3, 0), at(0, 1), at(1, 1), at(2, 1), at(3, 1), at(0, 2), at(1, 2), at(2, 2), at(3, 2), at(0, 3), at(1, 3), at(2, 3), at(3, 3));
		}
#endif // LITEFX_BUILD_WITH_GLM

#ifdef LITEFX_BUILD_WITH_DIRECTX_MATH
	public:
		/// <summary>
		/// Initializes a matrix from a DirectX matrix instance.
		/// </summary>
		/// <param name="mat">The DirectX matrix to initialize the matrix instance with.</param>
		constexpr Matrix(const DirectX::XMFLOAT3X3& mat) noexcept {
			for (int r { 0 }; r < 3; ++r)
				for (int c { 0 }; c < 3; ++c)
					at(r, c) = mat(r, c);
		}

		/// <summary>
		/// Initializes a matrix from a DirectX matrix instance.
		/// </summary>
		/// <param name="mat">The DirectX matrix to initialize the matrix instance with.</param>
		constexpr Matrix(const DirectX::XMFLOAT4X3& mat) noexcept {
			for (int r { 0 }; r < 4; ++r)
				for (int c { 0 }; c < 3; ++c)
					at(r, c) = mat(r, c);
		}

		/// <summary>
		/// Initializes a matrix from a DirectX matrix instance.
		/// </summary>
		/// <param name="mat">The DirectX matrix to initialize the matrix instance with.</param>
		constexpr Matrix(const DirectX::XMFLOAT3X4& mat) noexcept {
			for (int r { 0 }; r < 3; ++r)
				for (int c { 0 }; c < 4; ++c)
					at(r, c) = mat(r, c);
		}

		/// <summary>
		/// Initializes a matrix from a DirectX matrix instance.
		/// </summary>
		/// <param name="mat">The DirectX matrix to initialize the matrix instance with.</param>
		constexpr Matrix(const DirectX::XMFLOAT4X4& mat) noexcept {
			for (int r { 0 }; r < 4; ++r)
				for (int c { 0 }; c < 4; ++c)
					at(r, c) = mat(r, c);
		}

		/// <summary>
		/// Converts the matrix into a DirectX matrix.
		/// </summary>
		/// <returns>The DirectX matrix instance.</returns>
		constexpr operator DirectX::XMMATRIX() const noexcept requires ((mat_rows == 3 || mat_rows == 4) && (mat_cols == 3 || mat_cols == 4) && std::convertible_to<scalar_type, float>) {
			if constexpr (mat_rows == 3 && mat_cols == 3)
			{
				DirectX::XMFLOAT3X3 mat = static_cast<DirectX::XMFLOAT3X3>(*this);
				return DirectX::XMLoadFloat3x3(&mat);
			}
			else if constexpr (mat_rows == 3 && mat_cols == 4)
			{
				DirectX::XMFLOAT3X4 mat = static_cast<DirectX::XMFLOAT3X4>(*this);
				return DirectX::XMLoadFloat3x4(&mat);
			}
			else if constexpr (mat_rows == 4 && mat_cols == 3)
			{
				DirectX::XMFLOAT4X3 mat = static_cast<DirectX::XMFLOAT4X3>(*this);
				return DirectX::XMLoadFloat4x3(&mat);
			}
			if constexpr (mat_rows == 4 && mat_cols == 4)
			{
				DirectX::XMFLOAT4X4 mat = static_cast<DirectX::XMFLOAT4X4>(*this);
				return DirectX::XMLoadFloat4x4(&mat);
			}

			std::unreachable();
		}

		/// <summary>
		/// Converts the matrix into a 3x3 DirectX matrix.
		/// </summary>
		/// <returns>The DirectX matrix instance.</returns>
		constexpr operator DirectX::XMFLOAT3X3() const noexcept requires (mat_rows >= 3 && mat_cols >= 3 && std::convertible_to<scalar_type, float>) {
			return DirectX::XMFLOAT3X3(at(0, 0), at(0, 1), at(0, 2), at(1, 0), at(1, 2), at(2, 0), at(2, 1), at(2, 2));
		}

		/// <summary>
		/// Converts the matrix into a 4x3 DirectX matrix.
		/// </summary>
		/// <returns>The DirectX matrix instance.</returns>
		constexpr operator DirectX::XMFLOAT4X3() const noexcept requires (mat_rows >= 4 && mat_cols >= 3 && std::convertible_to<scalar_type, float>) {
			return DirectX::XMFLOAT4X3(at(0, 0), at(0, 1), at(0, 2), at(1, 0), at(1, 2), at(2, 0), at(2, 1), at(2, 2), at(3, 0), at(3, 1), at(3, 2));
		}

		/// <summary>
		/// Converts the matrix into a 3x4 DirectX matrix.
		/// </summary>
		/// <returns>The DirectX matrix instance.</returns>
		constexpr operator DirectX::XMFLOAT3X4() const noexcept requires (mat_rows >= 3 && mat_cols >= 4 && std::convertible_to<scalar_type, float>) {
			return DirectX::XMFLOAT3X4(at(0, 0), at(0, 1), at(0, 2), at(0, 3), at(1, 0), at(1, 2), at(1, 3), at(2, 0), at(2, 1), at(2, 2), at(2, 3));
		}

		/// <summary>
		/// Converts the matrix into a 4x4 DirectX matrix.
		/// </summary>
		/// <returns>The DirectX matrix instance.</returns>
		constexpr operator DirectX::XMFLOAT4X4() const noexcept requires (mat_rows >= 4 && mat_cols >= 4 && std::convertible_to<scalar_type, float>) {
			return DirectX::XMFLOAT4X4(at(0, 0), at(0, 1), at(0, 2), at(0, 3), at(1, 0), at(1, 2), at(1, 3), at(2, 0), at(2, 1), at(2, 2), at(2, 3), at(3, 0), at(3, 1), at(3, 2), at(3, 3));
		}
#endif // LITEFX_BUILD_WITH_DIRECTX_MATH
	};

	/// <summary>
	/// A generic matrix with 2 rows and 2 columns.
	/// </summary>
	/// <typeparam name="T">The type of the matrix elements.</typeparam>
	template<typename T> using TMatrix2   = Matrix<T, 2, 2>;

	/// <summary>
	/// A generic matrix with 3 rows and 3 columns.
	/// </summary>
	/// <typeparam name="T">The type of the matrix elements.</typeparam>
	template<typename T> using TMatrix3   = Matrix<T, 3, 3>;

	/// <summary>
	/// A generic matrix with 4 rows and 4 columns.
	/// </summary>
	/// <typeparam name="T">The type of the matrix elements.</typeparam>
	template<typename T> using TMatrix4   = Matrix<T, 4, 4>;

	/// <summary>
	/// A generic matrix with 3 rows and 4 columns.
	/// </summary>
	/// <typeparam name="T">The type of the matrix elements.</typeparam>
	template<typename T> using TMatrix3x4 = Matrix<T, 3, 4>;

}