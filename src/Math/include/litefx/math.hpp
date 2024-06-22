#pragma once

#include <litefx/core.h>

#if !defined (LITEFX_MATH_API) && defined(LITEFX_BUILD_AS_SHARED)
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

#if defined(LITEFX_BUILD_WITH_GLM)
#include <glm/glm.hpp>
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
#include <DirectXMath.h>
#endif

#include <litefx/vector.hpp>
#include <litefx/matrix.hpp>

namespace LiteFX::Math {
	using namespace LiteFX;

	/// <summary>
	/// A type for an unsigned 8 bit integer.
	/// </summary>
	using Byte = uint8_t;

	/// <summary>
	/// A type for a signed 16 bit integer.
	/// </summary>
	using Int16 = int16_t;

	/// <summary>
	/// A type for an unsigned 16 bit integer.
	/// </summary>
	using UInt16 = uint16_t;

	/// <summary>
	/// A type for a signed 32 bit integer.
	/// </summary>
	using Int32 = int32_t;

	/// <summary>
	/// A type for an unsigned 32 bit integer.
	/// </summary>
	using UInt32 = uint32_t;

	/// <summary>
	/// A type for a signed 64 bit integer.
	/// </summary>
	using Int64 = int64_t;

	/// <summary>
	/// A type for an unsigned 64 bit integer.
	/// </summary>
	using UInt64 = uint64_t;

	/// <summary>
	/// A type for a floating point value with single precision.
	/// </summary>
	using Float = float_t;

	/// <summary>
	/// A type for a floating point value with double precision.
	/// </summary>
	using Double = double_t;

	/// <summary>
	/// A literal to define a byte.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the byte.</param>
	/// <returns>The value as byte.</returns>
	inline constexpr Byte operator "" _b(unsigned long long int arg) noexcept {
		return static_cast<Byte>(arg);
	}

	/// <summary>
	/// A literal to define a 16 bit integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 16 bit integer.</returns>
	inline constexpr Int16 operator "" _i16(unsigned long long int arg) noexcept {
		return static_cast<Int16>(arg);
	}

	/// <summary>
	/// A literal to define a 16 bit unsigned integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 16 bit unsigned integer.</returns>
	inline constexpr UInt16 operator "" _ui16(unsigned long long int arg) noexcept {
		return static_cast<UInt16>(arg);
	}

	/// <summary>
	/// A literal to define a 32 bit integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 32 bit integer.</returns>
	inline constexpr Int32 operator "" _i32(unsigned long long int arg) noexcept {
		return static_cast<Int32>(arg);
	}

	/// <summary>
	/// A literal to define a 32 bit unsigned integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 32 bit unsigned integer.</returns>
	inline constexpr UInt32 operator "" _ui32(unsigned long long int arg) noexcept {
		return static_cast<UInt32>(arg);
	}

	/// <summary>
	/// A literal to define a 64 bit integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 64 bit integer.</returns>
	inline constexpr Int64 operator "" _i64(unsigned long long int arg) noexcept {
		return static_cast<Int64>(arg);
	}

	/// <summary>
	/// A literal to define a 64 bit unsigned integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 64 bit unsigned integer.</returns>
	inline constexpr UInt64 operator "" _ui64(unsigned long long int arg) noexcept {
		return static_cast<UInt64>(arg);
	}

	/// <summary>
	/// A literal to define a floating point value with single precision.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the floating point number.</param>
	/// <returns>The value as floating point number.</returns>
	inline constexpr Float operator "" _f32(long double arg) noexcept {
		return static_cast<Float>(arg);
	}

	/// <summary>
	/// A literal to define a floating point value with double precision.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the floating point number.</param>
	/// <returns>The value as floating point number.</returns>
	inline constexpr Double operator "" _f64(long double arg) noexcept {
		return static_cast<Double>(arg);
	}

