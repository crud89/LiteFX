#pragma once

// Disable warning C4251: STL class needs to have dll-interface to be used by clients.
// See: https://stackoverflow.com/a/22054743/1254352
#pragma warning(disable: 4251)

#include <algorithm>
#include <string>
#include <optional>
#include <map>
#include <vector>
#include <tuple>
#include <memory>
#include <stdexcept>
#include <functional>
#include <variant>

#ifndef LITEFX_DEFINE_FLAGS
#  define LITEFX_DEFINE_FLAGS(T) \
	inline T operator| (const T lhs, const T rhs) { using _base_t = std::underlying_type_t<T>; return static_cast<T>(static_cast<_base_t>(lhs) | static_cast<_base_t>(rhs)); } \
	inline T& operator|= (T& lhs, const T& rhs) { lhs = lhs | rhs; return lhs; } \
	inline T operator& (const T lhs, const T rhs) { using _base_t = std::underlying_type_t<T>; return static_cast<T>(static_cast<_base_t>(lhs) & static_cast<_base_t>(rhs)); } \
	inline T& operator&= (T& lhs, const T& rhs) { lhs = lhs & rhs; return lhs; }
#endif

#ifndef LITEFX_FLAG_IS_SET
#  define LITEFX_FLAG_IS_SET(val, flag) static_cast<int>(val & flag) != 0
#endif

namespace LiteFX {

	using String = std::string;
	using Exception = std::exception;
	using Handle = void*;

	template<class TKey, class TVal>
	using Dictionary = std::map<TKey, TVal>;

	template<class T>
	using Array = std::vector<T>;

	template<class T>
	using Optional = std::optional<T>;

	template<class T, class TDeleter = std::default_delete<T>>
	using UniquePtr = std::unique_ptr<T, TDeleter>;

	template <class T>
	using SharedPtr = std::shared_ptr<T>;

	template <class... T>
	using Tuple = std::tuple<T...>;

	template <class... T>
	using Variant = std::variant<T...>;

	template <class T>
	UniquePtr<T> makeUnique() {
		return std::make_unique<T>();
	}

	template <class T, class... TArgs>
	UniquePtr<T> makeUnique(TArgs&&... _args) {
		return std::make_unique<T>(std::forward<TArgs>(_args)...);
	}

	template <class T>
	SharedPtr<T> makeShared() {
		return std::make_shared<T>()
	}

	template <class T, class... TArgs>
	SharedPtr<T> makeShared(TArgs&&... _args) {
		return std::make_shared<T>(std::forward<TArgs>(_args)...);
	}

	template <class T>
	SharedPtr<T> makeShared(UniquePtr<T>& ptr) {
		return std::make_shared<T>(ptr.release());
	}

#if (defined(BUILD_LITEFX_PIMPL) && BUILD_LITEFX_PIMPL) || (!defined(BUILD_LITEFX_PIMPL)) && !defined(LITEFX_IMPLEMENTATION)
	template <class pImpl>
	class PimplPtr {
	private:
		UniquePtr<pImpl> m_ptr;

	public:
		PimplPtr() noexcept = default;
		PimplPtr(const PimplPtr& src) noexcept : m_ptr(new pImpl(*src.m_ptr)) {}
		PimplPtr(PimplPtr&&) noexcept = default;
		PimplPtr& operator= (const PimplPtr& src) noexcept { m_ptr.reset(new pImpl(*src.m_ptr)); return *this; }
		PimplPtr& operator= (PimplPtr&&) noexcept = default;
		~PimplPtr() noexcept = default;

	public:
		void destroy() { m_ptr = nullptr; }

	private:
		PimplPtr(pImpl* pimpl) : m_ptr(pimpl) { }

	public:
		pImpl& operator* () const noexcept { return *m_ptr; }
		pImpl* operator-> () const noexcept { return m_ptr.get(); }

	public:
		template <class T, class... Arg>
		friend PimplPtr<T> makePimpl(Arg&&... arg);
	};

	template <class T, class... Arg>
	PimplPtr<T> makePimpl(Arg&&... arg) {
		return PimplPtr<T>(new T(std::forward<Arg>(arg)...));
	}

#  define LITEFX_IMPLEMENTATION(impl) private: \
	class impl; \
	PimplPtr<impl> m_impl;
#endif

	template <class THandle>
	class IResource {
	private:
		THandle m_handle;

	protected:
		IResource(const THandle handle) noexcept : m_handle(handle) { }

	public:
		IResource(const IResource&) = delete;
		IResource(IResource&&) = delete;
		virtual ~IResource() noexcept = default;

	protected:
		THandle& handle() noexcept { return m_handle; }

	public:
		THandle handle() const noexcept { return m_handle; }
	};

	template <typename T>
	class IBuilder {
	public:
		///
		/// TODO: Use this everywhere and rename this project to goFX!
		///
		virtual UniquePtr<T> go() = 0;
	};

	template <typename T, typename TBuilder>
	class Initializer {
	public:
		using type = T;
		using builder_type = TBuilder;
		using initializer_type = Initializer<type, builder_type>;

	private:
		UniquePtr<type> m_instance;
		TBuilder& m_builder;

	public:
		Initializer(TBuilder& builder, UniquePtr<T>&& instance) noexcept : m_builder(std::move(builder)), m_instance(std::move(instance)) { }
		Initializer(const Initializer&) = delete;
		Initializer(Initializer&&) = delete;
		virtual ~Initializer() noexcept = default;

	public:
		virtual TBuilder& use() { m_builder.use(std::move(m_instance)); return m_builder; }
	};

}