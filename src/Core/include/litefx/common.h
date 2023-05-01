#pragma once

#ifndef DEFINE_EXCEPTION
#  define DEFINE_EXCEPTION(name, base) \
	class name : public ExceptionBase<base, name> { \
	public: \
		using ExceptionBase<base, name>::ExceptionBase; \
	}
#endif // DEFINE_EXCEPTION

#ifndef LITEFX_BUILDER
#  define LITEFX_BUILDER(BuilderType) \
	public: \
		using builder_type = BuilderType; \
		friend class BuilderType;
#endif // LITEFX_BUILDER

#ifndef LITEFX_DEFINE_FLAGS
#  define LITEFX_DEFINE_FLAGS(T) \
	inline T operator| (const T lhs, const T rhs) { using _base_t = std::underlying_type_t<T>; return static_cast<T>(static_cast<_base_t>(lhs) | static_cast<_base_t>(rhs)); } \
	inline T& operator|= (T& lhs, const T& rhs) { lhs = lhs | rhs; return lhs; } \
	inline T operator& (const T lhs, const T rhs) { using _base_t = std::underlying_type_t<T>; return static_cast<T>(static_cast<_base_t>(lhs) & static_cast<_base_t>(rhs)); } \
	inline T& operator&= (T& lhs, const T& rhs) { lhs = lhs & rhs; return lhs; }
#endif // LITEFX_DEFINE_FLAGS

#ifndef LITEFX_FLAG_IS_SET
#  define LITEFX_FLAG_IS_SET(val, flag) \
	static_cast<bool>((std::to_underlying(val) & std::to_underlying(flag)) == std::to_underlying(flag))
#endif // LITEFX_FLAG_IS_SET

/// <summary>
/// Declares the implementation for the public interface of a class.
/// </summary>
/// <remarks>
/// A class can access the instance of the implementation instance using the pointer `m_impl` after declaring the implementation
/// using this macro.
/// </remarks>
/// <seealso cref="Implement" />
#ifndef LITEFX_IMPLEMENTATION
#  define LITEFX_IMPLEMENTATION(impl) \
	private: \
		class impl; \
		PimplPtr<impl> m_impl;
#endif // LITEFX_IMPLEMENTATION