	/// <summary>
	/// Aligns a value <paramref name="size" /> to an alignment <paramref name="alignment" />.
	/// </summary>
	/// <typeparam name="T">The type of the value to align.</typeparam>
	/// <param name="size">The value to align.</param>
	/// <param name="alignment">The alignment to align the value to.</param>
	/// <returns>The aligned value.</returns>
	template <typename T = UInt32>
	static constexpr inline T align(T size, T alignment) { 
		return (size + alignment - 1) & ~(alignment - 1); 
	}

#pragma region Vector
	/// <summary>
	/// A vector that contains a single float.
	/// </summary>
	class LITEFX_MATH_API Vector1f : public Vector<Float, 1> {
	public:
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		/// <summary>
		/// Converts a vector of type `glm::f32vec1`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector1f(const glm::f32vec1& v) noexcept;

		/// <summary>
		/// Converts a vector of type `glm::f32vec1`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector1f(glm::f32vec1&& v) noexcept;

		/// <summary>
		/// Converts the vector into the type `glm::f32vec1`.
		/// </summary>
		inline operator glm::f32vec1() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		/// <summary>
		/// Converts a vector of type `DirectX::XMVECTOR`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector1f(const DirectX::XMVECTOR& v) noexcept;

		/// <summary>
		/// Converts a vector of type `DirectX::XMVECTOR`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector1f(DirectX::XMVECTOR&& v) noexcept;

		/// <summary>
		/// Converts the vector into the type `DirectX::XMVECTOR`.
		/// </summary>
		inline operator DirectX::XMVECTOR() const noexcept;
#endif
	};

	// NOTE: Vector1i unsupported, due to lack of `XMStoreSInt` overload for scalar signed integers.

	/// <summary>
	/// A vector that contains a single unsigned integer.
	/// </summary>
	class LITEFX_MATH_API Vector1u : public Vector<UInt32, 1> {
	public:
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		/// <summary>
		/// Converts a vector of type `glm::u32vec1`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector1u(const glm::u32vec1& v) noexcept;

		/// <summary>
		/// Converts a vector of type `glm::u32vec1`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector1u(glm::u32vec1&& v) noexcept;

		/// <summary>
		/// Converts the vector into the type `glm::u32vec1`.
		/// </summary>
		inline operator glm::u32vec1() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		/// <summary>
		/// Converts a vector of type `DirectX::XMVECTOR`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector1u(const DirectX::XMVECTOR& v) noexcept;

		/// <summary>
		/// Converts a vector of type `DirectX::XMVECTOR`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector1u(DirectX::XMVECTOR&& v) noexcept;

		/// <summary>
		/// Converts the vector into the type `DirectX::XMVECTOR`.
		/// </summary>
		inline operator DirectX::XMVECTOR() const noexcept;
#endif
	};

	/// <summary>
	/// A vector that contains two floats.
	/// </summary>
	class LITEFX_MATH_API Vector2f : public Vector<Float, 2> {
	public:
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		/// <summary>
		/// Converts a vector of type `glm::f32vec2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2f(const glm::f32vec2& v) noexcept;

		/// <summary>
		/// Converts a vector of type `glm::f32vec2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2f(glm::f32vec2&& v) noexcept;

		/// <summary>
		/// Converts the vector into the type `glm::f32vec2`.
		/// </summary>
		inline operator glm::f32vec2() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		/// <summary>
		/// Converts a vector of type `DirectX::XMVECTOR`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2f(const DirectX::XMVECTOR& v) noexcept;

		/// <summary>
		/// Converts a vector of type `DirectX::XMVECTOR`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2f(DirectX::XMVECTOR&& v) noexcept;

		/// <summary>
		/// Converts a vector of type `DirectX::XMFLOAT2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2f(const DirectX::XMFLOAT2& v) noexcept;

		/// <summary>
		/// Converts a vector of type `DirectX::XMFLOAT2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2f(DirectX::XMFLOAT2&& v) noexcept;

		/// <summary>
		/// Converts the vector into the type `DirectX::XMVECTOR`.
		/// </summary>
		inline operator DirectX::XMVECTOR() const noexcept;

		/// <summary>
		/// Converts the vector into the type `DirectX::XMFLOAT2`.
		/// </summary>
		inline operator DirectX::XMFLOAT2() const noexcept;
#endif
	};

