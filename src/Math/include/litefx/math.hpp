#pragma once

#include <litefx/core.h>

#if !defined (LITEFX_MATH_API)
#  if defined(LiteFX_Math_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_MATH_API __declspec(dllexport)
#  elif (defined(LiteFX_Math_EXPORTS) || defined(__APPLE__)) && defined __GNUC__ && __GNUC__ >= 4
#    define LITEFX_MATH_API __attribute__ ((visibility ("default")))
#  elif !defined(LiteFX_Math_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_MATH_API __declspec(dllimport)
#  endif
#endif

#ifndef LITEFX_MATH_API
#  define LITEFX_MATH_API
#endif

#if defined(BUILD_ENABLE_GLM)
#include <glm/glm.hpp>
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
#include <DirectXMath.h>
#endif

namespace LiteFX::Math {
	using namespace LiteFX;

	template <typename T, int DIM> class LITEFX_MATH_API Vector;

	using Byte = uint8_t;
	using Int16 = int16_t;
	using UInt16 = uint16_t;
	using Int32 = int32_t;
	using UInt32 = uint32_t;
	using Int64 = int64_t;
	using UInt64 = uint64_t;
	using Float = float_t;
	using Double = double_t;

	template <>
	class LITEFX_MATH_API Vector<Float, 1> {
	public:
		static constexpr size_t vec_size = 1;
		using scalar_type = Float;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const scalar_type& x) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type* elements() const noexcept;
		inline scalar_type* elements() noexcept;
		inline const scalar_type& operator[](const int& i) const noexcept;
		inline scalar_type& operator[](const int& i) noexcept;
		inline const scalar_type& x() const;
		inline scalar_type& x();

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::f32vec1& v) noexcept;
		inline operator glm::f32vec1() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		inline operator DirectX::XMVECTOR() noexcept;
		inline operator scalar_type() noexcept;
#endif
	};

	template <>
	class LITEFX_MATH_API Vector<UInt32, 1> {
	public:
		static constexpr size_t vec_size = 1;
		using scalar_type = UInt32;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const scalar_type& x) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type* elements() const noexcept;
		inline scalar_type* elements() noexcept;
		inline const scalar_type& operator[](const int& i) const noexcept;
		inline scalar_type& operator[](const int& i) noexcept;
		inline const scalar_type& x() const;
		inline scalar_type& x();

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::u32vec1& v) noexcept;
		inline operator glm::u32vec1() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		inline operator DirectX::XMVECTOR() noexcept;
		inline operator scalar_type() noexcept;
#endif
	};

	template <>
	class LITEFX_MATH_API Vector<Float, 2> {
	public:
		static constexpr size_t vec_size = 2;
		using scalar_type = Float;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const scalar_type& v) noexcept;
		inline Vector(const scalar_type& x, const scalar_type& y) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type* elements() const noexcept;
		inline scalar_type* elements() noexcept;
		inline const scalar_type& operator[](const int& i) const noexcept;
		inline scalar_type& operator[](const int& i) noexcept;
		inline const scalar_type& x() const;
		inline scalar_type& x();
		inline const scalar_type& y() const;
		inline scalar_type& y();

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::f32vec2& v) noexcept;
		inline operator glm::f32vec2() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		inline Vector(const DirectX::XMFLOAT2& v) noexcept;
		inline operator DirectX::XMVECTOR() noexcept;
		inline operator DirectX::XMFLOAT2() noexcept;
#endif
	};

	template <>
	class LITEFX_MATH_API Vector<UInt32, 2> {
	public:
		static constexpr size_t vec_size = 2;
		using scalar_type = UInt32;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const scalar_type& v) noexcept;
		inline Vector(const scalar_type& x, const scalar_type& y) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type* elements() const noexcept;
		inline scalar_type* elements() noexcept;
		inline const scalar_type& operator[](const int& i) const noexcept;
		inline scalar_type& operator[](const int& i) noexcept;
		inline const scalar_type& x() const;
		inline scalar_type& x();
		inline const scalar_type& y() const;
		inline scalar_type& y();

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::u32vec2& v) noexcept;
		inline operator glm::u32vec2() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		inline Vector(const DirectX::XMUINT2& v) noexcept;
		inline operator DirectX::XMVECTOR() noexcept;
		inline operator DirectX::XMUINT2() noexcept;
