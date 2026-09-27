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

	/// @brief An algebraic matrix type.
	///
	/// Note that matrices in the engine are row-major by convention.
	///
	/// Matrices act as optimized storage containers only. All algebraic operations are not part of the library itself, but rather covered by supported linear algebra libraries.
	///
	/// @tparam T The type of the matrix scalar elements. Must be in standard layout (i.e., `std::is_standard_layout_v<T>` must evaluate to `true`).
	/// @tparam ROWS The number of rows of the matrix. Must be greater than 1.
	/// @tparam COLS The number of columns of the matrix. Must be greater than 1.
	template <typename T, unsigned ROWS, unsigned COLS> requires 
		(ROWS >= 2 && COLS >= 2) && std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T>
	struct Matrix final {
	public:
		/// @brief Stores the number of rows of the matrix.
		static constexpr size_t mat_rows = ROWS;

		/// @brief Stores the number of columns of the matrix.
		static constexpr size_t mat_cols = COLS;

		/// @brief The type of the matrix elements.
		using scalar_type = T;

		/// @brief The type of the matrix itself.
		using mat_type = Matrix<scalar_type, mat_rows, mat_cols>;

		/// @brief The type of the matrix, but without pre-defined dimensions.
		///
		/// @tparam rows The number of rows of the matrix.
		/// @tparam cols The number of columns of the matrix.
		template <unsigned rows, unsigned cols>
		using generic_mat_type = Matrix<scalar_type, rows, cols>;

	protected:
		using array_type = std::array<scalar_type, mat_rows * mat_cols>;
		array_type m_elements = { }; // NOLINT

	public:
		/// @brief Initializes an empty matrix.
		constexpr Matrix() noexcept = default;

		/// @brief Initializes a matrix where all elements take the value provided by @p val.
		///
		/// @param val The value to initialize all elements of the matrix with.
		constexpr Matrix(T val) noexcept {
			std::fill(std::begin(m_elements), std::end(m_elements), val);
		}

		/// @brief Initializes a matrix with an array of values.
		///
		/// @param array The array of values to take over by the matrix.
		constexpr Matrix(array_type&& array) noexcept :
			m_elements(std::move(array)) 
		{
		}

		/// @brief Initializes the matrix with a set of values.
		///
		/// @param elements The values to initialize the matrix with.
		constexpr Matrix(std::initializer_list<scalar_type> elements) noexcept {
			std::ranges::move(elements, std::begin(m_elements));
		}

		/// @brief Initializes a copy from another matrix, that might have different dimensions.
		///
		/// @tparam rows The rows of the other matrix.
		/// @tparam cols The columns of the other matrix.
		/// @param _other The other matrix.
		template <unsigned rows, unsigned cols>
		constexpr Matrix(const Matrix<scalar_type, rows, cols>& _other) {
			for (size_t r { 0 }; r < rows && r < mat_rows; ++r)
				std::ranges::copy(_other.row(r), std::begin(m_elements) + r * mat_cols);
		}

		/// @brief Initializes a matrix by taking over another matrix.
		///
		/// @param _other The matrix to take over.
		constexpr Matrix(Matrix&& _other) noexcept = default;

		/// @brief Initializes a matrix with the values provided by another matrix.
		///
		/// @param _other The other matrix to copy the values from.
		constexpr Matrix(const Matrix& _other) = default;

		/// @brief Moves the elements of the other matrix to the current matrix.
		///
		/// @param _other The matrix to take over.
		/// @return A reference to the current matrix instance.
		constexpr Matrix& operator=(Matrix&& _other) noexcept = default;

		/// @brief Copies the elements of another matrix into the current matrix.
		///
		/// @param _other The matrix to copy the elements from.
		/// @return A reference to the current matrix instance.
		constexpr Matrix& operator=(const Matrix& _other) = default;

		/// @brief Destroys the matrix instance.
		constexpr ~Matrix() noexcept = default;

		/// @brief Returns an identity matrix.
		///
		/// @return An identity matrix instance.
		constexpr static mat_type identity() noexcept {
			std::array<scalar_type, mat_rows * mat_cols> data { };

			for (size_t i = 0; i < mat_rows && i < mat_cols; ++i)
				data[i * mat_cols + i] = 1.0f; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

			return mat_type(std::move(data));
		}

	public:
		/// @brief Returns a pointer to the raw data of the matrix.
		///
		/// @return A pointer to the raw data of the matrix.
		constexpr const scalar_type* elements() const noexcept {
			return m_elements.data();
		}

		/// @brief Returns a pointer to the raw data of the matrix.
		///
		/// @return A pointer to the raw data of the matrix.
		constexpr scalar_type* elements() noexcept {
			return m_elements.data();
		}

		/// @brief Returns the number of elements of the matrix.
		///
		/// @return The number of elements of the matrix.
		consteval size_t size() const noexcept {
			return mat_rows * mat_cols;
		}

		/// @brief Returns an iterator for that addresses the begin of the matrix elements.
		///
		/// @return An iterator for that addresses the begin of the matrix elements.
		constexpr auto begin() noexcept {
			return m_elements.begin();
		}

		/// @brief Returns an iterator for that addresses the end of the matrix elements.
		///
		/// @return An iterator for that addresses the end of the matrix elements.
		constexpr auto end() noexcept {
			return m_elements.end();
		}

		/// @brief Returns a constant iterator for that addresses the begin of the matrix elements.
		///
		/// @return A constant iterator for that addresses the begin of the matrix elements.
		constexpr auto cbegin() const noexcept {
			return m_elements.cbegin();
		}

		/// @brief Returns a constant iterator for that addresses the end of the matrix elements.
		///
		/// @return A constant iterator for that addresses the end of the matrix elements.
		constexpr auto cend() const noexcept {
			return m_elements.cend();
		}

		/// @brief Returns the element at a specified position.
		///
		/// @param row The row of the element.
		/// @param col The column of the element.
		/// @return The scalar value at the provided row and column.
		constexpr scalar_type at(size_t row, size_t col) const noexcept {
			assert(row < mat_rows && col < mat_cols);

			return m_elements[row * mat_cols + col];
		};

		/// @brief Returns the element at a specified position.
		///
		/// @param row The row of the element.
		/// @param col The column of the element.
		/// @return A reference of the scalar value at the provided row and column.
		constexpr scalar_type& at(size_t row, size_t col) noexcept {
			assert(row < mat_rows && col < mat_cols);

			return m_elements[row * mat_cols + col];
		};

		/// @brief Returns a view over a row of the matrix.
		///
		/// @param row The index of the row to view.
		/// @return A view over the specified matrix row.
		constexpr std::span<const scalar_type> row(size_t row) const noexcept {
			assert(row < mat_rows);

			return std::span(m_elements.begin() + row * mat_cols, mat_cols);
		}

		/// @brief Returns a view over a row of the matrix.
		///
		/// @param row The index of the row to view.
		/// @return A view over the specified matrix row.
		constexpr std::span<scalar_type> row(size_t row) noexcept {
			assert(row < mat_rows);

			return std::span(m_elements.begin() + row * mat_cols, mat_cols);
		}

		/// @brief Returns a copy of a column over the matrix.
		///
		/// Note that this call involves a copy, which may be inefficient if done frequently. Prefer converting the matrix into an `std::mdspan` instead, if supported.
		///
		/// @param col The index of the column of the matrix.
		/// @return An array containing a copy of the specified column.
		constexpr std::array<scalar_type, mat_cols> column(size_t col) const noexcept {
			assert(col <= mat_cols);

			return m_elements | std::views::drop(col) | std::views::stride(mat_cols) | std::ranges::to<std::array<scalar_type, mat_cols>>();
		}

#ifdef __cpp_multidimensional_subscript
		/// @brief Returns an element of the matrix.
		///
		/// @param row The row of the element.
		/// @param col The column of the element.
		/// @return The scalar value at the provided row and column.
		constexpr scalar_type operator[](size_t row, size_t col) const noexcept {
			return this->at(row, col);
		}

		/// @brief Returns an element of the matrix.
		///
		/// @param row The row of the element.
		/// @param col The column of the element.
		/// @return A reference of the scalar value at the provided row and column.
		constexpr scalar_type& operator[](size_t row, size_t col) noexcept {
			return this->at(row, col);
		}
#endif

		/// @brief Returns an element of the matrix.
		///
		/// @param position The row and column position of the matrix element.
		/// @return The scalar value at the provided position.
		constexpr scalar_type operator[](std::pair<size_t, size_t> position) const noexcept {
			return this->at(position.first, position.second);
		}

		/// @brief Returns an element of the matrix.
		///
		/// @param position The row and column position of the matrix element.
		/// @return A reference of the scalar value at the provided position.
		constexpr scalar_type& operator[](std::pair<size_t, size_t> position) noexcept {
			return this->at(position.first, position.second);
		}

#ifdef __cpp_lib_mdspan
		/// @brief Converts the matrix into a multi-dimensional view over the elements.
		constexpr operator std::mdspan<const scalar_type, std::extents<std::size_t, mat_rows, mat_cols>>() const noexcept {
			return std::mdspan<const scalar_type, std::extents<std::size_t, mat_rows, mat_cols>>(m_elements.data());
		}

		/// @brief Converts the matrix into a multi-dimensional view over the elements.
		constexpr operator std::mdspan<scalar_type, std::extents<std::size_t, mat_rows, mat_cols>>() noexcept {
			return std::mdspan<scalar_type, std::extents<std::size_t, mat_rows, mat_cols>>(m_elements.data());
		}
#endif

		/// @brief Converts the matrix to an instance of `std::array`.
		constexpr operator std::array<T, mat_rows * mat_cols>() const noexcept {
			return m_elements;
		}

		/// @brief Converts the matrix into an instance of type `std::vector`.
		constexpr operator std::vector<T>() const noexcept {
			return std::vector<T>(std::begin(m_elements), std::end(m_elements));
		}

		/// @brief Converts the matrix into a linear view over the elements.
		constexpr operator std::span<const scalar_type>() const noexcept {
			return std::span(m_elements.data(), m_elements.size());
		}

		/// @brief Converts the matrix into a linear view over the elements.
		constexpr operator std::span<scalar_type>() noexcept {
			return std::span(m_elements.data(), m_elements.size());
		}

		/// @brief Returns a copy of the matrix where the elements are transposed.
		///
		/// You can use this operations, if you want to iterate all columns of the matrix in a more efficient way. Transposing effectively turns a row-major matrix into a column-major one.
		///
		/// @return A copy of the matrix where the elements are transposed.
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

		/// @brief Returns whether or not the matrix is symmetric, that is the number of rows and columns are equal.
		///
		/// @return `true`, if the matrix is symmetric and `false` otherwise.
		consteval bool symmetric() const noexcept {
			return ROWS == COLS;
		}

#ifdef LITEFX_BUILD_WITH_GLM
		// NOTE: glm stores matrices in column-major order and also initializes them this way.
	public:
		/// @brief Initializes a matrix from a glm matrix instance.
		///
		/// @param mat The glm matrix to initialize the matrix instance with.
		constexpr Matrix(const glm::mat<mat_cols, mat_rows, scalar_type>& mat) noexcept {
			for (int r { 0 }; r < mat_rows; ++r)
				for (int c { 0 }; c < mat_cols; ++c)
					m_elements[r * mat_cols + c] = mat[c][r]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
		}

		/// @brief Initializes a matrix from a glm matrix instance.
		///
		/// @param mat The glm matrix to initialize the matrix instance with.
		constexpr Matrix(glm::mat<mat_cols, mat_rows, scalar_type>&& mat) noexcept {
			for (size_t r { 0 }; r < mat_rows; ++r)
				for (size_t c { 0 }; c < mat_cols; ++c)
					m_elements[r * mat_cols + c] = std::move(mat[c][r]); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
		}

		/// @brief Converts the matrix into a glm matrix.
		///
		/// @return The glm matrix instance.
		template <unsigned rows, unsigned cols>
		constexpr operator glm::mat<cols, rows, scalar_type>() const noexcept requires (mat_rows >= rows && mat_cols >= cols) {
			std::array<scalar_type, static_cast<size_t>(cols * rows)> data;
			glm::mat<cols, rows, scalar_type> mat;

			for (size_t c { 0 }; c < cols; ++c)
				for (size_t r { 0 }; r < rows; ++r)
					data[c * mat_rows + r] = this->at(r, c);

			std::memcpy(&mat, data.data(), data.size() * sizeof(scalar_type));
			return mat;
		}

		/// @brief Converts the matrix into a 2x2 glm matrix.
		///
		/// @return The glm matrix instance.
		constexpr operator glm::mat<2, 2, scalar_type>() const noexcept requires (mat_rows >= 2 && mat_cols >= 2) {
			return glm::mat<2, 2, scalar_type>(at(0, 0), at(1, 0), at(0, 1), at(1, 1));
		}

		/// @brief Converts the matrix into a 3x2 glm matrix.
		///
		/// @return The glm matrix instance.
		constexpr operator glm::mat<2, 3, scalar_type>() const noexcept requires (mat_rows >= 3 && mat_cols >= 2) {
			return glm::mat<2, 3, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(0, 1), at(1, 1), at(2, 1));
		}

		/// @brief Converts the matrix into a 4x2 glm matrix.
		///
		/// @return The glm matrix instance.
		constexpr operator glm::mat<2, 4, scalar_type>() const noexcept requires (mat_rows >= 4 && mat_cols >= 2) {
			return glm::mat<2, 4, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(3, 0), at(0, 1), at(1, 1), at(2, 1), at(3, 1));
		}

		/// @brief Converts the matrix into a 2x3 glm matrix.
		///
		/// @return The glm matrix instance.
		constexpr operator glm::mat<3, 2, scalar_type>() const noexcept requires (mat_rows >= 2 && mat_cols >= 3) {
			return glm::mat<3, 2, scalar_type>(at(0, 0), at(1, 0), at(0, 1), at(1, 1), at(0, 2), at(1, 2));
		}

		/// @brief Converts the matrix into a 2x4 glm matrix.
		///
		/// @return The glm matrix instance.
		constexpr operator glm::mat<4, 2, scalar_type>() const noexcept requires (mat_rows >= 2 && mat_cols >= 4) {
			return glm::mat<4, 2, scalar_type>(at(0, 0), at(1, 0), at(0, 1), at(1, 1), at(0, 2), at(1, 2), at(0, 3), at(1, 3));
		}

		/// @brief Converts the matrix into a 3x3 glm matrix.
		///
		/// @return The glm matrix instance.
		constexpr operator glm::mat<3, 3, scalar_type>() const noexcept requires (mat_rows >= 3 && mat_cols >= 3) {
			return glm::mat<3, 3, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(0, 1), at(1, 1), at(2, 1), at(0, 2), at(1, 2), at(2, 2));
		}

		/// @brief Converts the matrix into a 4x3 glm matrix.
		///
		/// @return The glm matrix instance.
		constexpr operator glm::mat<3, 4, scalar_type>() const noexcept requires (mat_rows >= 4 && mat_cols >= 3) {
			return glm::mat<3, 4, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(3, 0), at(0, 1), at(1, 1), at(2, 1), at(3, 1), at(0, 2), at(1, 2), at(2, 2), at(3, 2));
		}

		/// @brief Converts the matrix into a 3x4 glm matrix.
		///
		/// @return The glm matrix instance.
		constexpr operator glm::mat<4, 3, scalar_type>() const noexcept requires (mat_rows >= 3 && mat_cols >= 4) {
			return glm::mat<4, 3, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(0, 1), at(1, 1), at(2, 1), at(0, 2), at(1, 2), at(2, 2), at(0, 3), at(1, 3), at(2, 3));
		}

		/// @brief Converts the matrix into a 4x4 glm matrix.
		///
		/// @return The glm matrix instance.
		constexpr operator glm::mat<4, 4, scalar_type>() const noexcept requires (mat_rows >= 4 && mat_cols >= 4) {
			return glm::mat<4, 4, scalar_type>(at(0, 0), at(1, 0), at(2, 0), at(3, 0), at(0, 1), at(1, 1), at(2, 1), at(3, 1), at(0, 2), at(1, 2), at(2, 2), at(3, 2), at(0, 3), at(1, 3), at(2, 3), at(3, 3));
		}
