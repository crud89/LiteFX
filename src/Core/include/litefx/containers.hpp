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
#include <forward_list>
#include <queue>
#include <tuple>
#include <memory>
#include <functional>
#include <variant>
#include <ranges>
#include <mutex>

#include "generator.hpp"	// NOTE: Should be replaced by #include <generator> once it is available.
#include "traits.hpp"
#include "string.hpp"
#include "exceptions.hpp"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#ifndef LITEFX_DEFINE_FLAGS
#  define LITEFX_DEFINE_FLAGS(T) \
	constexpr T operator| (const T lhs, const T rhs) { using _base_t = std::underlying_type_t<T>; return static_cast<T>(static_cast<_base_t>(lhs) | static_cast<_base_t>(rhs)); } \
	constexpr T& operator|= (T& lhs, const T& rhs) { lhs = lhs | rhs; return lhs; } \
	constexpr T operator& (const T lhs, const T rhs) { using _base_t = std::underlying_type_t<T>; return static_cast<T>(static_cast<_base_t>(lhs) & static_cast<_base_t>(rhs)); } \
	constexpr T& operator&= (T& lhs, const T& rhs) { lhs = lhs & rhs; return lhs; }
#endif

#ifndef LITEFX_FLAG_IS_SET
#  define LITEFX_FLAG_IS_SET(val, flag) static_cast<bool>((std::to_underlying(val) & std::to_underlying(flag)) == std::to_underlying(flag))
#endif