	/// <summary>
	/// A vector that contains two unsigned integers.
	/// </summary>
	class LITEFX_MATH_API Vector2u : public Vector<UInt32, 2> {
	public:
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		/// <summary>
		/// Converts a vector of type `glm::u32vec2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2u(const glm::u32vec2& v) noexcept;

		/// <summary>
		/// Converts a vector of type `glm::u32vec2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2u(glm::u32vec2&& v) noexcept;

		/// <summary>
		/// Converts the vector into the type `glm::u32vec2`.
		/// </summary>
		inline operator glm::u32vec2() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		/// <summary>
		/// Converts a vector of type `DirectX::XMVECTOR`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2u(const DirectX::XMVECTOR& v) noexcept;

		/// <summary>
		/// Converts a vector of type `DirectX::XMVECTOR`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2u(DirectX::XMVECTOR&& v) noexcept;

		/// <summary>
		/// Converts a vector of type `DirectX::XMUINT2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2u(const DirectX::XMUINT2& v) noexcept;

		/// <summary>
		/// Converts a vector of type `DirectX::XMUINT2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2u(DirectX::XMUINT2&& v) noexcept;

		/// <summary>
		/// Converts the vector into the type `DirectX::XMVECTOR`.
		/// </summary>
		inline operator DirectX::XMVECTOR() const noexcept;

		/// <summary>
		/// Converts the vector into the type `DirectX::XMUINT2`.
		/// </summary>
		inline operator DirectX::XMUINT2() const noexcept;
#endif
	};

	/// <summary>
	/// A vector that contains two signed integers.
	/// </summary>
	class LITEFX_MATH_API Vector2i : public Vector<Int32, 2> {
	public:
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		/// <summary>
		/// Converts a vector of type `glm::i32vec2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2i(const glm::i32vec2& v) noexcept;

		/// <summary>
		/// Converts a vector of type `glm::i32vec2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2i(glm::i32vec2&& v) noexcept;

		/// <summary>
		/// Converts the vector into the type `glm::i32vec2`.
		/// </summary>
		inline operator glm::i32vec2() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		/// <summary>
		/// Converts a vector of type `DirectX::XMVECTOR`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2i(const DirectX::XMVECTOR& v) noexcept;

		/// <summary>
		/// Converts a vector of type `DirectX::XMVECTOR`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2i(DirectX::XMVECTOR&& v) noexcept;

		/// <summary>
		/// Converts a vector of type `DirectX::XMINT2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2i(const DirectX::XMINT2& v) noexcept;

		/// <summary>
		/// Converts a vector of type `DirectX::XMINT2`.
		/// </summary>
		/// <param name="v">The vector to convert.</param>
		Vector2i(DirectX::XMINT2&& v) noexcept;

		/// <summary>
		/// Converts the vector into the type `DirectX::XMVECTOR`.
		/// </summary>
		inline operator DirectX::XMVECTOR() const noexcept;

		/// <summary>
		/// Converts the vector into the type `DirectX::XMINT2`.
		/// </summary>
		inline operator DirectX::XMINT2() const noexcept;
#endif
	};

	/// <summary>
	/// A vector that stores three floats.
	/// </summary>
	class LITEFX_MATH_API Vector3f : public Vector<Float, 3> {
	public:
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		Vector3f(const glm::f32vec3& v) noexcept;
		Vector3f(glm::f32vec3&& v) noexcept;
		inline operator glm::f32vec3() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
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
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		Vector3u(const glm::u32vec3& v) noexcept;
		Vector3u(glm::u32vec3&& v) noexcept;
		inline operator glm::u32vec3() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
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
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		Vector3i(const glm::i32vec3& v) noexcept;
		Vector3i(glm::i32vec3&& v) noexcept;
		inline operator glm::i32vec3() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
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
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		Vector4f(const glm::f32vec4& v) noexcept;
		Vector4f(glm::f32vec4&& v) noexcept;
		inline operator glm::f32vec4() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
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
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		Vector4u(const glm::u32vec4& v) noexcept;
		Vector4u(glm::u32vec4&& v) noexcept;
		inline operator glm::u32vec4() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
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
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		Vector4i(const glm::i32vec4& v) noexcept;
		Vector4i(glm::i32vec4&& v) noexcept;
		inline operator glm::i32vec4() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		Vector4i(const DirectX::XMVECTOR& v) noexcept;
		Vector4i(DirectX::XMVECTOR&& v) noexcept;
		Vector4i(const DirectX::XMINT4& v) noexcept;
		Vector4i(DirectX::XMINT4&& v) noexcept;
		inline operator DirectX::XMVECTOR() const noexcept;
		inline operator DirectX::XMINT4() const noexcept;
#endif
	};

