#pragma once

#include <litefx/core_types.hpp>

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
	
	template <typename T>
	class LITEFX_MATH_API Vector<T, 1> {
	public:
		static constexpr size_t vec_size = 1;
		using vec_type = Vector<T, vec_size>;

	private:
		T m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const T& x) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::tvec1<T>& v) noexcept;
		inline operator glm::tvec1<T>() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline Vector(const DirectX::XMVECTOR& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline Vector(const DirectX::XMVECTOR& v) noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline operator DirectX::XMVECTOR () noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline operator Float () noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline operator DirectX::XMVECTOR () noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline operator UInt32 () noexcept;
#endif
	};

	template <typename T>
	class LITEFX_MATH_API Vector<T, 2> {
	public:
		static constexpr size_t vec_size = 2;
		using vec_type = Vector<T, vec_size>;

	private:
		T m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const T& x, const T& y) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::tvec2<T>& v) noexcept;
		inline operator glm::tvec2<T>() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline Vector(const DirectX::XMVECTOR& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline Vector(const DirectX::XMFLOAT2& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline Vector(const DirectX::XMVECTOR& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline Vector(const DirectX::XMUINT2& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline Vector(const DirectX::XMVECTOR& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline Vector(const DirectX::XMINT2& v) noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline operator DirectX::XMVECTOR () noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline operator DirectX::XMFLOAT2 () noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline operator DirectX::XMVECTOR () noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline operator DirectX::XMUINT2 () noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline operator DirectX::XMVECTOR () noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline operator DirectX::XMINT2 () noexcept;
#endif
	};

	template <typename T>
	class LITEFX_MATH_API Vector<T, 3> {
	public:
		static constexpr size_t vec_size = 3;
		using vec_type = Vector<T, vec_size>;

	private:
		T m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const T& x, const T& y, const T& z) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::tvec3<T>& v) noexcept;
		inline operator glm::tvec3<T>() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline Vector(const DirectX::XMVECTOR& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline Vector(const DirectX::XMFLOAT3& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline Vector(const DirectX::XMVECTOR& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline Vector(const DirectX::XMUINT3& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline Vector(const DirectX::XMVECTOR& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline Vector(const DirectX::XMINT3& v) noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline operator DirectX::XMVECTOR () noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline operator DirectX::XMFLOAT3 () noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline operator DirectX::XMVECTOR () noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline operator DirectX::XMUINT3 () noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline operator DirectX::XMVECTOR () noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline operator DirectX::XMINT3 () noexcept;
#endif
	};

	template <typename T>
	class LITEFX_MATH_API Vector<T, 4> {
	public:
		static constexpr size_t vec_size = 4;
		using vec_type = Vector<T, vec_size>;

	private:
		T m_elements[vec_size];

	public:
		Vector() noexcept = delete;
		inline Vector(const T& x, const T& y, const T& z, const T& w) noexcept;
		inline Vector(const vec_type& _other) noexcept;
		inline Vector(vec_type&& _other) noexcept;
		virtual inline ~Vector() noexcept = default;

#if defined(BUILD_ENABLE_GLM)
		inline Vector(const glm::tvec4<T>& v) noexcept;
		inline operator glm::tvec4<T>() noexcept;
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline Vector(const DirectX::XMVECTOR& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline Vector(const DirectX::XMFLOAT4& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline Vector(const DirectX::XMVECTOR& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline Vector(const DirectX::XMUINT4& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline Vector(const DirectX::XMVECTOR& v) noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline Vector(const DirectX::XMINT4& v) noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline operator DirectX::XMVECTOR () noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
		inline operator DirectX::XMFLOAT4 () noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline operator DirectX::XMVECTOR () noexcept;

		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
		inline operator DirectX::XMUINT4 () noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline operator DirectX::XMVECTOR () noexcept;
		
		template <typename U = T, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
		inline operator DirectX::XMINT4 () noexcept;
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