#pragma once

// Disable warning C4251: STL class needs to have dll-interface to be used by clients.
// See: https://stackoverflow.com/a/22054743/1254352
#pragma warning(disable: 4251)

#include <array>
#include <cassert>
#include <algorithm>
#include <string>
#include <optional>
#include <map>
#include <vector>
#include <queue>
#include <tuple>
#include <memory>
#include <functional>
#include <variant>
#include <ranges>
#include <mutex>

#include "traits.hpp"
#include "string.hpp"
#include "exceptions.hpp"

#ifndef LITEFX_DEFINE_FLAGS
#  define LITEFX_DEFINE_FLAGS(T) \
	inline T operator| (const T lhs, const T rhs) { using _base_t = std::underlying_type_t<T>; return static_cast<T>(static_cast<_base_t>(lhs) | static_cast<_base_t>(rhs)); } \
	inline T& operator|= (T& lhs, const T& rhs) { lhs = lhs | rhs; return lhs; } \
	inline T operator& (const T lhs, const T rhs) { using _base_t = std::underlying_type_t<T>; return static_cast<T>(static_cast<_base_t>(lhs) & static_cast<_base_t>(rhs)); } \
	inline T& operator&= (T& lhs, const T& rhs) { lhs = lhs & rhs; return lhs; }
#endif

#ifndef LITEFX_FLAG_IS_SET
#  define LITEFX_FLAG_IS_SET(val, flag) (static_cast<UInt32>(val) & static_cast<UInt32>(flag)) == static_cast<UInt32>(flag)
#endif

namespace LiteFX {

	/// <summary>
	/// Represents a handle type.
	/// </summary>
	using Handle = void*;

	/// <summary>
	/// Represents a dictionary that maps a key to a certain value.
	/// </summary>
	/// <typeparam name="TKey">The type of the key.</typeparam>
	/// <typeparam name="TVal">The type of the value.</typeparam>
	template<class TKey, class TVal>
	using Dictionary = std::map<TKey, TVal>;

	/// <summary>
	/// Represents a dynamic array.
	/// </summary>
	/// <typeparam name="T">The type of the array elements.</typeparam>
	template<class T>
	using Array = std::vector<T>;

	/// <summary>
	/// Represents a queue.
	/// </summary>
	/// <typeparam name="T">The type of the queue elements.</typeparam>
	template<class T>
	using Queue = std::queue<T>;

	/// <summary>
	/// Represents a view of an array.
	/// </summary>
	/// <typeparam name="T">The type of the array elements.</typeparam>
	template<class T>
	using Span = std::span<T>;

	/// <summary>
	/// Represents an optional value.
	/// </summary>
	/// <typeparam name="T">The type of the optional value.</typeparam>
	template<class T>
	using Optional = std::optional<T>;

	/// <summary>
	/// Represents a unique pointer, that expresses exclusive ownership.
	/// </summary>
	/// <typeparam name="T">The type of the object the pointer points to.</typeparam>
	/// <typeparam name="TDeleter">The deleter for the pointed object.</typeparam>
	template<class T, class TDeleter = std::default_delete<T>>
	using UniquePtr = std::unique_ptr<T, TDeleter>;

	/// <summary>
	/// Represents a shared pointer, that expresses non-exclusive ownership.
	/// </summary>
	/// <typeparam name="T">The type of the object the pointer points to.</typeparam>
	template <class T>
	using SharedPtr = std::shared_ptr<T>;

	/// <summary>
	/// Represents a weak pointer, that expresses a reference to a shared pointer instance.
	/// </summary>
	/// <typeparam name="T">The type of the object the pointer points to.</typeparam>
	template <class T>
	using WeakPtr = std::weak_ptr<T>;

	/// <summary>
	/// Represents a tuple of multiple objects.
	/// </summary>
	/// <typeparam name="...T">The types of the objects, contained by the tuple.</typeparam>
	template <class... T>
	using Tuple = std::tuple<T...>;

	/// <summary>
	/// Represents a variant of objects.
	/// </summary>
	/// <typeparam name="...T">The types of the objects, that can be contained by the tuple.</typeparam>
	template <class... T>
	using Variant = std::variant<T...>;

	/// <summary>
	/// Creates a new unique pointer.
	/// </summary>
	/// <typeparam name="T">The type of the object, the pointer points to.</typeparam>
	/// <returns>A new unique pointer.</returns>
	template <class T>
	UniquePtr<T> makeUnique() {
		return std::make_unique<T>();
	}