	/// <summary>
	/// Contains additional exported vector types.
	/// </summary>
	namespace Vectors {
		/// <summary>
		/// A vector that contains a single byte.
		/// </summary>
		using ByteVector1 = TVector1<Byte>;

		/// <summary>
		/// A vector that contains two bytes.
		/// </summary>
		using ByteVector2 = TVector2<Byte>;

		/// <summary>
		/// A vector that contains three bytes.
		/// </summary>
		using ByteVector3 = TVector3<Byte>;

		/// <summary>
		/// A vector that contains four bytes.
		/// </summary>
		using ByteVector4 = TVector4<Byte>;

		/// <summary>
		/// A vector that contains a single 16 bit signed integer.
		/// </summary>
		using Int16Vector1 = TVector1<Int16>;

		/// <summary>
		/// A vector that contains two 16 bit signed integers.
		/// </summary>
		using Int16Vector2 = TVector2<Int16>;

		/// <summary>
		/// A vector that contains three 16 bit signed integers.
		/// </summary>
		using Int16Vector3 = TVector3<Int16>;

		/// <summary>
		/// A vector that contains four 16 bit signed integers.
		/// </summary>
		using Int16Vector4 = TVector4<Int16>;

		/// <summary>
		/// A vector that contains a single 16 bit unsigned integer.
		/// </summary>
		using UInt16Vector1 = TVector1<UInt16>;

		/// <summary>
		/// A vector that contains two 16 bit unsigned integers.
		/// </summary>
		using UInt16Vector2 = TVector2<UInt16>;

		/// <summary>
		/// A vector that contains three 16 bit unsigned integers.
		/// </summary>
		using UInt16Vector3 = TVector3<UInt16>;

		/// <summary>
		/// A vector that contains four 16 bit unsigned integers.
		/// </summary>
		using UInt16Vector4 = TVector4<UInt16>;

		/// <summary>
		/// A vector that contains a single 32 bit signed integer.
		/// </summary>
		using Int32Vector1 = TVector1<Int32>;

		/// <summary>
		/// A vector that contains two 32 bit signed integers.
		/// </summary>
		using Int32Vector2 = TVector2<Int32>;

		/// <summary>
		/// A vector that contains three 32 bit signed integers.
		/// </summary>
		using Int32Vector3 = TVector3<Int32>;

		/// <summary>
		/// A vector that contains four 32 bit signed integers.
		/// </summary>
		using Int32Vector4 = TVector4<Int32>;

		/// <summary>
		/// A vector that contains a single 32 bit unsigned integer.
		/// </summary>
		using UInt32Vector1 = TVector1<UInt32>;

		/// <summary>
		/// A vector that contains two 32 bit unsigned integers.
		/// </summary>
		using UInt32Vector2 = TVector2<UInt32>;

		/// <summary>
		/// A vector that contains three 32 bit unsigned integers.
		/// </summary>
		using UInt32Vector3 = TVector3<UInt32>;

		/// <summary>
		/// A vector that contains four 32 bit unsigned integers.
		/// </summary>
		using UInt32Vector4 = TVector4<UInt32>;

		/// <summary>
		/// A vector that contains a single 64 bit signed integer.
		/// </summary>
		using Int64Vector1 = TVector1<Int64>;

		/// <summary>
		/// A vector that contains two 64 bit signed integers.
		/// </summary>
		using Int64Vector2 = TVector2<Int64>;

		/// <summary>
		/// A vector that contains three 64 bit signed integers.
		/// </summary>
		using Int64Vector3 = TVector3<Int64>;

		/// <summary>
		/// A vector that contains four 64 bit signed integers.
		/// </summary>
		using Int64Vector4 = TVector4<Int64>;

