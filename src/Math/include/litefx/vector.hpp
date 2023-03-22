#pragma once

#include <cassert>
#include <algorithm>

namespace LiteFX::Math {

	template <typename T, unsigned DIM>
	class Vector {
	public:
		static constexpr size_t vec_size = DIM;
		using scalar_type = T;
		using vec_type = Vector<scalar_type, vec_size>;

	protected:
		scalar_type m_elements[vec_size] = { };

	public:
		Vector() noexcept = default;

		Vector(const T& val) noexcept {
			std::fill(std::begin(m_elements), std::end(m_elements), val);
		}

		inline Vector(const vec_type& _other) noexcept { operator=(_other); }
		inline Vector(vec_type&& _other) noexcept { operator=(_other); }
		//virtual inline ~Vector() noexcept = default;

		Vector(T x, T y) noexcept requires(DIM == 2) {
			m_elements[0] = x;
			m_elements[1] = y;
		};

		Vector(T x, T y, T z) noexcept requires(DIM == 3) {
			m_elements[0] = x;
			m_elements[1] = y;
			m_elements[2] = z;
		};

		Vector(T x, T y, T z, T w) noexcept requires(DIM == 4) {
			m_elements[0] = x;
			m_elements[1] = y;
			m_elements[2] = z;
			m_elements[3] = w;
		};

	public:
		Vector<T, DIM>& operator= (const Vector<T, DIM>& _other) noexcept {
			std::copy(std::begin(_other.m_elements), std::end(_other.m_elements), std::begin(m_elements));
			return *this;
		}

		Vector<T, DIM>& operator= (Vector<T, DIM>&& _other) noexcept {
			std::move(std::begin(_other.m_elements), std::end(_other.m_elements), std::begin(m_elements));
			return *this;
		}

		inline const T& operator[](const unsigned int& i) const noexcept {
			assert(i < DIM);

			return m_elements[i];
		}

		inline T& operator[](const unsigned int& i) noexcept {
			assert(i < DIM);

			return m_elements[i];
		}

	public:
		inline const scalar_type* elements() const noexcept {
			return m_elements;
		}

		inline int size() const noexcept {
			return vec_size;
		}

		inline const scalar_type& x() const noexcept requires (DIM > 0) {
			return m_elements[0];
		}

		inline scalar_type& x() noexcept requires (DIM > 0) {
			return m_elements[0];
		}

		inline const scalar_type& y() const noexcept requires (DIM > 1) {
			return m_elements[1];
		}

		inline scalar_type& y() noexcept requires (DIM > 1) {
			return m_elements[1];
		}

		inline const scalar_type& z() const noexcept requires (DIM > 2) {
			return m_elements[2];
		}

		inline scalar_type& z() noexcept requires (DIM > 2) {
			return m_elements[2];
		}

		inline const scalar_type& w() const noexcept requires (DIM > 3) {
			return m_elements[3];
		}

		inline scalar_type& w() noexcept requires (DIM > 3) {
			return m_elements[3];
		}
	};

	template<typename T> using TVector1 = Vector<T, 1>;
	template<typename T> using TVector2 = Vector<T, 2>;
	template<typename T> using TVector3 = Vector<T, 3>;
	template<typename T> using TVector4 = Vector<T, 4>;

}