	/// <summary>
	/// Creates a new unique pointer.
	/// </summary>
	/// <typeparam name="T">The type of the object, the pointer points to.</typeparam>
	/// <returns>A new unique pointer.</returns>
	template <class T, class... TArgs>
	UniquePtr<T> makeUnique(TArgs&&... _args) {
		return std::make_unique<T>(std::forward<TArgs>(_args)...);
	}

	/// <summary>
	/// Creates a new shared pointer.
	/// </summary>
	/// <typeparam name="T">The type of the object, the pointer points to.</typeparam>
	/// <returns>A new shared pointer.</returns>
	template <class T>
	SharedPtr<T> makeShared() {
		return std::make_shared<T>();
	}

	/// <summary>
	/// Creates a new shared pointer.
	/// </summary>
	/// <typeparam name="T">The type of the object, the pointer points to.</typeparam>
	/// <returns>A new shared pointer.</returns>
	template <class T, class... TArgs>
	SharedPtr<T> makeShared(TArgs&&... _args) {
		return std::make_shared<T>(std::forward<TArgs>(_args)...);
	}

	/// <summary>
	/// Transfers a unique pointer to a shared pointer. The unique pointer will be released during this process.
	/// </summary>
	/// <typeparam name="T">The type of the object, the pointer points to.</typeparam>
	/// <param name="ptr">The unique pointer that should be turned into a shared pointer.</param>
	/// <returns>A new shared pointer.</returns>
	template <class T>
	SharedPtr<T> makeShared(UniquePtr<T>&& ptr) {
		return std::make_shared<T>(ptr.release());
	}

	/// <summary>
	/// Contains helpers for working with ranges and views.
	/// </summary>
	namespace ranges {

		template <typename TContainer>
		struct to_container { };

		template <typename TContainer, std::ranges::range TRange> requires 
			std::convertible_to<std::ranges::range_value_t<TRange>, typename TContainer::value_type>
		inline TContainer operator|(TRange&& range, to_container<TContainer>) {
			auto it = range | std::views::common;
			return TContainer{ it.begin(), it.end() };
		}

		/// <summary>
		/// 
		/// </summary>
		/// <remarks>
		/// This may be replaced by <c>std::views::to</c> in the future.
		/// </remarks>
		/// <returns></returns>
		template <std::ranges::range TContainer> requires 
			(!std::ranges::view<TContainer>)
		auto to() {
			return to_container<TContainer>{};
		}

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

	template <class TInterface>
	class Implement {
	public:
		using interface_type = TInterface;
		using base = Implement<interface_type>;

	protected:
		TInterface* m_parent{ nullptr };

	public:
		Implement(TInterface* parent) : m_parent(parent) {
			if (parent == nullptr)
				throw std::runtime_error("Initializing an implementation requires the parent to be provided.");
		}

		Implement(Implement<TInterface>&&) = delete;
		Implement(const Implement<TInterface>&) = delete;
		virtual ~Implement() = default;
	};

	template <class THandle>
	class IResource {
	public:
		virtual ~IResource() noexcept = default;

	protected:
		virtual THandle& handle() noexcept = 0;

	public:
		virtual const THandle& handle() const noexcept = 0;
	};

	template <class THandle>
	class Resource : public virtual IResource<THandle> {
	private:
		THandle m_handle;

	protected:
		explicit Resource(const THandle handle) noexcept : m_handle(handle) { }

	public:
		Resource(const Resource&) = delete;
		Resource(Resource&&) = delete;
		virtual ~Resource() noexcept = default;

	protected:
		THandle& handle() noexcept override { return m_handle; }

	public:
		const THandle& handle() const noexcept override { return m_handle; }
	};

	template <typename TDerived, typename T, typename TParent = std::nullptr_t, typename TPointer = UniquePtr<T>>
	class Builder;

	template <typename TDerived, typename T, typename TPointer>
	class Builder<TDerived, T, std::nullptr_t, typename TPointer> {
	private:
		TPointer m_instance;

	public:
		using derived_type = TDerived;
		using instance_type = T;
		using parent_type = std::nullptr_t;
		using pointer_type = TPointer;
		using builder_type = Builder<derived_type, instance_type, parent_type, pointer_type>;

	public:
		const T* instance() const noexcept { return m_instance.get(); }

	protected:
		T* instance() noexcept { return m_instance.get(); }

	public:
		explicit Builder(TPointer&& instance) noexcept : m_instance(std::move(instance)) { }
		Builder(const builder_type&) = delete;
		Builder(builder_type&& _other) noexcept : m_instance(std::move(_other.m_instance)) { }
		virtual ~Builder() noexcept = default;

