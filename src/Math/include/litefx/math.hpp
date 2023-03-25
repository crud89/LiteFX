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

#if defined(BUILD_WITH_GLM)
#include <glm/glm.hpp>
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
#include <DirectXMath.h>
#endif

#include <litefx/vector.hpp>
#include <litefx/matrix.hpp>

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

	inline constexpr Byte operator "" _b(unsigned long long int arg) noexcept {
		return static_cast<Byte>(arg);
	}

	inline constexpr Int16 operator "" _i16(unsigned long long int arg) noexcept {
		return static_cast<Int16>(arg);
	}

	inline constexpr UInt16 operator "" _ui16(unsigned long long int arg) noexcept {
		return static_cast<UInt16>(arg);
	}

	inline constexpr Int32 operator "" _i32(unsigned long long int arg) noexcept {
		return static_cast<Int32>(arg);
	}

	inline constexpr UInt32 operator "" _ui32(unsigned long long int arg) noexcept {
		return static_cast<UInt32>(arg);
	}

	inline constexpr Int64 operator "" _i64(unsigned long long int arg) noexcept {
		return static_cast<Int64>(arg);
	}

	inline constexpr UInt64 operator "" _ui64(unsigned long long int arg) noexcept {
		return static_cast<UInt64>(arg);
	}

	inline constexpr Float operator "" _f32(long double arg) noexcept {
		return static_cast<Float>(arg);
	}

	inline constexpr Double operator "" _f64(long double arg) noexcept {
		return static_cast<Double>(arg);
	}