#endif
	};

	template <>
	class LITEFX_MATH_API Vector<Int32, 2> {
	public:
		static constexpr size_t vec_size = 2;
		using scalar_type = Int32;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const scalar_type& v) noexcept;
		inline Vector(const scalar_type& x, const scalar_type& y) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type* elements() const noexcept;
		inline scalar_type* elements() noexcept;
		inline const scalar_type& operator[](const int& i) const noexcept;
		inline scalar_type& operator[](const int& i) noexcept;
		inline const scalar_type& x() const;
		inline scalar_type& x();
		inline const scalar_type& y() const;
		inline scalar_type& y();

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::i32vec2& v) noexcept;
		inline operator glm::i32vec2() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		inline Vector(const DirectX::XMINT2& v) noexcept;
		inline operator DirectX::XMVECTOR() noexcept;
		inline operator DirectX::XMINT2() noexcept;
#endif
	};

	template <>
	class LITEFX_MATH_API Vector<Float, 3> {
	public:
		static constexpr size_t vec_size = 3;
		using scalar_type = Float;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const scalar_type& v) noexcept;
		inline Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type* elements() const noexcept;
		inline scalar_type* elements() noexcept;
		inline const scalar_type& operator[](const int& i) const noexcept;
		inline scalar_type& operator[](const int& i) noexcept;
		inline const scalar_type& x() const;
		inline scalar_type& x();
		inline const scalar_type& y() const;
		inline scalar_type& y();
		inline const scalar_type& z() const;
		inline scalar_type& z();

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::f32vec3& v) noexcept;
		inline operator glm::f32vec3() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		inline Vector(const DirectX::XMFLOAT3& v) noexcept;
		inline operator DirectX::XMVECTOR() noexcept;
		inline operator DirectX::XMFLOAT3() noexcept;
#endif
	};

	template <>
	class LITEFX_MATH_API Vector<UInt32, 3> {
	public:
		static constexpr size_t vec_size = 3;
		using scalar_type = UInt32;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const scalar_type& v) noexcept;
		inline Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type* elements() const noexcept;
		inline scalar_type* elements() noexcept;
		inline const scalar_type& operator[](const int& i) const noexcept;
		inline scalar_type& operator[](const int& i) noexcept;
		inline const scalar_type& x() const;
		inline scalar_type& x();
		inline const scalar_type& y() const;
		inline scalar_type& y();
		inline const scalar_type& z() const;
		inline scalar_type& z();

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::u32vec3& v) noexcept;
		inline operator glm::u32vec3() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		inline Vector(const DirectX::XMUINT3& v) noexcept;
		inline operator DirectX::XMVECTOR() noexcept;
		inline operator DirectX::XMUINT3() noexcept;
#endif
	};

	template <>
	class LITEFX_MATH_API Vector<Int32, 3> {
	public:
		static constexpr size_t vec_size = 3;
		using scalar_type = Int32;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const scalar_type& v) noexcept;
		inline Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type* elements() const noexcept;
		inline scalar_type* elements() noexcept;
		inline const scalar_type& operator[](const int& i) const noexcept;
		inline scalar_type& operator[](const int& i) noexcept;
		inline const scalar_type& x() const;
		inline scalar_type& x();
		inline const scalar_type& y() const;
		inline scalar_type& y();
		inline const scalar_type& z() const;
		inline scalar_type& z();

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::i32vec3& v) noexcept;
		inline operator glm::i32vec3() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		inline Vector(const DirectX::XMINT3& v) noexcept;
		inline operator DirectX::XMVECTOR() noexcept;
		inline operator DirectX::XMINT3() noexcept;