		/// <summary>
		/// A vector that contains a single 64 bit unsigned integer.
		/// </summary>
		using UInt64Vector1 = TVector1<UInt64>;

		/// <summary>
		/// A vector that contains two 64 bit unsigned integers.
		/// </summary>
		using UInt64Vector2 = TVector2<UInt64>;

		/// <summary>
		/// A vector that contains three 64 bit unsigned integers.
		/// </summary>
		using UInt64Vector3 = TVector3<UInt64>;

		/// <summary>
		/// A vector that contains four 64 bit unsigned integers.
		/// </summary>
		using UInt64Vector4 = TVector4<UInt64>;

		/// <summary>
		/// A vector that contains a single floating point value with single precision.
		/// </summary>
		using FloatVector1 = TVector1<Float>;

		/// <summary>
		/// A vector that contains two floating point values with single precision.
		/// </summary>
		using FloatVector2 = TVector2<Float>;

		/// <summary>
		/// A vector that contains three floating point values with single precision.
		/// </summary>
		using FloatVector3 = TVector3<Float>;

		/// <summary>
		/// A vector that contains four floating point values with single precision.
		/// </summary>
		using FloatVector4 = TVector4<Float>;

		/// <summary>
		/// A vector that contains a single floating point value with double precision.
		/// </summary>
		using DoubleVector1 = TVector1<Double>;

		/// <summary>
		/// A vector that contains two floating point values with double precision.
		/// </summary>
		using DoubleVector2 = TVector2<Double>;

		/// <summary>
		/// A vector that contains three floating point values with double precision.
		/// </summary>
		using DoubleVector3 = TVector3<Double>;

		/// <summary>
		/// A vector that contains four floating point values with double precision.
		/// </summary>
		using DoubleVector4 = TVector4<Double>;
	}
#pragma endregion

#pragma region Size
	class LITEFX_MATH_API Size4d : public Vector<size_t, 4> {
	public:
		Size4d() noexcept;
		Size4d(size_t v) noexcept;
		Size4d(size_t w, size_t h, size_t d, size_t a) noexcept;
		Size4d(const Size4d&) noexcept;
		Size4d(Size4d&&) noexcept;
		//virtual ~Size4d() noexcept = default;

	public:
		inline Size4d& operator=(const Size4d& _other) noexcept;
		inline Size4d& operator=(Size4d&& _other) noexcept;
		inline Size4d operator/(size_t s) noexcept;
		inline Size4d& operator/=(size_t s) noexcept;
		inline Size4d operator*(size_t s) noexcept;
		inline Size4d& operator*=(size_t s) noexcept;
		inline Size4d operator+(const Size4d& s) noexcept;
		inline Size4d& operator+=(const Size4d& s) noexcept;
		inline Size4d operator-(const Size4d& s) noexcept;
		inline Size4d& operator-=(const Size4d& s) noexcept;

	public:
		inline size_t width() const noexcept;
		inline size_t& width() noexcept;
		inline size_t height() const noexcept;
		inline size_t& height() noexcept;
		inline size_t depth() const noexcept;
		inline size_t& depth() noexcept;
		inline size_t alpha() const noexcept;
		inline size_t& alpha() noexcept;
	};

	class LITEFX_MATH_API Size3d : public Vector<size_t, 3> {
	public:
		Size3d() noexcept;
		Size3d(size_t v) noexcept;
		Size3d(size_t w, size_t h, size_t d) noexcept;
		Size3d(const Size3d&) noexcept;
		Size3d(Size3d&&) noexcept;
		//virtual ~Size3d() noexcept = default;

	public:
		inline Size3d& operator=(const Size3d& _other) noexcept;
		inline Size3d& operator=(Size3d&& _other) noexcept;
		inline operator Size4d() const noexcept;
		inline Size3d operator/(size_t s) noexcept;
		inline Size3d& operator/=(size_t s) noexcept;
		inline Size3d operator*(size_t s) noexcept;
		inline Size3d& operator*=(size_t s) noexcept;
		inline Size3d operator+(const Size3d& s) noexcept;
		inline Size3d& operator+=(const Size3d& s) noexcept;
		inline Size3d operator-(const Size3d& s) noexcept;
		inline Size3d& operator-=(const Size3d& s) noexcept;