#pragma region Vector
	class LITEFX_MATH_API Vector1f : public Vector<Float, 1> {
	public:
		Vector1f() noexcept;
		Vector1f(const Float& v) noexcept;
		Vector1f(const Vector1f&) noexcept;
		Vector1f(const Vector<Float, 1>&) noexcept;
		Vector1f(Vector1f&&) noexcept;
		Vector1f(Vector<Float, 1>&&) noexcept;
		//virtual ~Vector1f() noexcept = default;

	public:
		inline Vector1f& operator=(const Vector<Float, 1>& _other) noexcept;
		inline Vector1f& operator=(Vector<Float, 1>&& _other) noexcept;
		inline Vector1f& operator=(const Array<Float>& _other) noexcept;
		inline Vector1f& operator=(const Vector1f& _other) noexcept;
		inline Vector1f& operator=(Vector1f&& _other) noexcept;
		inline const Float& operator[](const unsigned int& i) const noexcept;
		inline Float& operator[](const unsigned int& i) noexcept;
		inline operator Array<Float>() noexcept;

#if defined(BUILD_WITH_GLM)
	public:
		Vector1f(const glm::f32vec1& v) noexcept;
		Vector1f(glm::f32vec1&& v) noexcept;
		inline operator glm::f32vec1() const noexcept;
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
	public:
		Vector1f(const DirectX::XMVECTOR& v) noexcept;
		Vector1f(DirectX::XMVECTOR&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector1u : public Vector<UInt32, 1> {
	public:
		Vector1u() noexcept;
		Vector1u(const UInt32& v) noexcept;
		Vector1u(const Vector1u&) noexcept;
		Vector1u(const Vector<UInt32, 1>&) noexcept;
		Vector1u(Vector1u&&) noexcept;
		Vector1u(Vector<UInt32, 1>&&) noexcept;
		//virtual ~Vector1u() noexcept = default;
		inline operator UInt32() noexcept;

	public:
		inline Vector1u& operator=(const Vector<UInt32, 1>& _other) noexcept;
		inline Vector1u& operator=(Vector<UInt32, 1>&& _other) noexcept;
		inline Vector1u& operator=(const Array<UInt32>& _other) noexcept;
		inline Vector1u& operator=(const Vector1u& _other) noexcept;
		inline Vector1u& operator=(Vector1u&& _other) noexcept;
		inline const UInt32& operator[](const unsigned int& i) const noexcept;
		inline UInt32& operator[](const unsigned int& i) noexcept;
		inline operator Array<UInt32>() noexcept;

#if defined(BUILD_WITH_GLM)
	public:
		Vector1u(const glm::u32vec1& v) noexcept;
		Vector1u(glm::u32vec1&& v) noexcept;
		inline operator glm::u32vec1() const noexcept;
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
	public:
		Vector1u(const DirectX::XMVECTOR& v) noexcept;
		Vector1u(DirectX::XMVECTOR&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector2f : public Vector<Float, 2> {
	public:
		Vector2f() noexcept;
		Vector2f(const Float& v) noexcept;
		Vector2f(const Float& x, const Float& y) noexcept;
		Vector2f(const Vector2f&) noexcept;
		Vector2f(const Vector<Float, 2>&) noexcept;
		Vector2f(Vector2f&&) noexcept;
		Vector2f(Vector<Float, 2>&&) noexcept;
		//virtual ~Vector2f() noexcept = default;

	public:
		inline Vector2f& operator=(const Vector<Float, 2>& _other) noexcept;
		inline Vector2f& operator=(Vector<Float, 2>&& _other) noexcept;
		inline Vector2f& operator=(const Array<Float>& _other) noexcept;
		inline Vector2f& operator=(const Vector2f& _other) noexcept;
		inline Vector2f& operator=(Vector2f&& _other) noexcept;
		inline const Float& operator[](const unsigned int& i) const noexcept;
		inline Float& operator[](const unsigned int& i) noexcept;
		inline operator Array<Float>() noexcept;

#if defined(BUILD_WITH_GLM)
	public:
		Vector2f(const glm::f32vec2& v) noexcept;
		Vector2f(glm::f32vec2&& v) noexcept;
		inline operator glm::f32vec2() const noexcept;
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
	public:
		Vector2f(const DirectX::XMVECTOR& v) noexcept;
		Vector2f(DirectX::XMVECTOR&& v) noexcept;
		Vector2f(const DirectX::XMFLOAT2& v) noexcept;
		Vector2f(DirectX::XMFLOAT2&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
		inline operator DirectX::XMFLOAT2() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector2u : public Vector<UInt32, 2> {
	public:
		Vector2u() noexcept;
		Vector2u(const UInt32& v) noexcept;
		Vector2u(const UInt32& x, const UInt32& y) noexcept;
		Vector2u(const Vector2u&) noexcept;
		Vector2u(const Vector<UInt32, 2>&) noexcept;
		Vector2u(Vector2u&&) noexcept;
		Vector2u(Vector<UInt32, 2>&&) noexcept;
		//virtual ~Vector2u() noexcept = default;

	public:
		inline Vector2u& operator=(const Vector<UInt32, 2>& _other) noexcept;
		inline Vector2u& operator=(Vector<UInt32, 2>&& _other) noexcept;
		inline Vector2u& operator=(const Array<UInt32>& _other) noexcept;
		inline Vector2u& operator=(const Vector2u& _other) noexcept;
		inline Vector2u& operator=(Vector2u&& _other) noexcept;
		inline const UInt32& operator[](const unsigned int& i) const noexcept;
		inline UInt32& operator[](const unsigned int& i) noexcept;
		inline operator Array<UInt32>() noexcept;

#if defined(BUILD_WITH_GLM)
	public:
		Vector2u(const glm::u32vec2& v) noexcept;
		Vector2u(glm::u32vec2&& v) noexcept;
		inline operator glm::u32vec2() const noexcept;
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
	public:
		Vector2u(const DirectX::XMVECTOR& v) noexcept;
		Vector2u(DirectX::XMVECTOR&& v) noexcept;
		Vector2u(const DirectX::XMUINT2& v) noexcept;
		Vector2u(DirectX::XMUINT2&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
		inline operator DirectX::XMUINT2() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector2i : public Vector<Int32, 2> {
	public:
		Vector2i() noexcept;
		Vector2i(const Int32& v) noexcept;
		Vector2i(const Int32& x, const Int32& y) noexcept;
		Vector2i(const Vector2i&) noexcept;
		Vector2i(const Vector<Int32, 2>&) noexcept;
		Vector2i(Vector2i&&) noexcept;
		Vector2i(Vector<Int32, 2>&&) noexcept;
		//virtual ~Vector2i() noexcept = default;

	public:
		inline Vector2i& operator=(const Vector<Int32, 2>& _other) noexcept;
		inline Vector2i& operator=(Vector<Int32, 2>&& _other) noexcept;
		inline Vector2i& operator=(const Array<Int32>& _other) noexcept;
		inline Vector2i& operator=(const Vector2i& _other) noexcept;
		inline Vector2i& operator=(Vector2i&& _other) noexcept;
		inline const Int32& operator[](const unsigned int& i) const noexcept;
		inline Int32& operator[](const unsigned int& i) noexcept;
		inline operator Array<Int32>() noexcept;

#if defined(BUILD_WITH_GLM)
	public:
		Vector2i(const glm::i32vec2& v) noexcept;
		Vector2i(glm::i32vec2&& v) noexcept;
		inline operator glm::i32vec2() const noexcept;
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
	public:
		Vector2i(const DirectX::XMVECTOR& v) noexcept;
		Vector2i(DirectX::XMVECTOR&& v) noexcept;
		Vector2i(const DirectX::XMINT2& v) noexcept;
		Vector2i(DirectX::XMINT2&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
		inline operator DirectX::XMINT2() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector3f : public Vector<Float, 3> {
	public:
		Vector3f() noexcept;
		Vector3f(const Float& v) noexcept;
		Vector3f(const Float& x, const Float& y, const Float& z) noexcept;
		Vector3f(const Vector3f&) noexcept;
		Vector3f(const Vector<Float, 3>&) noexcept;
		Vector3f(Vector3f&&) noexcept;
		Vector3f(Vector<Float, 3>&&) noexcept;
		//virtual ~Vector3f() noexcept = default;

	public:
		inline Vector3f& operator=(const Vector<Float, 3>& _other) noexcept;
		inline Vector3f& operator=(Vector<Float, 3>&& _other) noexcept;
		inline Vector3f& operator=(const Array<Float>& _other) noexcept;
		inline Vector3f& operator=(const Vector3f& _other) noexcept;
		inline Vector3f& operator=(Vector3f&& _other) noexcept;
		inline const Float& operator[](const unsigned int& i) const noexcept;
		inline Float& operator[](const unsigned int& i) noexcept;
		inline operator Array<Float>() noexcept;

#if defined(BUILD_WITH_GLM)
	public:
		Vector3f(const glm::f32vec3& v) noexcept;
		Vector3f(glm::f32vec3&& v) noexcept;
		inline operator glm::f32vec3() const noexcept;
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
	public:
		Vector3f(const DirectX::XMVECTOR& v) noexcept;
		Vector3f(DirectX::XMVECTOR&& v) noexcept;
		Vector3f(const DirectX::XMFLOAT3& v) noexcept;
		Vector3f(DirectX::XMFLOAT3&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
		inline operator DirectX::XMFLOAT3() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector3u : public Vector<UInt32, 3> {
	public:
		Vector3u() noexcept;
		Vector3u(const UInt32& v) noexcept;
		Vector3u(const UInt32& x, const UInt32& y, const UInt32& z) noexcept;
		Vector3u(const Vector3u&) noexcept;
		Vector3u(const Vector<UInt32, 3>&) noexcept;
		Vector3u(Vector3u&&) noexcept;
		Vector3u(Vector<UInt32, 3>&&) noexcept;
		//virtual ~Vector3u() noexcept = default;

	public:
		inline Vector3u& operator=(const Vector<UInt32, 3>& _other) noexcept;
		inline Vector3u& operator=(Vector<UInt32, 3>&& _other) noexcept;
		inline Vector3u& operator=(const Array<UInt32>& _other) noexcept;
		inline Vector3u& operator=(const Vector3u& _other) noexcept;
		inline Vector3u& operator=(Vector3u&& _other) noexcept;
		inline const UInt32& operator[](const unsigned int& i) const noexcept;
		inline UInt32& operator[](const unsigned int& i) noexcept;
		inline operator Array<UInt32>() noexcept;

#if defined(BUILD_WITH_GLM)
	public:
		Vector3u(const glm::u32vec3& v) noexcept;
		Vector3u(glm::u32vec3&& v) noexcept;
		inline operator glm::u32vec3() const noexcept;
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
	public:
		Vector3u(const DirectX::XMVECTOR& v) noexcept;
		Vector3u(DirectX::XMVECTOR&& v) noexcept;
		Vector3u(const DirectX::XMUINT3& v) noexcept;
		Vector3u(DirectX::XMUINT3&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
		inline operator DirectX::XMUINT3() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector3i : public Vector<Int32, 3> {
	public:
		Vector3i() noexcept;
		Vector3i(const Int32& v) noexcept;
		Vector3i(const Int32& x, const Int32& y, const Int32& z) noexcept;
		Vector3i(const Vector3i&) noexcept;
		Vector3i(const Vector<Int32, 3>&) noexcept;
		Vector3i(Vector3i&&) noexcept;
		Vector3i(Vector<Int32, 3>&&) noexcept;
		//virtual ~Vector3i() noexcept = default;

	public:
		inline Vector3i& operator=(const Vector<Int32, 3>& _other) noexcept;
		inline Vector3i& operator=(Vector<Int32, 3>&& _other) noexcept;
		inline Vector3i& operator=(const Array<Int32>& _other) noexcept;
		inline Vector3i& operator=(const Vector3i& _other) noexcept;
		inline Vector3i& operator=(Vector3i&& _other) noexcept;
		inline const Int32& operator[](const unsigned int& i) const noexcept;
		inline Int32& operator[](const unsigned int& i) noexcept;
		inline operator Array<Int32>() noexcept;

#if defined(BUILD_WITH_GLM)
	public:
		Vector3i(const glm::i32vec3& v) noexcept;
		Vector3i(glm::i32vec3&& v) noexcept;
		inline operator glm::i32vec3() const noexcept;
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
	public:
		Vector3i(const DirectX::XMVECTOR& v) noexcept;
		Vector3i(DirectX::XMVECTOR&& v) noexcept;
		Vector3i(const DirectX::XMINT3& v) noexcept;
		Vector3i(DirectX::XMINT3&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
		inline operator DirectX::XMINT3() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector4f : public Vector<Float, 4> {
	public:
		Vector4f() noexcept;
		Vector4f(const Float& v) noexcept;
		Vector4f(const Float& x, const Float& y, const Float& z, const Float& w) noexcept;
		Vector4f(const Vector4f&) noexcept;
		Vector4f(const Vector<Float, 4>&) noexcept;
		Vector4f(Vector4f&&) noexcept;
		Vector4f(Vector<Float, 4>&&) noexcept;
		//virtual ~Vector4f() noexcept = default;

	public:
		inline Vector4f& operator=(const Vector<Float, 4>& _other) noexcept;
		inline Vector4f& operator=(Vector<Float, 4>&& _other) noexcept;
		inline Vector4f& operator=(const Array<Float>& _other) noexcept;
		inline Vector4f& operator=(const Vector4f& _other) noexcept;
		inline Vector4f& operator=(Vector4f&& _other) noexcept;
		inline const Float& operator[](const unsigned int& i) const noexcept;
		inline Float& operator[](const unsigned int& i) noexcept;
		inline operator Array<Float>() noexcept;

#if defined(BUILD_WITH_GLM)
	public:
		Vector4f(const glm::f32vec4& v) noexcept;
		Vector4f(glm::f32vec4&& v) noexcept;
		inline operator glm::f32vec4() const noexcept;
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
	public:
		Vector4f(const DirectX::XMVECTOR& v) noexcept;
		Vector4f(DirectX::XMVECTOR&& v) noexcept;
		Vector4f(const DirectX::XMFLOAT4& v) noexcept;
		Vector4f(DirectX::XMFLOAT4&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
		inline operator DirectX::XMFLOAT4() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector4u : public Vector<UInt32, 4> {
	public:
		Vector4u() noexcept;
		Vector4u(const UInt32& v) noexcept;
		Vector4u(const UInt32& x, const UInt32& y, const UInt32& z, const UInt32& w) noexcept;
		Vector4u(const Vector4u&) noexcept;
		Vector4u(const Vector<UInt32, 4>&) noexcept;
		Vector4u(Vector4u&&) noexcept;
		Vector4u(Vector<UInt32, 4>&&) noexcept;
		//virtual ~Vector4u() noexcept = default;

	public:
		inline Vector4u& operator=(const Vector<UInt32, 4>& _other) noexcept;
		inline Vector4u& operator=(Vector<UInt32, 4>&& _other) noexcept;
		inline Vector4u& operator=(const Array<UInt32>& _other) noexcept;
		inline Vector4u& operator=(const Vector4u& _other) noexcept;
		inline Vector4u& operator=(Vector4u&& _other) noexcept;
		inline const UInt32& operator[](const unsigned int& i) const noexcept;
		inline UInt32& operator[](const unsigned int& i) noexcept;
		inline operator Array<UInt32>() noexcept;

#if defined(BUILD_WITH_GLM)
	public:
		Vector4u(const glm::u32vec4& v) noexcept;
		Vector4u(glm::u32vec4&& v) noexcept;
		inline operator glm::u32vec4() const noexcept;
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
	public:
		Vector4u(const DirectX::XMVECTOR& v) noexcept;
		Vector4u(DirectX::XMVECTOR&& v) noexcept;
		Vector4u(const DirectX::XMUINT4& v) noexcept;
		Vector4u(DirectX::XMUINT4&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
		inline operator DirectX::XMUINT4() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector4i : public Vector<Int32, 4> {
	public:
		Vector4i() noexcept;
		Vector4i(const Int32& v) noexcept;
		Vector4i(const Int32& x, const Int32& y, const Int32& z, const Int32& w) noexcept;
		Vector4i(const Vector4i&) noexcept;
		Vector4i(const Vector<Int32, 4>&) noexcept;
		Vector4i(Vector4i&&) noexcept;
		Vector4i(Vector<Int32, 4>&&) noexcept;
		//virtual ~Vector4i() noexcept = default;

	public:
		inline Vector4i& operator=(const Vector<Int32, 4>& _other) noexcept;
		inline Vector4i& operator=(Vector<Int32, 4>&& _other) noexcept;
		inline Vector4i& operator=(const Array<Int32>& _other) noexcept;
		inline Vector4i& operator=(const Vector4i& _other) noexcept;
		inline Vector4i& operator=(Vector4i&& _other) noexcept;
		inline const Int32& operator[](const unsigned int& i) const noexcept;
		inline Int32& operator[](const unsigned int& i) noexcept;
		inline operator Array<Int32>() noexcept;

#if defined(BUILD_WITH_GLM)
	public:
		Vector4i(const glm::i32vec4& v) noexcept;
		Vector4i(glm::i32vec4&& v) noexcept;
		inline operator glm::i32vec4() const noexcept;
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
	public:
		Vector4i(const DirectX::XMVECTOR& v) noexcept;
		Vector4i(DirectX::XMVECTOR&& v) noexcept;
		Vector4i(const DirectX::XMINT4& v) noexcept;
		Vector4i(DirectX::XMINT4&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
		inline operator DirectX::XMINT4() const noexcept;
#endif
	};

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
#pragma endregion

#pragma region Size
	class LITEFX_MATH_API Size4d : public Vector<size_t, 4> {
	public:
		Size4d() noexcept;
		Size4d(const size_t& v) noexcept;
		Size4d(const size_t& w, const size_t& h, const size_t& d, const size_t& a) noexcept;
		Size4d(const Size4d&) noexcept;
		Size4d(Size4d&&) noexcept;
		//virtual ~Size4d() noexcept = default;

	public:
		inline Size4d& operator=(const Size4d& _other) noexcept;
		inline Size4d& operator=(Size4d&& _other) noexcept;
		inline Size4d operator/(const size_t& s) noexcept;
		inline Size4d& operator/=(const size_t& s) noexcept;
		inline Size4d operator*(const size_t& s) noexcept;
		inline Size4d& operator*=(const size_t& s) noexcept;
		inline Size4d operator+(const Size4d& s) noexcept;
		inline Size4d& operator+=(const Size4d& s) noexcept;
		inline Size4d operator-(const Size4d& s) noexcept;
		inline Size4d& operator-=(const Size4d& s) noexcept;

	public:
		inline const size_t& width() const noexcept;
		inline size_t& width() noexcept;
		inline const size_t& height() const noexcept;
		inline size_t& height() noexcept;
		inline const size_t& depth() const noexcept;
		inline size_t& depth() noexcept;
		inline const size_t& alpha() const noexcept;
		inline size_t& alpha() noexcept;
	};

	class LITEFX_MATH_API Size3d : public Vector<size_t, 3> {
	public:
		Size3d() noexcept;
		Size3d(const size_t& v) noexcept;
		Size3d(const size_t& w, const size_t& h, const size_t& d) noexcept;
		Size3d(const Size3d&) noexcept;
		Size3d(Size3d&&) noexcept;
		//virtual ~Size3d() noexcept = default;

	public:
		inline Size3d& operator=(const Size3d& _other) noexcept;
		inline Size3d& operator=(Size3d&& _other) noexcept;
		inline operator Size4d() const noexcept;
		inline Size3d operator/(const size_t& s) noexcept;
		inline Size3d& operator/=(const size_t& s) noexcept;
		inline Size3d operator*(const size_t& s) noexcept;
		inline Size3d& operator*=(const size_t& s) noexcept;
		inline Size3d operator+(const Size3d& s) noexcept;
		inline Size3d& operator+=(const Size3d& s) noexcept;
		inline Size3d operator-(const Size3d& s) noexcept;
		inline Size3d& operator-=(const Size3d& s) noexcept;

	public:
		inline const size_t& width() const noexcept;
		inline size_t& width() noexcept;
		inline const size_t& height() const noexcept;
		inline size_t& height() noexcept;
		inline const size_t& depth() const noexcept;
		inline size_t& depth() noexcept;
	};

	class LITEFX_MATH_API Size2d : public Vector<size_t, 2> {
	public:
		Size2d() noexcept;
		Size2d(const size_t& v) noexcept;
		Size2d(const size_t& w, const size_t& h) noexcept;
		Size2d(const Size2d&) noexcept;
		Size2d(Size2d&&) noexcept;
		//virtual ~Size2d() noexcept = default;

	public:
		inline Size2d& operator=(const Size2d& _other) noexcept;
		inline Size2d& operator=(Size2d&& _other) noexcept;
		inline operator Size3d() const noexcept;
		inline operator Size4d() const noexcept;
		inline Size2d operator/(const size_t& s) noexcept;
		inline Size2d& operator/=(const size_t& s) noexcept;
		inline Size2d operator*(const size_t& s) noexcept;
		inline Size2d& operator*=(const size_t& s) noexcept;
		inline Size2d operator+(const Size2d& s) noexcept;
		inline Size2d& operator+=(const Size2d& s) noexcept;
		inline Size2d operator-(const Size2d& s) noexcept;
		inline Size2d& operator-=(const Size2d& s) noexcept;

	public:
		inline const size_t& width() const noexcept;
		inline size_t& width() noexcept;
		inline const size_t& height() const noexcept;
		inline size_t& height() noexcept;
	};
#pragma endregion

#pragma region Rectangle
	class LITEFX_MATH_API Rect : public Vector<size_t, 4> {
	public:
		Rect() noexcept;
		Rect(const Vector<size_t, 2>& pos, const size_t& w, const size_t& h) noexcept;
		Rect(const size_t& x, const size_t& y, const size_t& w, const size_t& h) noexcept;
		Rect(const Rect&) noexcept;
		Rect(Rect&&) noexcept;
		//virtual ~Rect() noexcept = default;

	public:
		inline Rect& operator=(const Rect& _other) noexcept;
		inline Rect& operator=(Rect&& _other) noexcept;

	public:
		inline Vector<size_t, 2> position() const noexcept;
		inline Size2d extent() const noexcept;
		inline const size_t& width() const noexcept;
		inline size_t& width() noexcept;
		inline const size_t& height() const noexcept;
		inline size_t& height() noexcept;
	};

	class LITEFX_MATH_API RectI : public Vector<Int32, 4> {
	public:
		RectI() noexcept;
		RectI(const Vector<Int32, 2>& pos, const Int32& w, const Int32& h) noexcept;
		RectI(const Int32& x, const Int32& y, const Int32& w, const Int32& h) noexcept;
		RectI(const RectI&) noexcept;
		RectI(RectI&&) noexcept;
		//virtual ~RectI() noexcept = default;

	public:
		inline RectI& operator=(const RectI& _other) noexcept;
		inline RectI& operator=(RectI&& _other) noexcept;

	public:
		inline Vector<Int32, 2> position() const noexcept;
		inline Size2d extent() const noexcept;
		inline const Int32& width() const noexcept;
		inline Int32& width() noexcept;
		inline const Int32& height() const noexcept;
		inline Int32& height() noexcept;
	};

	class LITEFX_MATH_API RectF : public Vector<Float, 4> {
	public:
		RectF() noexcept;
		RectF(const Vector<Float, 2>& pos, const Float& w, const Float& h) noexcept;
		RectF(const Float& x, const Float& y, const Float& w, const Float& h) noexcept;
		RectF(const RectF&) noexcept;
		RectF(RectF&&) noexcept;
		//virtual ~RectF() noexcept = default;

	public:
		inline RectF& operator=(const RectF& _other) noexcept;
		inline RectF& operator=(RectF&& _other) noexcept;

	public:
		inline Vector<Float, 2> position() const noexcept;
		inline Size2d extent() const noexcept;
		inline const Float& width() const noexcept;
		inline Float& width() noexcept;
		inline const Float& height() const noexcept;
		inline Float& height() noexcept;
	};
#pragma endregion
}