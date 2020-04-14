#pragma once

// Disable warning C4251: STL class needs to have dll-interface to be used by clients.
// See: https://stackoverflow.com/a/22054743/1254352
#pragma warning(disable: 4251)

#include <cassert>
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

#include "traits.hpp"

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

	template <typename TDerived, typename T, typename TParent = std::nullptr_t>
	class Builder;

	template <typename TDerived, typename T>
	class Builder<typename TDerived, T, std::nullptr_t> {
	private:
		UniquePtr<T> m_instance;

	public:
		using derived_type = TDerived;
		using instance_type = T;
		using parent_type = std::nullptr_t;
		using builder_type = Builder<derived_type, instance_type, parent_type>;

	protected:
		T* instance() const noexcept { return m_instance.get(); }

	public:
		Builder(UniquePtr<T>&& instance) noexcept : m_instance(std::move(instance)) { }
		Builder(const builder_type&) = delete;
		Builder(builder_type&& _other) noexcept : m_instance(std::move(_other.m_instance)) { }
		virtual ~Builder() noexcept = default;

	public:
		template <typename TInstance, typename ...TArgs, std::enable_if_t<rtti::has_builder_v<TInstance>, int> = 0, typename TBuilder = TInstance::builder>
		TBuilder make(TArgs&&... _args) {
			static_assert(std::is_convertible_v<TDerived*, typename TBuilder::parent_type*>, "The provided builder requires a different parent.");
			return TBuilder(*static_cast<TDerived*>(this), makeUnique<TInstance>(*m_instance.get(), std::forward<TArgs>(_args)...));
		}

		template <typename TInstance, typename ...TArgs, std::enable_if_t<!rtti::has_builder_v<TInstance>, int> = 0, typename TBuilder = Builder<TInstance, builder_type>>
		TBuilder make(TArgs&&... _args) {
			static_assert(std::is_convertible_v<TDerived*, typename TBuilder::parent_type*>, "The provided builder requires a different parent.");
			return TBuilder(*static_cast<TDerived*>(this), makeUnique<TInstance>(*m_instance.get(), std::forward<TArgs>(_args)...));
		}

		template <typename TInstance>
		void use(UniquePtr<TInstance>&&) { throw std::runtime_error("The usage overload provided for this type."); }

		virtual UniquePtr<T> go() {
			return std::move(m_instance);
		}

		template <typename TInstance>
		UniquePtr<TInstance> goFor() {
			return UniquePtr<TInstance>(dynamic_cast<TInstance*>(this->go().release()));
		}
	};

	template <typename TDerived, typename T, typename TParent>
	class Builder {
	private:
		UniquePtr<T> m_instance;
		TParent& m_parent;

	public:
		using derived_type = TDerived;
		using instance_type = T;
		using parent_type = TParent;
		using builder_type = Builder<derived_type, instance_type, parent_type>;

	protected:
		const T* instance() const noexcept { return m_instance.get(); }
		//const TParent& parent() const noexcept { return m_parent; }

	public:
		Builder(TParent& parent, UniquePtr<T>&& instance) noexcept : m_parent(parent), m_instance(std::move(instance)) { }
		Builder(const builder_type&) = delete;
		Builder(builder_type&& _other) noexcept : m_instance(std::move(_other.m_instance)), m_parent(_other.m_parent) { }
		virtual ~Builder() noexcept = default;

	public:
		template <typename TInstance, typename ...TArgs, std::enable_if_t<rtti::has_builder_v<TInstance>, int> = 0, typename TBuilder = TInstance::builder>
		TBuilder make(TArgs&&... _args) {
			static_assert(std::is_convertible_v<TDerived*, typename TBuilder::parent_type*>, "The provided builder requires a different parent.");
			return TBuilder(*static_cast<TDerived*>(this), makeUnique<TInstance>(*m_instance.get(), std::forward<TArgs>(_args)...));
		}

		template <typename TInstance, typename ...TArgs, std::enable_if_t<!rtti::has_builder_v<TInstance>, int> = 0, typename TBuilder = Builder<TInstance, builder_type>>
		TBuilder make(TArgs&&... _args) {
			static_assert(std::is_convertible_v<TDerived*, typename TBuilder::parent_type*>, "The provided builder requires a different parent.");
			return TBuilder(*static_cast<TDerived*>(this), makeUnique<TInstance>(*m_instance.get(), std::forward<TArgs>(_args)...));
		}

		virtual TParent& go() {
			m_parent.use(std::move(m_instance));
			return m_parent;
		}
	};

}