#endif
	};

	template <>
	class LITEFX_MATH_API Vector<Float, 4> {
	public:
		static constexpr size_t vec_size = 4;
		using scalar_type = Float;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const scalar_type& v) noexcept;
		inline Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z, const scalar_type& w) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type* elements() const noexcept;
		inline scalar_type* elements() noexcept;
		inline const scalar_type& operator[](const int& i) const noexcept;
		inline scalar_type& operator[](const int& i) noexcept;
		inline const scalar_type& x() const;
		inline scalar_type& x();
		inline const scalar_type& y() const;
		inline scalar_type& y();
		inline const scalar_type& z() const;
		inline scalar_type& z();
		inline const scalar_type& w() const;
		inline scalar_type& w();

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::f32vec4& v) noexcept;
		inline operator glm::f32vec4() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		inline Vector(const DirectX::XMFLOAT4& v) noexcept;
		inline operator DirectX::XMVECTOR() noexcept;
		inline operator DirectX::XMFLOAT4() noexcept;
#endif
	};

	template <>
	class LITEFX_MATH_API Vector<UInt32, 4> {
	public:
		static constexpr size_t vec_size = 4;
		using scalar_type = UInt32;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const scalar_type& v) noexcept;
		inline Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z, const scalar_type& w) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type* elements() const noexcept;
		inline scalar_type* elements() noexcept;
		inline const scalar_type& operator[](const int& i) const noexcept;
		inline scalar_type& operator[](const int& i) noexcept;
		inline const scalar_type& x() const;
		inline scalar_type& x();
		inline const scalar_type& y() const;
		inline scalar_type& y();
		inline const scalar_type& z() const;
		inline scalar_type& z();
		inline const scalar_type& w() const;
		inline scalar_type& w();

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::u32vec4& v) noexcept;
		inline operator glm::u32vec4() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		inline Vector(const DirectX::XMUINT4& v) noexcept;
		inline operator DirectX::XMVECTOR() noexcept;
		inline operator DirectX::XMUINT4() noexcept;
#endif
	};

	template <>
	class LITEFX_MATH_API Vector<Int32, 4> {
	public:
		static constexpr size_t vec_size = 4;
		using scalar_type = Int32;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const scalar_type& v) noexcept;
		inline Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z, const scalar_type& w) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type* elements() const noexcept;
		inline scalar_type* elements() noexcept;
		inline const scalar_type& operator[](const int& i) const noexcept;
		inline scalar_type& operator[](const int& i) noexcept;
		inline const scalar_type& x() const;
		inline scalar_type& x();
		inline const scalar_type& y() const;
		inline scalar_type& y();
		inline const scalar_type& z() const;
		inline scalar_type& z();
		inline const scalar_type& w() const;
		inline scalar_type& w();

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::i32vec4& v) noexcept;
		inline operator glm::i32vec4() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		inline Vector(const DirectX::XMINT4& v) noexcept;
		inline operator DirectX::XMVECTOR() noexcept;
		inline operator DirectX::XMINT4() noexcept;
#endif
	};

	template<typename T> using TVector1 = Vector<T, 1>;
	template<typename T> using TVector2 = Vector<T, 2>;
	template<typename T> using TVector3 = Vector<T, 3>;
	template<typename T> using TVector4 = Vector<T, 4>;

	typedef TVector1<UInt32> Vector1u;
	typedef TVector1<Float> Vector1f;
	typedef TVector2<Int32> Vector2i;
	typedef TVector2<UInt32> Vector2u;
	typedef TVector2<Float> Vector2f;
	typedef TVector3<Int32> Vector3i;
	typedef TVector3<UInt32> Vector3u;
	typedef TVector3<Float> Vector3f;
	typedef TVector4<Int32> Vector4i;
	typedef TVector4<UInt32> Vector4u;
	typedef TVector4<Float> Vector4f;
}