// NOLINTEND(cppcoreguidelines-macro-usage)

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
	using Dictionary = std::unordered_map<TKey, TVal>;

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
	/// A switch that can be used to select a callable from a parameter type.
	/// </summary>
	template<class... TArgs> 
	struct type_switch : TArgs... { 
		using TArgs::operator()...; 
	};

	/// <summary>
	/// Represents a copyable and assignable reference wrapper.
	/// </summary>
	/// <typeparam name="T">The base type of the reference.</typeparam>
	template <class T>
	using Ref = std::reference_wrapper<T>;

	/// <summary>
	/// Creates a new unique pointer.
	/// </summary>
	/// <typeparam name="T">The type of the object, the pointer points to.</typeparam>
	/// <returns>A new unique pointer.</returns>
	template <class T>
	[[nodiscard]] constexpr UniquePtr<T> makeUnique() {
		return std::make_unique<T>();
	}

	/// <summary>
	/// Creates a new unique pointer.
	/// </summary>
	/// <typeparam name="T">The type of the object, the pointer points to.</typeparam>
	/// <returns>A new unique pointer.</returns>
	template <class T, class... TArgs>
	[[nodiscard]] constexpr UniquePtr<T> makeUnique(TArgs&&... _args) {
		return std::make_unique<T>(std::forward<TArgs>(_args)...);
	}

	/// <summary>
	/// Creates a new shared pointer.
	/// </summary>
	/// <typeparam name="T">The type of the object, the pointer points to.</typeparam>
	/// <returns>A new shared pointer.</returns>
	template <class T>
	[[nodiscard]] constexpr SharedPtr<T> makeShared() {
		return std::make_shared<T>();
	}

	/// <summary>
	/// Creates a new shared pointer.
	/// </summary>
	/// <typeparam name="T">The type of the object, the pointer points to.</typeparam>
	/// <returns>A new shared pointer.</returns>
	template <class T, class... TArgs>
	[[nodiscard]] constexpr SharedPtr<T> makeShared(TArgs&&... _args) {
		return std::make_shared<T>(std::forward<TArgs>(_args)...);
	}

	/// <summary>
	/// Transfers a unique pointer to a shared pointer. The unique pointer will be released during this process.
	/// </summary>
	/// <typeparam name="T">The type of the object, the pointer points to.</typeparam>
	/// <param name="ptr">The unique pointer that should be turned into a shared pointer.</param>
	/// <returns>A new shared pointer.</returns>
	template <class T>
	[[nodiscard]] constexpr SharedPtr<T> asShared(UniquePtr<T>&& ptr) {
		SharedPtr<T> shared = std::move(ptr);
		return shared;
	}

	/// <summary>
	/// Describes an intermediate container for elements of type <typeparamref name="T" />.
	/// </summary>
	/// <remarks>
	/// An `Enumerable` is a generic runtime-polymorphic container designed for class interfaces. Differently to STL containers, it does not impose constraints other than the 
	/// requirement of being forward-iterable. It's purpose is to pass immutable containers between objects. Since it is compatible to the STL *ranges* library, it can be 
	/// constructed from arbitrary containers.
	/// 
	/// Note that `Enumerable` *owns* it's elements, which means that a copy *might* occur, if the input range does not contain rvalue references. You might want to wrap the
	/// element type in a <see cref="Ref" />, if you want to provide access to source elements. If you want to transfer ownership of the elements to the `Enumerable`, you can 
	/// use `std::views::as_rvalue` on the source range or view. This is required for non-copyable (move-only) types, such as <see cref="UniquePtr" />. Note, however, that this 
	/// might leave the original container in an undefined state.
	/// </remarks>
	/// <typeparam name="T">The type of the container elements.</typeparam>
	template <typename T>
	class Enumerable {
	public:
		using value_type = T;
		using array_type = std::forward_list<T>;
		using allocator_type = array_type::allocator_type;
		using size_type = array_type::size_type;
		using difference_type = array_type::difference_type;
		using pointer = array_type::pointer;
		using const_pointer = array_type::const_pointer;
		using iterator = array_type::iterator;
		using const_iterator = array_type::const_iterator;
		using reference = value_type&;
		using const_reference = const value_type&;

	private:
		array_type m_elements;
		size_type m_size = 0;

	public:
		/// <summary>
		/// Creates a new `Enumerable` from an arbitrary input range or view.
		/// </summary>
		/// <param name="input">The input range or view that contains the elements the `Enumerable` is initialized with.</param>
		constexpr Enumerable(std::ranges::input_range auto&& input) requires
			std::convertible_to<std::ranges::range_value_t<decltype(input)>, T> :
			m_size(0)
		{
			auto it = m_elements.before_begin();

			for (auto elem : input)
			{
				m_elements.insert_after(it, std::forward<std::ranges::range_value_t<decltype(input)>>(elem));
				it++;
				m_size++;
			}
		}

		/// <summary>
		/// Creates a new `Enumerable` from an initializer list.
		/// </summary>
		/// <param name="input">The initializer list that contains the elements, the `Enumerable` is initialized with.</param>
		constexpr Enumerable(std::initializer_list<T> input) :
			m_elements{ input }, m_size{ input.size() }
		{
		}

		/// <summary>
		/// Creates a new `Enumerable` from a set of arguments.
		/// </summary>
		/// <typeparam name="...TArgs">The types of the arguments.</typeparam>
		/// <param name="...args">The arguments.</param>
		template <typename... TArgs> requires meta::are_same<T, TArgs...>
		constexpr explicit Enumerable(TArgs&&... args) noexcept
		{
			auto input = std::to_array({ std::forward<TArgs>(args)... });
			m_size = input.size();
			m_elements = { std::make_move_iterator(std::begin(input)), std::make_move_iterator(std::end(input)) };
		}

		/// <summary>
		/// Creates a new `Enumerable` from a forward list.
		/// </summary>
		/// <remarks>
		/// Initializing the enumerable by moving a forward list may be more efficient than the other constructors.
		/// </remarks>
		/// <param name="input">The forward list that contains the elements, the `Enumerable` is initialized with.</param>
		constexpr Enumerable(array_type&& input) noexcept :
			m_elements(std::move(input)), m_size(0)
		{
			for (auto& elem : m_elements)
				m_size++;
		}

		/// <summary>
		/// Creates an empty `Enumerable`.
		/// </summary>
		constexpr Enumerable() requires std::default_initializable<array_type> = default;

		/// <summary>
		/// Initializes the `Enumerable` by taking over <paramref name="_other" />.
		/// </summary>
		/// <remarks>
		/// Note that this constructor can only be used of <typeparamref name="T" /> is movable.
		/// </remarks>
		/// <param name="_other">The `Enumerable` to take over.</param>
		constexpr Enumerable(Enumerable<T>&& _other) noexcept = default;

		/// <summary>
		/// Initializes the `Enumerable` by taking over <paramref name="_other" />.
		/// </summary>
		/// <remarks>
		/// Note that this constructor can only be used of <typeparamref name="T" /> is movable.
		/// </remarks>
		/// <param name="_other">The `Enumerable` to take over.</param>
		/// <returns>A reference of the `Enumerable` after the move.</returns>
		constexpr Enumerable<T>& operator=(Enumerable<T>&& _other) noexcept = default;

		/// <summary>
		/// Initializes the `Enumerable` by copying <paramref name="_other" />.
		/// </summary>
		/// <remarks>
		/// Note that this constructor can only be used of <typeparamref name="T" /> is copyable.
		/// </remarks>
		/// <param name="_other">The `Enumerable` to copy.</param>
		constexpr Enumerable(const Enumerable<T>& _other) = default;

		/// <summary>
		/// Initializes the `Enumerable` by copying <paramref name="_other" />.
		/// </summary>
		/// <remarks>
		/// Note that this constructor can only be used of <typeparamref name="T" /> is copyable.
		/// </remarks>
		/// <param name="_other">The `Enumerable` to copy.</param>
		/// <returns>A reference of the `Enumerable` after the copy.</returns>
		constexpr Enumerable<T>& operator=(const Enumerable<T>& _other) = default;

		constexpr ~Enumerable() noexcept = default;

	public:
		/// <summary>
		/// Returns the number of elements of the `Enumerable`.
		/// </summary>
		/// <returns>The number of elements of the `Enumerable`.</returns>
		constexpr size_type size() const noexcept {
			return m_size;
		}

		/// <summary>
		/// Returns `true`, if the `Enumerable` is empty and `false` otherwise.
		/// </summary>
		/// <returns>`true`, if the `Enumerable` is empty and `false` otherwise.</returns>
		constexpr bool empty() const noexcept {
			return m_elements.empty();
		}

		/// <summary>
		/// Returns the iterator that points to the beginning of the `Enumerable`.
		/// </summary>
		/// <returns>The iterator that points to the beginning of the `Enumerable`.</returns>
		constexpr iterator begin() noexcept {
			return m_elements.begin();
		}

		/// <summary>
		/// Returns the iterator that points to the beginning of the `Enumerable`.
		/// </summary>
		/// <returns>The iterator that points to the beginning of the `Enumerable`.</returns>
		constexpr const_iterator begin() const noexcept {
			return m_elements.begin();
		}

		/// <summary>
		/// Returns the iterator that points to the beginning of the `Enumerable`.
		/// </summary>
		/// <returns>The iterator that points to the beginning of the `Enumerable`.</returns>
		constexpr const_iterator cbegin() noexcept {
			return m_elements.cbegin();
		}

		/// <summary>
		/// Returns the iterator that points to the beginning of the `Enumerable`.
		/// </summary>
		/// <returns>The iterator that points to the beginning of the `Enumerable`.</returns>
		constexpr const_iterator cbegin() const noexcept {
			return m_elements.cbegin();
		}

		/// <summary>
		/// Returns the iterator that points to the ending of the `Enumerable`.
		/// </summary>
		/// <returns>The iterator that points to the ending of the `Enumerable`.</returns>
		constexpr iterator end() noexcept {
			return m_elements.end();
		}

		/// <summary>
		/// Returns the iterator that points to the ending of the `Enumerable`.
		/// </summary>
		/// <returns>The iterator that points to the ending of the `Enumerable`.</returns>
		constexpr const_iterator end() const noexcept {
			return m_elements.end();
		}

		/// <summary>
		/// Returns the iterator that points to the ending of the `Enumerable`.
		/// </summary>
		/// <returns>The iterator that points to the ending of the `Enumerable`.</returns>
		constexpr const_iterator cend() noexcept {
			return m_elements.cend();
		}

		/// <summary>
		/// Returns the iterator that points to the ending of the `Enumerable`.
		/// </summary>
		/// <returns>The iterator that points to the ending of the `Enumerable`.</returns>
		constexpr const_iterator cend() const noexcept {
			return m_elements.cend();
		}

		/// <summary>
		/// Returns the first element of the `Enumerable`, if it is not empty.
		/// </summary>
		/// <returns>The first element of the `Enumerable`, if it is not empty.</returns>
		constexpr decltype(auto) front() {
			return m_elements.front();
		}

		/// <summary>
		/// Returns the first element of the `Enumerable`, if it is not empty.
		/// </summary>
		/// <returns>The first element of the `Enumerable`, if it is not empty.</returns>
		constexpr decltype(auto) front() const {
			return m_elements.front();
		}
	};