#endif // LITEFX_BUILD_WITH_GLM

#ifdef LITEFX_BUILD_WITH_DIRECTX_MATH
	public:
		/// @brief Initializes a matrix from a DirectX matrix instance.
		///
		/// @param mat The DirectX matrix to initialize the matrix instance with.
		constexpr Matrix(const DirectX::XMFLOAT3X3& mat) noexcept {
			for (int r { 0 }; r < 3; ++r)
				for (int c { 0 }; c < 3; ++c)
					at(r, c) = mat(r, c);
		}

		/// @brief Initializes a matrix from a DirectX matrix instance.
		///
		/// @param mat The DirectX matrix to initialize the matrix instance with.
		constexpr Matrix(const DirectX::XMFLOAT4X3& mat) noexcept {
			for (int r { 0 }; r < 4; ++r)
				for (int c { 0 }; c < 3; ++c)
					at(r, c) = mat(r, c);
		}

		/// @brief Initializes a matrix from a DirectX matrix instance.
		///
		/// @param mat The DirectX matrix to initialize the matrix instance with.
		constexpr Matrix(const DirectX::XMFLOAT3X4& mat) noexcept {
			for (int r { 0 }; r < 3; ++r)
				for (int c { 0 }; c < 4; ++c)
					at(r, c) = mat(r, c);
		}

		/// @brief Initializes a matrix from a DirectX matrix instance.
		///
		/// @param mat The DirectX matrix to initialize the matrix instance with.
		constexpr Matrix(const DirectX::XMFLOAT4X4& mat) noexcept {
			for (int r { 0 }; r < 4; ++r)
				for (int c { 0 }; c < 4; ++c)
					at(r, c) = mat(r, c);
		}

		/// @brief Converts the matrix into a DirectX matrix.
		///
		/// @return The DirectX matrix instance.
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

		/// @brief Converts the matrix into a 3x3 DirectX matrix.
		///
		/// @return The DirectX matrix instance.
		constexpr operator DirectX::XMFLOAT3X3() const noexcept requires (mat_rows >= 3 && mat_cols >= 3 && std::convertible_to<scalar_type, float>) {
			return DirectX::XMFLOAT3X3(at(0, 0), at(0, 1), at(0, 2), at(1, 0), at(1, 2), at(2, 0), at(2, 1), at(2, 2));
		}

		/// @brief Converts the matrix into a 4x3 DirectX matrix.
		///
		/// @return The DirectX matrix instance.
		constexpr operator DirectX::XMFLOAT4X3() const noexcept requires (mat_rows >= 4 && mat_cols >= 3 && std::convertible_to<scalar_type, float>) {
			return DirectX::XMFLOAT4X3(at(0, 0), at(0, 1), at(0, 2), at(1, 0), at(1, 2), at(2, 0), at(2, 1), at(2, 2), at(3, 0), at(3, 1), at(3, 2));
		}

		/// @brief Converts the matrix into a 3x4 DirectX matrix.
		///
		/// @return The DirectX matrix instance.
		constexpr operator DirectX::XMFLOAT3X4() const noexcept requires (mat_rows >= 3 && mat_cols >= 4 && std::convertible_to<scalar_type, float>) {
			return DirectX::XMFLOAT3X4(at(0, 0), at(0, 1), at(0, 2), at(0, 3), at(1, 0), at(1, 2), at(1, 3), at(2, 0), at(2, 1), at(2, 2), at(2, 3));
		}

		/// @brief Converts the matrix into a 4x4 DirectX matrix.
		///
		/// @return The DirectX matrix instance.
		constexpr operator DirectX::XMFLOAT4X4() const noexcept requires (mat_rows >= 4 && mat_cols >= 4 && std::convertible_to<scalar_type, float>) {
			return DirectX::XMFLOAT4X4(at(0, 0), at(0, 1), at(0, 2), at(0, 3), at(1, 0), at(1, 2), at(1, 3), at(2, 0), at(2, 1), at(2, 2), at(2, 3), at(3, 0), at(3, 1), at(3, 2), at(3, 3));
		}
#endif // LITEFX_BUILD_WITH_DIRECTX_MATH
	};

	/// @brief A generic matrix with 2 rows and 2 columns.
	///
	/// @tparam T The type of the matrix elements.
	template<typename T> using TMatrix2   = Matrix<T, 2, 2>;

	/// @brief A generic matrix with 3 rows and 3 columns.
	///
	/// @tparam T The type of the matrix elements.
	template<typename T> using TMatrix3   = Matrix<T, 3, 3>;

	/// @brief A generic matrix with 4 rows and 4 columns.
	///
	/// @tparam T The type of the matrix elements.
	template<typename T> using TMatrix4   = Matrix<T, 4, 4>;

	/// @brief A generic matrix with 3 rows and 4 columns.
	///
	/// @tparam T The type of the matrix elements.
	template<typename T> using TMatrix3x4 = Matrix<T, 3, 4>;

}