	public:
		// TODO: Provide concept (`is_buildable<TBuilder>`)
		template <typename TInstance>
		void use(pointer_type&&) { static_assert(false, "The current builder does not provide an suitable overload of the `use` method for the type `TInstance`."); }

		//template <rtti::has_builder TInstance, typename ...TArgs, std::enable_if_t<std::is_same_v<typename TInstance::builder::pointer_type, UniquePtr<TInstance>>, int> = 0, typename TBuilder = TInstance::builder> requires 
		//	std::is_convertible_v<TDerived*, typename TBuilder::parent_type*> && rtti::is_explicitly_constructible<TInstance, const T&, TArgs...>
		//TBuilder make(TArgs&&... _args) {
		//	return TBuilder(*static_cast<TDerived*>(this), makeUnique<TInstance>(*m_instance.get(), std::forward<TArgs>(_args)...));
		//}

		//template <rtti::has_builder TInstance, typename ...TArgs, std::enable_if_t<std::is_same_v<typename TInstance::builder::pointer_type, SharedPtr<TInstance>>, int> = 0, typename TBuilder = TInstance::builder> requires 
		//	std::is_convertible_v<TDerived*, typename TBuilder::parent_type*> && rtti::is_explicitly_constructible_v<TInstance, const T&, TArgs...>
		//TBuilder make(TArgs&&... _args) {
		//	return TBuilder(*static_cast<TDerived*>(this), makeShared<TInstance>(*m_instance.get(), std::forward<TArgs>(_args)...));
		//}

		[[nodiscard]]
		virtual TPointer go() {
			return std::move(m_instance);
		}
	};

	template <typename TDerived, typename T, typename TParent, typename TPointer>
	class Builder {
	private:
		TPointer m_instance;
		TParent& m_parent;

	public:
		using derived_type = TDerived;
		using instance_type = T;
		using parent_type = TParent;
		using pointer_type = TPointer;
		using builder_type = Builder<derived_type, instance_type, parent_type, pointer_type>;

	public:
		const T* instance() const noexcept { return m_instance.get(); }
		const TParent& parent() const noexcept { return m_parent; }

	protected:
		T* instance() noexcept { return m_instance.get(); }

	public:
		explicit Builder(TParent& parent, TPointer&& instance) noexcept : m_parent(parent), m_instance(std::move(instance)) { }
		Builder(const builder_type&) = delete;
		Builder(builder_type&& _other) noexcept : m_instance(std::move(_other.m_instance)), m_parent(_other.m_parent) { }
		virtual ~Builder() noexcept = default;

	public:
		// TODO: Provide concept (`is_buildable<TBuilder>`)
		template <typename TInstance>
		void use(pointer_type&&) { static_assert(false, "The current builder does not provide an suitable overload of the `use` method for the type `TInstance`."); }

		//template <rtti::has_builder TInstance, typename ...TArgs, std::enable_if_t<std::is_same_v<typename TInstance::builder::pointer_type, UniquePtr<TInstance>>, int> = 0, typename TBuilder = TInstance::builder> requires 
		//	std::is_convertible_v<TDerived*, typename TBuilder::parent_type*> && rtti::is_explicitly_constructible_v<TInstance, const T&, TArgs...>
		//TBuilder make(TArgs&&... _args) {
		//	return TBuilder(*static_cast<TDerived*>(this), makeUnique<TInstance>(*m_instance.get(), std::forward<TArgs>(_args)...));
		//}

		//template <rtti::has_builder TInstance, typename ...TArgs, std::enable_if_t<std::is_same_v<typename TInstance::builder::pointer_type, SharedPtr<TInstance>>, int> = 0, typename TBuilder = TInstance::builder> requires 
		//	std::is_convertible_v<TDerived*, typename TBuilder::parent_type*> && rtti::is_explicitly_constructible_v<TInstance, const T&, TArgs...>
		//TBuilder make(TArgs&&... _args) {
		//	return TBuilder(*static_cast<TDerived*>(this), makeShared<TInstance>(*m_instance.get(), std::forward<TArgs>(_args)...));
		//}

		[[nodiscard]]
		virtual TParent& go() {
			m_parent.use(std::move(m_instance));
			return m_parent;
		}
	};

#if !defined(LITEFX_BUILDER)
#	define LITEFX_BUILDER(BuilderType) public: \
		/*using builder = BuilderType;*/ \
		friend class BuilderType;
#endif

}