#if (defined(BUILD_LITEFX_PIMPL) && BUILD_LITEFX_PIMPL) || (!defined(BUILD_LITEFX_PIMPL)) && !defined(LITEFX_IMPLEMENTATION)
	/// <summary>
	/// A smart pointer that manages an implementation instance for a public interface class.
	/// </summary>
	/// <typeparam name="pImpl">The type of the implementation class.</typeparam>
	template <class pImpl>
	class PimplPtr final {
	private:
		/// <summary>
		/// Stores the shared pointer to the implementation.
		/// </summary>
		SharedPtr<pImpl> m_ptr;

	public:
		/// <summary>
		/// Initializes a new pointer to an implementation instance.
		/// </summary>
		constexpr PimplPtr() /*requires std::is_default_constructible_v<pImpl>*/ :
			m_ptr(makeShared<pImpl>()) { }

		/// <summary>
		/// Initializes a new pointer of an implementation.
		/// </summary>
		/// <typeparam name="...TArgs">The types of the arguments passed to the implementation constructor.</typeparam>
		/// <param name="...args">The arguments passed to the implementation constructor.</param>
		template <typename... TArgs>
		constexpr PimplPtr(TArgs&&... args) /*requires std::constructible_from<pImpl, TArgs...>*/ :
			m_ptr(makeShared<pImpl>(std::forward<TArgs>(args)...)) { } // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)

		/// <summary>
		/// Initializes a new pointer to a copy of the implementation instance managed by <paramref name="src" />.
		/// </summary>
		/// <param name="src">The source pointer to copy the implementation instance from.</param>
		constexpr PimplPtr(const PimplPtr& src) /*requires std::copy_constructible<pImpl>*/ : 
			m_ptr(makeShared<pImpl>(*src.m_ptr)) { }

		/// <summary>
		/// Initializes a new pointer by taking over the implementation instance managed by <paramref name="src" />.
		/// </summary>
		/// <param name="src">The source pointer to take over.</param>
		constexpr PimplPtr(PimplPtr&& src) noexcept = default;

		/// <summary>
		/// Initializes a new pointer to a copy of the implementation instance managed by <paramref name="src" />.
		/// </summary>
		/// <remarks>
		/// Note that this will share ownership between this instance and <paramref name="src" />. Only use this method, if you <see cref="release" /> either
		/// of both implementation pointers manually!
		/// </remarks>
		/// <param name="src">The source pointer to copy the implementation instance from.</param>
		/// <returns>A new pointer to the provided implementation instance.</returns>
		constexpr PimplPtr& operator=(const PimplPtr& src) /*requires std::copy_constructible<pImpl>*/
		{
			if (&src != this)
				m_ptr = makeShared<pImpl>(*src.m_ptr);

			return *this; 
		}

		/// <summary>
		/// Initializes a new pointer by taking over the implementation instance managed by <paramref name="src" />.
		/// </summary>
		/// <param name="src">The source pointer to take over.</param>
		/// <returns>A new pointer to the provided implementation instance.</returns>
		constexpr PimplPtr& operator=(PimplPtr&& src) noexcept = default;

		constexpr ~PimplPtr() noexcept = default;

	public:
		/// <summary>
		/// Returns a reference to the managed implementation instance.
		/// </summary>
		/// <returns>A reference to the managed implementation instance.</returns>
		constexpr pImpl& operator* () const noexcept { 
			return *m_ptr; 
		}

		/// <summary>
		/// Returns a pointer to the managed implementation instance.
		/// </summary>
		/// <returns>A pointer to the managed implementation instance.</returns>
		constexpr pImpl* operator-> () const noexcept { 
			return m_ptr.get(); 
		}
	};

	// NOLINTBEGIN(cppcoreguidelines-macro-usage)

	/// <summary>
	/// Declares the implementation for the public interface of a class.
	/// </summary>
	/// <remarks>
	/// A class can access the instance of the implementation instance using the pointer `m_impl` after declaring the implementation
	/// using this macro.
	/// </remarks>
	/// <seealso cref="Implement" />