	public:
		inline size_t width() const noexcept;
		inline size_t& width() noexcept;
		inline size_t height() const noexcept;
		inline size_t& height() noexcept;
		inline size_t depth() const noexcept;
		inline size_t& depth() noexcept;
	};

	class LITEFX_MATH_API Size2d : public Vector<size_t, 2> {
	public:
		Size2d() noexcept;
		Size2d(size_t v) noexcept;
		Size2d(size_t w, size_t h) noexcept;
		Size2d(const Size2d&) noexcept;
		Size2d(Size2d&&) noexcept;
		//virtual ~Size2d() noexcept = default;

	public:
		inline Size2d& operator=(const Size2d& _other) noexcept;
		inline Size2d& operator=(Size2d&& _other) noexcept;
		inline operator Size3d() const noexcept;
		inline operator Size4d() const noexcept;
		inline Size2d operator/(size_t s) noexcept;
		inline Size2d& operator/=(size_t s) noexcept;
		inline Size2d operator*(size_t s) noexcept;
		inline Size2d& operator*=(size_t s) noexcept;
		inline Size2d operator+(const Size2d& s) noexcept;
		inline Size2d& operator+=(const Size2d& s) noexcept;
		inline Size2d operator-(const Size2d& s) noexcept;
		inline Size2d& operator-=(const Size2d& s) noexcept;

	public:
		inline size_t width() const noexcept;
		inline size_t& width() noexcept;
		inline size_t height() const noexcept;
		inline size_t& height() noexcept;
	};
#pragma endregion

#pragma region Rectangle
	class LITEFX_MATH_API Rect : public Vector<size_t, 4> {
	public:
		Rect() noexcept;
		Rect(const Vector<size_t, 2>& pos, size_t w, size_t h) noexcept;
		Rect(size_t x, size_t y, size_t w, size_t h) noexcept;
		Rect(const Rect&) noexcept;
		Rect(Rect&&) noexcept;
		//virtual ~Rect() noexcept = default;

	public:
		inline Rect& operator=(const Rect& _other) noexcept;
		inline Rect& operator=(Rect&& _other) noexcept;

	public:
		inline Vector<size_t, 2> position() const noexcept;
		inline Size2d extent() const noexcept;
		inline size_t width() const noexcept;
		inline size_t& width() noexcept;
		inline size_t height() const noexcept;
		inline size_t& height() noexcept;
	};

	class LITEFX_MATH_API RectI : public Vector<Int32, 4> {
	public:
		RectI() noexcept;
		RectI(const Vector<Int32, 2>& pos, Int32 w, Int32 h) noexcept;
		RectI(Int32 x, Int32 y, Int32 w, Int32 h) noexcept;
		RectI(const RectI&) noexcept;
		RectI(RectI&&) noexcept;
		//virtual ~RectI() noexcept = default;

	public:
		inline RectI& operator=(const RectI& _other) noexcept;
		inline RectI& operator=(RectI&& _other) noexcept;

	public:
		inline Vector<Int32, 2> position() const noexcept;
		inline Size2d extent() const noexcept;
		inline Int32 width() const noexcept;
		inline Int32& width() noexcept;
		inline Int32 height() const noexcept;
		inline Int32& height() noexcept;
	};

	class LITEFX_MATH_API RectF : public Vector<Float, 4> {
	public:
		RectF() noexcept;
		RectF(const Vector<Float, 2>& pos, Float w, Float h) noexcept;
		RectF(Float x, Float y, Float w, Float h) noexcept;
		RectF(const RectF&) noexcept;
		RectF(RectF&&) noexcept;
		//virtual ~RectF() noexcept = default;

	public:
		inline RectF& operator=(const RectF& _other) noexcept;
		inline RectF& operator=(RectF&& _other) noexcept;

	public:
		inline Vector<Float, 2> position() const noexcept;
		inline Size2d extent() const noexcept;
		inline Float width() const noexcept;
		inline Float& width() noexcept;
		inline Float height() const noexcept;
		inline Float& height() noexcept;
	};
#pragma endregion
}