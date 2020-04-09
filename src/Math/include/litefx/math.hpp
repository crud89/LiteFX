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

	using Byte = uint8_t;
	using Int16 = int16_t;
	using UInt16 = uint16_t;
	using Int32 = int32_t;
	using UInt32 = uint32_t;
	using Int64 = int64_t;
	using UInt64 = uint64_t;
	using Float = float_t;
	using Double = double_t;

	template <typename T, int DIM>
	class LITEFX_MATH_API Vector {
	public:
		static constexpr size_t vec_size = DIM;
		using scalar_type = T;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		inline Vector() noexcept = default;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		inline const scalar_type& elements() const noexcept;
	};

	template <typename T>
	class LITEFX_MATH_API Vector<T, 1> {
	public:
		static constexpr size_t vec_size = 1;
		using scalar_type = T;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		//inline Vector(const scalar_type& x) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		//inline const scalar_type& x() const;
		//inline scalar_type& x();
	};

	template <typename T>
	class LITEFX_MATH_API Vector<T, 2> {
	public:
		static constexpr size_t vec_size = 2;
		using scalar_type = T;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		//inline Vector(const scalar_type& x) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		//inline const scalar_type& x() const;
		//inline scalar_type& x();
		//inline const scalar_type& y() const;
		//inline scalar_type& y();
	};

	template <typename T>
	class LITEFX_MATH_API Vector<T, 3> {
	public:
		static constexpr size_t vec_size = 3;
		using scalar_type = T;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		//inline Vector(const scalar_type& x) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		//inline const scalar_type& x() const;
		//inline scalar_type& x();
		//inline const scalar_type& y() const;
		//inline scalar_type& y();
		//inline const scalar_type& z() const;
		//inline scalar_type& z();
	};

	template <typename T>
	class LITEFX_MATH_API Vector<T, 4> {
	public:
		static constexpr size_t vec_size = 4;
		using scalar_type = T;
		using vec_type = Vector<scalar_type, vec_size>;

	private:
		scalar_type m_elements[vec_size];

	public:
		//inline Vector(const scalar_type& x) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

	public:
		//inline const scalar_type& x() const;
		//inline scalar_type& x();
		//inline const scalar_type& y() const;
		//inline scalar_type& y();
		//inline const scalar_type& z() const;
		//inline scalar_type& z();
		//inline const scalar_type& w() const;
		//inline scalar_type& w();
	};

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

	// Define exported vector types.
	namespace Vectors {
		using ByteVector1 = TVector1<Byte>;
		using ByteVector2 = TVector2<Byte>;
		using ByteVector3 = TVector3<Byte>;
		using ByteVector4 = TVector4<Byte>;
		using Int16Vector1 = TVector1<Int16>;
		using Int16Vector2 = TVector2<Int16>;
		using Int16Vector3 = TVector3<Int16>;
		using Int16Vector4 = TVector4<Int16>;
		using UInt16Vector1 = TVector1<UInt16>;
		using UInt16Vector2 = TVector2<UInt16>;
		using UInt16Vector3 = TVector3<UInt16>;
		using UInt16Vector4 = TVector4<UInt16>;
		using Int32Vector1 = TVector1<Int32>;
		using Int32Vector2 = TVector2<Int32>;
		using Int32Vector3 = TVector3<Int32>;
		using Int32Vector4 = TVector4<Int32>;
		using UInt32Vector1 = TVector1<UInt32>;
		using UInt32Vector2 = TVector2<UInt32>;
		using UInt32Vector3 = TVector3<UInt32>;
		using UInt32Vector4 = TVector4<UInt32>;
		using Int64Vector1 = TVector1<Int64>;
		using Int64Vector2 = TVector2<Int64>;
		using Int64Vector3 = TVector3<Int64>;
		using Int64Vector4 = TVector4<Int64>;
		using UInt64Vector1 = TVector1<UInt64>;
		using UInt64Vector2 = TVector2<UInt64>;
		using UInt64Vector3 = TVector3<UInt64>;
		using UInt64Vector4 = TVector4<UInt64>;
		using FloatVector1 = TVector1<Float>;
		using FloatVector2 = TVector2<Float>;
		using FloatVector3 = TVector3<Float>;
		using FloatVector4 = TVector4<Float>;
		using DoubleVector1 = TVector1<Double>;
		using DoubleVector2 = TVector2<Double>;
		using DoubleVector3 = TVector3<Double>;
		using DoubleVector4 = TVector4<Double>;
	}

	// Define vector types that support conversion to higher level APIs.
	typedef Vectors::UInt32Vector1 Vector1u;
	typedef Vectors::FloatVector1 Vector1f;
	typedef Vectors::Int32Vector2 Vector2i;
	typedef Vectors::UInt32Vector2 Vector2u;
	typedef Vectors::FloatVector2 Vector2f;
	typedef Vectors::Int32Vector3 Vector3i;
	typedef Vectors::UInt32Vector3 Vector3u;
	typedef Vectors::FloatVector3 Vector3f;
	typedef Vectors::Int32Vector4 Vector4i;
	typedef Vectors::UInt32Vector4 Vector4u;
	typedef Vectors::FloatVector4 Vector4f;

	// Define other special vector types.
	typedef TVector2<size_t> Size2d;
	typedef TVector3<size_t> Size3d;
	typedef TVector4<size_t> Size4d;
}