#  define LITEFX_IMPLEMENTATION(impl) private: \
	class impl; \
	PimplPtr<impl> m_impl; \
	friend class PimplPtr<impl>; \
	friend class impl;

	// NOLINTEND(cppcoreguidelines-macro-usage)
#endif

	/// <summary>
	/// Provides access to a resource managed by the class.
	/// </summary>
	/// <seealso cref="Resource" />
	/// <typeparam name="THandle">The type of the resource.</typeparam>
	template <class THandle>
	class IResource {
	protected:
		IResource() noexcept = default;
		IResource(const IResource&) = delete;
		IResource(IResource&&) noexcept = default;
		IResource& operator=(const IResource&) = delete;
		IResource& operator=(IResource&&) noexcept = default;

	public:
		virtual ~IResource() noexcept = default;

	protected:
		/// <summary>
		/// Returns the resource managed by the class.
		/// </summary>
		/// <returns>The resource managed by the class.</returns>
		virtual THandle& handle() noexcept = 0;

	public:
		/// <summary>
		/// Returns the resource managed by the class.
		/// </summary>
		/// <returns>The resource managed by the class.</returns>
		virtual const THandle& handle() const noexcept = 0;
	};

	/// <summary>
	/// Implements the <see cref="IResource" /> interface.
	/// </summary>
	/// <typeparam name="THandle">The type of the resource.</typeparam>
	template <class THandle>
	class Resource : public virtual IResource<THandle> {
	public:
        using handle_type = THandle;

	private:
		THandle m_handle;

	protected:
		/// <summary>
		/// Initializes the managed resource.
		/// </summary>
		/// <param name="handle">The managed resource handle.</param>
		explicit Resource(THandle handle) noexcept : m_handle(std::move(handle)) { }
		Resource(const Resource&) = delete;
		Resource(Resource&&) noexcept = default;
		Resource& operator=(const Resource&) = delete;
		Resource& operator=(Resource&&) noexcept = default;

	public:
		~Resource() noexcept override = default;

	protected:
		/// <inheritdoc />
		THandle& handle() noexcept override { return m_handle; }

	public:
		/// <inheritdoc />
		const THandle& handle() const noexcept override { return m_handle; }
	};

	/// <summary>
	/// Describes an generic builder type.
	/// </summary>
	/// <remarks>
	/// Builders are helper classes to create objects or object hierarchies. A builder is called *root builder*, if <typeparamref name="TParent" /> is
	/// set to `std::nullptr_t`, otherwise it is called *child builder* and <typeparamref name="TParent" /> is set to the type of the parent builder in
	/// the hierarchy. A builder can be a child builder to either a root builder or another child builder. However, at the top of the hierarchy there
	/// needs to be one root builder and each hierarchy can only have one root builder.
	/// 
	/// Building a child object using a child builder can be terminated by calling `add` on it. This will call `use` on the parent builder and pass the
	/// object instance to it. If any work needs to be done by the builder before passing it to the parent builder, it is possible to overwrite the 
	/// `build` method. For root builders no additional call is required. Instead a root builder provides a move-assignment operator for 
	/// <typeparamref name="T" />. Assigning the builder instance to an instance of <typeparamref name="TPointer" /> will return the instance object.
	/// Similar to child builders, it is possible to overwrite the `build` method, to perform any additional pre-construction work.
	/// 
	/// Builders create the object instances they manage in form of smart pointers. The <typeparamref name="TPointer" /> can either be set to any smart
	/// pointer type that wraps <typeparamref name="T" /> for convenience.
	/// </remarks>
	/// <typeparam name="T">The type of the object the builder builds.</typeparam>
	/// <typeparam name="TParent">The type of the parent builder or `std::nullptr_t`.</typeparam>
	/// <typeparam name="TPointer">The type of the pointer, used to access the instance of <typeparamref name="T" /> this builder builds.</typeparam>
	template <typename T, typename TParent = std::nullptr_t, typename TPointer = UniquePtr<T>>
	class Builder;

	/// <summary>
	/// Describes a root builder.
	/// </summary>
	/// <typeparam name="T">The type of the object the builder builds.</typeparam>
	/// <typeparam name="TPointer">The type of the pointer, used to access the instance of <typeparamref name="T" /> this builder builds.</typeparam>
	/// <seealso href="https://github.com/crud89/LiteFX/wiki/Builders" />
	template <typename T, typename TPointer>
	class Builder<T, std::nullptr_t, TPointer> {
	private:
		TPointer m_instance;

	public:
		using instance_type = T;
		using parent_type = std::nullptr_t;
		using pointer_type = TPointer;

	public:
		/// <summary>
		/// Returns a pointer to the current instance of the object that is built by the builder.
		/// </summary>
		/// <returns>A pointer to the current object instance.</returns>
		constexpr const T* instance() const noexcept { return m_instance.get(); }

	protected:
		/// <summary>
		/// Returns a pointer to the current instance of the object that is built by the builder.
		/// </summary>
		/// <returns>A pointer to the current object instance.</returns>
		constexpr T* instance() noexcept { return m_instance.get(); }

	public:
		/// <summary>
		/// Initializes the builder instance.
		/// </summary>
		/// <param name="instance">The instance of the object to build.</param>
		constexpr explicit Builder(TPointer&& instance) noexcept : m_instance(std::move(instance)) { }

		/// <summary>
		/// Initializes the builder instance by taking over another instance.
		/// </summary>
		/// <param name="_other">The instance of another builder object to take over.</param>
		constexpr Builder(Builder&& _other) noexcept : m_instance(std::move(_other.m_instance)) { }

		Builder(const Builder&) = delete;
		auto operator=(const Builder&) = delete;
		auto operator=(const Builder&&) noexcept = delete;

		constexpr virtual ~Builder() noexcept = default;

	protected:
		/// <summary>
		/// Can be overwritten to perform any pre-construction work before the builder returns the final object instance.
		/// </summary>
		constexpr virtual void build() { };

	public:
		// TODO: Provide concept (`is_buildable<TBuilder>`)
		/// <summary>
		/// Called by child builders to pass a constructed object back to the parent builder.
		/// </summary>
		/// <remarks>
		/// This method must be implemented for each child builder, a builder can create. This introduces a hard dependency between child and parent 
		/// builders. It is not possible to define a child builder without implementing the counter part in the parent builder.
		/// </remarks>
		template <typename TInstance>
		void use(pointer_type&&) noexcept = delete;

		/// <summary>
		/// Calls <see cref="build" /> and returns the instance.
		/// </summary>
		[[nodiscard]] constexpr operator TPointer&& () {
			this->build();
			return std::move(m_instance);
		}
	};

	/// <summary>
	/// Describes a child builder.
	/// </summary>
	/// <typeparam name="T">The type of the object the builder builds.</typeparam>
	/// <typeparam name="TPointer">The type of the pointer, used to access the instance of <typeparamref name="T" /> this builder builds.</typeparam>
	/// <seealso href="https://github.com/crud89/LiteFX/wiki/Builders" />
	template <typename T, typename TParent, typename TPointer>
	class Builder {
	private:
		TPointer m_instance;
		TParent* m_parent;

	public:
		using instance_type = T;
		using parent_type = TParent;
		using pointer_type = TPointer;

	public:
		/// <summary>
		/// Returns a pointer to the current instance of the object that is built by the builder.
		/// </summary>
		/// <returns>A pointer to the current object instance.</returns>
		constexpr const T* instance() const noexcept { return m_instance.get(); }

		/// <summary>
		/// Returns a reference of the parent builder.
		/// </summary>
		/// <returns>A reference of the parent builder.</returns>
		constexpr const TParent& parent() const noexcept { return *m_parent; }

	protected:
		/// <summary>
		/// Returns a pointer to the current instance of the object that is built by the builder.
		/// </summary>
		/// <returns>A pointer to the current object instance.</returns>
		constexpr T* instance() noexcept { return m_instance.get(); }

	public:
		/// <summary>
		/// Initializes the builder instance.
		/// </summary>
		/// <param name="parent">The instance of the parent builder.</param>
		/// <param name="instance">The instance of the object to build.</param>
		constexpr explicit Builder(TParent& parent, TPointer&& instance) noexcept : m_instance(std::move(instance)), m_parent(&parent) { }
		
		/// <summary>
		/// Initializes the builder instance by taking over another instance.
		/// </summary>
		/// <param name="_other">The instance of another builder object to take over.</param>
		constexpr Builder(Builder&& _other) noexcept : m_instance(std::move(_other.m_instance)), m_parent(_other.m_parent) { }

		constexpr Builder(const Builder&) = delete;
		auto operator=(const Builder&) = delete;
		auto operator=(Builder&&) noexcept = delete;
		constexpr virtual ~Builder() noexcept = default;

	protected:
		/// <summary>
		/// Can be overwritten to perform any pre-construction work before the builder returns the final object instance.
		/// </summary>
		constexpr virtual void build() { };

	public:
		// TODO: Provide concept (`is_buildable<TBuilder>`)
		/// <summary>
		/// Called by child builders to pass a constructed object back to the parent builder.
		/// </summary>
		/// <remarks>
		/// This method must be implemented for each child builder, a builder can create. This introduces a hard dependency between child and parent 
		/// builders. It is not possible to define a child builder without implementing the counter part in the parent builder.
		/// </remarks>
		template <typename TInstance>
		void use(pointer_type&&) noexcept = delete;

		/// <summary>
		/// First, calls <see cref="build" />, then `use` on the parent builder using the current object instance and finally returns the parent builder.
		/// </summary>
		[[nodiscard]] constexpr TParent& add() {
			this->build();
			m_parent->use(std::move(m_instance));
			return *m_parent;
		}
	};

	// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#if !defined(LITEFX_BUILDER)
