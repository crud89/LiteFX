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
	constexpr Byte operator ""_b(unsigned long long int arg) noexcept {
		return static_cast<Byte>(arg);
	}

	/// <summary>
	/// A literal to define a 16 bit integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 16 bit integer.</returns>
	constexpr Int16 operator ""_i16(unsigned long long int arg) noexcept {
		return static_cast<Int16>(arg);
	}

	/// <summary>
	/// A literal to define a 16 bit unsigned integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 16 bit unsigned integer.</returns>
	constexpr UInt16 operator ""_ui16(unsigned long long int arg) noexcept {
		return static_cast<UInt16>(arg);
	}

	/// <summary>
	/// A literal to define a 32 bit integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 32 bit integer.</returns>
	constexpr Int32 operator ""_i32(unsigned long long int arg) noexcept {
		return static_cast<Int32>(arg);
	}

	/// <summary>
	/// A literal to define a 32 bit unsigned integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 32 bit unsigned integer.</returns>
	constexpr UInt32 operator ""_ui32(unsigned long long int arg) noexcept {
		return static_cast<UInt32>(arg);
	}

	/// <summary>
	/// A literal to define a 64 bit integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 64 bit integer.</returns>
	constexpr Int64 operator ""_i64(unsigned long long int arg) noexcept {
		return static_cast<Int64>(arg);
	}

	/// <summary>
	/// A literal to define a 64 bit unsigned integer.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the integer.</param>
	/// <returns>The value as 64 bit unsigned integer.</returns>
	constexpr UInt64 operator ""_ui64(unsigned long long int arg) noexcept {
		return static_cast<UInt64>(arg);
	}

	/// <summary>
	/// A literal to define a floating point value with single precision.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the floating point number.</param>
	/// <returns>The value as floating point number.</returns>
	constexpr Float operator ""_f32(long double arg) noexcept {
		return static_cast<Float>(arg);
	}

	/// <summary>
	/// A literal to define a floating point value with double precision.
	/// </summary>
	/// <param name="arg">The value that should be assigned to the floating point number.</param>
	/// <returns>The value as floating point number.</returns>
	constexpr Double operator ""_f64(long double arg) noexcept {
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
	static constexpr T align(T size, T alignment) { 
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
		operator glm::f32vec1() const noexcept;
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
		operator DirectX::XMVECTOR() const noexcept;
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
		operator glm::u32vec1() const noexcept;
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
		operator DirectX::XMVECTOR() const noexcept;
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
		operator glm::f32vec2() const noexcept;
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
		operator DirectX::XMVECTOR() const noexcept;

		/// <summary>
		/// Converts the vector into the type `DirectX::XMFLOAT2`.
		/// </summary>
		operator DirectX::XMFLOAT2() const noexcept;
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
		operator glm::u32vec2() const noexcept;
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
		operator DirectX::XMVECTOR() const noexcept;

		/// <summary>
		/// Converts the vector into the type `DirectX::XMUINT2`.
		/// </summary>
		operator DirectX::XMUINT2() const noexcept;
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
		operator glm::i32vec2() const noexcept;
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
		operator DirectX::XMVECTOR() const noexcept;

		/// <summary>
		/// Converts the vector into the type `DirectX::XMINT2`.
		/// </summary>
		operator DirectX::XMINT2() const noexcept;
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
		operator glm::f32vec3() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		Vector3f(const DirectX::XMVECTOR& v) noexcept;
		Vector3f(DirectX::XMVECTOR&& v) noexcept;
		Vector3f(const DirectX::XMFLOAT3& v) noexcept;
		Vector3f(DirectX::XMFLOAT3&& v) noexcept;
		operator DirectX::XMVECTOR() const noexcept;
		operator DirectX::XMFLOAT3() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector3u : public Vector<UInt32, 3> {
	public:
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		Vector3u(const glm::u32vec3& v) noexcept;
		Vector3u(glm::u32vec3&& v) noexcept;
		operator glm::u32vec3() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		Vector3u(const DirectX::XMVECTOR& v) noexcept;
		Vector3u(DirectX::XMVECTOR&& v) noexcept;
		Vector3u(const DirectX::XMUINT3& v) noexcept;
		Vector3u(DirectX::XMUINT3&& v) noexcept;
		operator DirectX::XMVECTOR() const noexcept;
		operator DirectX::XMUINT3() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector3i : public Vector<Int32, 3> {
	public:
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		Vector3i(const glm::i32vec3& v) noexcept;
		Vector3i(glm::i32vec3&& v) noexcept;
		operator glm::i32vec3() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		Vector3i(const DirectX::XMVECTOR& v) noexcept;
		Vector3i(DirectX::XMVECTOR&& v) noexcept;
		Vector3i(const DirectX::XMINT3& v) noexcept;
		Vector3i(DirectX::XMINT3&& v) noexcept;
		operator DirectX::XMVECTOR() const noexcept;
		operator DirectX::XMINT3() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector4f : public Vector<Float, 4> {
	public:
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		Vector4f(const glm::f32vec4& v) noexcept;
		Vector4f(glm::f32vec4&& v) noexcept;
		operator glm::f32vec4() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		Vector4f(const DirectX::XMVECTOR& v) noexcept;
		Vector4f(DirectX::XMVECTOR&& v) noexcept;
		Vector4f(const DirectX::XMFLOAT4& v) noexcept;
		Vector4f(DirectX::XMFLOAT4&& v) noexcept;
		operator DirectX::XMVECTOR() const noexcept;
		operator DirectX::XMFLOAT4() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector4u : public Vector<UInt32, 4> {
	public:
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		Vector4u(const glm::u32vec4& v) noexcept;
		Vector4u(glm::u32vec4&& v) noexcept;
		operator glm::u32vec4() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		Vector4u(const DirectX::XMVECTOR& v) noexcept;
		Vector4u(DirectX::XMVECTOR&& v) noexcept;
		Vector4u(const DirectX::XMUINT4& v) noexcept;
		Vector4u(DirectX::XMUINT4&& v) noexcept;
		operator DirectX::XMVECTOR() const noexcept;
		operator DirectX::XMUINT4() const noexcept;
#endif
	};

	class LITEFX_MATH_API Vector4i : public Vector<Int32, 4> {
	public:
		using Vector::Vector;

#if defined(LITEFX_BUILD_WITH_GLM)
	public:
		Vector4i(const glm::i32vec4& v) noexcept;
		Vector4i(glm::i32vec4&& v) noexcept;
		operator glm::i32vec4() const noexcept;
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
	public:
		Vector4i(const DirectX::XMVECTOR& v) noexcept;
		Vector4i(DirectX::XMVECTOR&& v) noexcept;
		Vector4i(const DirectX::XMINT4& v) noexcept;
		Vector4i(DirectX::XMINT4&& v) noexcept;
		operator DirectX::XMVECTOR() const noexcept;
		operator DirectX::XMINT4() const noexcept;
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
		Size4d(const Size4d&);
		Size4d(Size4d&&) noexcept;
		~Size4d() noexcept = default;

	public:
		Size4d& operator=(const Size4d& _other);
		Size4d& operator=(Size4d&& _other) noexcept;
		Size4d operator/(size_t s) noexcept;
		Size4d& operator/=(size_t s) noexcept;
		Size4d operator*(size_t s) noexcept;
		Size4d& operator*=(size_t s) noexcept;
		Size4d operator+(const Size4d& s) noexcept;
		Size4d& operator+=(const Size4d& s) noexcept;
		Size4d operator-(const Size4d& s) noexcept;
		Size4d& operator-=(const Size4d& s) noexcept;

	public:
		size_t width() const noexcept;
		size_t& width() noexcept;
		size_t height() const noexcept;
		size_t& height() noexcept;
		size_t depth() const noexcept;
		size_t& depth() noexcept;
		size_t alpha() const noexcept;
		size_t& alpha() noexcept;
	};

	class LITEFX_MATH_API Size3d : public Vector<size_t, 3> {
	public:
		Size3d() noexcept;
		Size3d(size_t v) noexcept;
		Size3d(size_t w, size_t h, size_t d) noexcept;
		Size3d(const Size3d&);
		Size3d(Size3d&&) noexcept;
		~Size3d() noexcept = default;

	public:
		Size3d& operator=(const Size3d& _other);
		Size3d& operator=(Size3d&& _other) noexcept;
		operator Size4d() const noexcept;
		Size3d operator/(size_t s) noexcept;
		Size3d& operator/=(size_t s) noexcept;
		Size3d operator*(size_t s) noexcept;
		Size3d& operator*=(size_t s) noexcept;
		Size3d operator+(const Size3d& s) noexcept;
		Size3d& operator+=(const Size3d& s) noexcept;
		Size3d operator-(const Size3d& s) noexcept;
		Size3d& operator-=(const Size3d& s) noexcept;

	public:
		size_t width() const noexcept;
		size_t& width() noexcept;
		size_t height() const noexcept;
		size_t& height() noexcept;
		size_t depth() const noexcept;
		size_t& depth() noexcept;
	};

	class LITEFX_MATH_API Size2d : public Vector<size_t, 2> {
	public:
		Size2d() noexcept;
		Size2d(size_t v) noexcept;
		Size2d(size_t w, size_t h) noexcept;
		Size2d(const Size2d&);
		Size2d(Size2d&&) noexcept;
		~Size2d() noexcept = default;

	public:
		Size2d& operator=(const Size2d& _other);
		Size2d& operator=(Size2d&& _other) noexcept;
		operator Size3d() const noexcept;
		operator Size4d() const noexcept;
		Size2d operator/(size_t s) noexcept;
		Size2d& operator/=(size_t s) noexcept;
		Size2d operator*(size_t s) noexcept;
		Size2d& operator*=(size_t s) noexcept;
		Size2d operator+(const Size2d& s) noexcept;
		Size2d& operator+=(const Size2d& s) noexcept;
		Size2d operator-(const Size2d& s) noexcept;
		Size2d& operator-=(const Size2d& s) noexcept;

	public:
		size_t width() const noexcept;
		size_t& width() noexcept;
		size_t height() const noexcept;
		size_t& height() noexcept;
	};
#pragma endregion

#pragma region Rectangle
	class LITEFX_MATH_API Rect : public Vector<size_t, 4> {
	public:
		Rect() noexcept;
		Rect(const Vector<size_t, 2>& pos, size_t w, size_t h) noexcept;
		Rect(size_t x, size_t y, size_t w, size_t h) noexcept;
		Rect(const Rect&);
		Rect(Rect&&) noexcept;
		~Rect() noexcept = default;

	public:
		Rect& operator=(const Rect& _other);
		Rect& operator=(Rect&& _other) noexcept;

	public:
		Vector<size_t, 2> position() const noexcept;
		Size2d extent() const noexcept;
		size_t width() const noexcept;
		size_t& width() noexcept;
		size_t height() const noexcept;
		size_t& height() noexcept;
	};

	class LITEFX_MATH_API RectI : public Vector<Int32, 4> {
	public:
		RectI() noexcept;
		RectI(const Vector<Int32, 2>& pos, Int32 w, Int32 h) noexcept;
		RectI(Int32 x, Int32 y, Int32 w, Int32 h) noexcept;
		RectI(const RectI&);
		RectI(RectI&&) noexcept;
		~RectI() noexcept = default;

	public:
		RectI& operator=(const RectI& _other);
		RectI& operator=(RectI&& _other) noexcept;

	public:
		Vector<Int32, 2> position() const noexcept;
		Size2d extent() const noexcept;
		Int32 width() const noexcept;
		Int32& width() noexcept;
		Int32 height() const noexcept;
		Int32& height() noexcept;
	};

	class LITEFX_MATH_API RectF : public Vector<Float, 4> {
	public:
		RectF() noexcept;
		RectF(const Vector<Float, 2>& pos, Float w, Float h) noexcept;
		RectF(Float x, Float y, Float w, Float h) noexcept;
		RectF(const RectF&);
		RectF(RectF&&) noexcept;
		~RectF() noexcept = default;

	public:
		RectF& operator=(const RectF& _other);
		RectF& operator=(RectF&& _other) noexcept;

	public:
		Vector<Float, 2> position() const noexcept;
		Size2d extent() const noexcept;
		Float width() const noexcept;
		Float& width() noexcept;
		Float height() const noexcept;
		Float& height() noexcept;
	};
#pragma endregion
}