#    define LITEFX_BUILDER(BuilderType) public: \
		using builder_type = BuilderType; \
		friend class BuilderType;
#endif // !defined(LITEFX_BUILDER)

	// NOLINTEND(cppcoreguidelines-macro-usage)

	/// <summary>
	/// Base class for an object that can be shared.
	/// </summary>
	/// <remarks>
	/// This is an improved version of `std::enable_shared_from_this` that supports inheritance. When inheriting from this class, follow the same practices as you would for `std::enable_shared_from_this`: do 
	/// not provide any public constructors; instead provide a private constructor and a publicly accessible static factory method, that returns a shared pointer.
	/// 
	/// Note that the above rule does not apply for objects that are stored within a <see cref="PimplPtr" />, as those are handled correctly by the pointer implementation.
	/// 
	/// You may want to create objects by creating a static factory method that calls the protected <see cref="SharedObject::create" /> method. This has the advantage of allocating a single memory block for 
	/// both, the object and the shared pointers control block. To do this, make sure to declare friendship to <see cref="SharedAllocator" /> in your class, as shown in the example below.
	/// </remarks>
	/// <example>
	/// <code>
	/// class Foo : public SharedObject {
	///     friend struct SharedObject::Allocator&gt;Foo&lt;;
	/// 
	/// private:
	///     explicit Foo(int a, std::string b) { }
	/// 
	/// public:
	///     static inline auto create(int a, std::string b) {
	///         return SharedObject::create&gt;Foo&lt;(a, b);
	///     }
	/// }
	/// </code>
	/// </example>
	/// <seealso href="https://en.cppreference.com/w/cpp/memory/enable_shared_from_this" />
	class SharedObject : public std::enable_shared_from_this<SharedObject> {
	protected:
		/// <summary>
		/// Initializes a new shared object.
		/// </summary>
		SharedObject() noexcept = default;
		SharedObject(SharedObject&&) noexcept = default;
		SharedObject(const SharedObject&) = default;
		SharedObject& operator=(SharedObject&&) noexcept = default;
		SharedObject& operator=(const SharedObject&) = default;

	public:
		/// <summary>
		/// Destroys the shared object.
		/// </summary>
		virtual ~SharedObject() noexcept = default;

	protected:
		/// <summary>
		/// An allocator used to allocate the shared object.
		/// </summary>
		/// <typeparam name="T">The type of the class that inherits from <see cref="SharedObject" />.</typeparam>
		template <typename T>
		struct Allocator : public std::allocator<T> {
			template<typename TParent, typename... TArgs>
			void construct(TParent* parent, TArgs&&... args) {
				::new(static_cast<void*>(parent)) TParent(std::forward<TArgs>(args)...);
			}
		};

		/// <summary>
		/// Generic factory method used to create instances of the shared object.
		/// </summary>
		/// <typeparam name="T">The type of the class that inherits from <see cref="SharedObject" />.</typeparam>
		/// <typeparam name="TArgs">The types of the arguments passed to the shared object's constructor.</typeparam>
		/// <param name="args">The arguments that are forwarded to the shared object's constructor.</param>
		/// <returns>A shared pointer of the shared object.</returns>
		/// <seealso cref="Allocator" />
		template <typename T, typename... TArgs>
		static inline auto create(TArgs&&... args) -> SharedPtr<T> {
			return std::allocate_shared<T>(Allocator<T>{}, std::forward<TArgs>(args)...);
		}

	public:
		/// <summary>
		/// Returns a shared pointer to the current object instance.
		/// </summary>
		template <typename TSelf>
		auto inline shared_from_this(this TSelf&& self) noexcept
		{
			return std::static_pointer_cast<std::remove_reference_t<TSelf>>(
				std::forward<TSelf>(self).std::template enable_shared_from_this<SharedObject>::shared_from_this());
		}
		
		/// <summary>
		/// Returns a weak pointer to the current object instance.
		/// </summary>
		template <typename TSelf>
		auto inline weak_from_this(this TSelf&& self) noexcept -> WeakPtr<std::remove_reference_t<TSelf>>
		{
			return std::static_pointer_cast<std::remove_reference_t<TSelf>>(
				std::forward<TSelf>(self).std::template enable_shared_from_this<SharedObject>::weak_from_this().lock());
		}
	};
}
