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
#include <generator>
#include <utility>
#include <iterator>
#include <cstddef>
#include <concepts>
#include <typeindex>

#ifdef __cpp_lib_mdspan
#include <mdspan>
#endif

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
	/// <typeparam name="T">The type of the container elements.</typeparam>
	/// <typeparam name="TVal">The type of the yielded value.</typeparam>
	template <typename T, typename TVal = void>
	using Generator = std::generator<T, TVal>;

	/// <summary>
	/// Evaluates if a type <typeparamref name="TCovariant" /> behaves covariant to a value type <typeparamref name="TValue" />. In this context, covariance is expressed as the type
	/// <typeparamref name="TCovariant" /> can be assigned a value of type <typeparamref name="TValue" /> or can be constructed from a value of <typeparamref name="TValue" />.
	/// </summary>
	/// <typeparam name="TValue">The type of the value.</typeparam>
	/// <typeparam name="TCovariant">The covariant type to check against <typeparamref name="TValue" />.</typeparam>
	template <typename TValue, typename TCovariant>
	concept is_covariant = std::is_assignable_v<TCovariant, TValue> || std::is_constructible_v<TCovariant, TValue>;

	/// <summary>
	/// Evaluates, if an iterator of type <typeparamref name="TIterator" /> iterates values that are covariant to <typeparamref name="TValue" />.
	/// </summary>
	/// <typeparam name="TIterator">The iterator to evaluate.</typeparam>
	/// <typeparam name="TValue">The type that the iterated values should be covariant to.</typeparam>
	template <typename TIterator, typename TValue>
	concept covariant_forward_iterator = std::forward_iterator<TIterator> && is_covariant<decltype(*std::declval<TIterator>()), TValue>;

	/// <summary>
	/// Wraps an iterator and returns covariants of type <typeparamref name="T" /> of the iterated value.
	/// </summary>
	/// <remarks>
	/// This iterator uses type erasure to hide the actual iterated types from the interface. This allows to iterate a range of class instances as a range of base class instances. However,
	/// due to the type erasure, each iteration requires a virtual indirection, resulting in slightly lower performance. Limit the use of this iterator to base class interfaces and return
	/// a reference to the actual underlying range in child classes instead for most performance.
	/// </remarks>
	/// <typeparam name="T">The type returned by the iterator, that is covariant to the actual iterated type.</typeparam>
	/// <seealso cref="Enumerable" />
	template <typename T>
	struct CovariantIterator {
	public:
		/// <summary>
		/// The type of the value that is iterated.
		/// </summary>
		using value_type = std::remove_cvref_t<T>;

		/// <summary>
		/// The category of the iterator.
		/// </summary>
		using iterator_category = std::forward_iterator_tag;

		/// <summary>
		/// The type that expresses the difference between two iterators.
		/// </summary>
		using difference_type = std::ptrdiff_t;

		/// <summary>
		/// The type of a pointer returned by the iterator.
		/// </summary>
		using pointer = std::remove_reference_t<T>*;

	private:
		struct iterator_base {
		protected:
			iterator_base() = default;
			iterator_base(iterator_base&&) noexcept = default;
			iterator_base(const iterator_base&) = default;
			iterator_base& operator=(iterator_base&&) noexcept = default;
			iterator_base& operator=(const iterator_base&) = default;

		public:
			virtual ~iterator_base() noexcept = default;

			virtual T operator*() const = 0;
			virtual iterator_base& operator++() = 0;
			virtual std::unique_ptr<iterator_base> operator++(int) = 0;
			virtual bool operator==(const iterator_base& _other) const noexcept = 0;
			virtual std::unique_ptr<iterator_base> copy() const = 0;
		};

		template <covariant_forward_iterator<T> TIterator>
		struct wrapped_iterator final : public iterator_base {
		private:
			TIterator _it;

		private:
			wrapped_iterator() = delete;
			wrapped_iterator(wrapped_iterator&&) noexcept = default;
			wrapped_iterator(const wrapped_iterator&) = delete;
			wrapped_iterator& operator=(wrapped_iterator&&) noexcept = default;
			wrapped_iterator& operator=(const wrapped_iterator&) = delete;

		public:
			inline wrapped_iterator(TIterator it) :
				_it(std::move(it))
			{ }

			inline ~wrapped_iterator() noexcept override = default;

			inline T operator*() const override {
				return *_it;
			};

			inline iterator_base& operator++() override {
				++_it;
				return *this;
			}

			inline std::unique_ptr<iterator_base> operator++(int) override {
				return std::make_unique<wrapped_iterator>(_it++);
			}

			inline bool operator==(const iterator_base& _other) const noexcept override {
				// NOTE: This is only safe if the other iterator is of the same type as the current iterator, which is enforced by the `CovariantIterator` class.
				return this->_it == static_cast<const wrapped_iterator&>(_other)._it;
			}

			inline std::unique_ptr<iterator_base> copy() const override {
				return std::make_unique<wrapped_iterator>(_it);
			}
		};

		std::unique_ptr<iterator_base> _iterator{ nullptr }; // NOTE: Starting with C++26 there may be a way to express this with a value-semantic unique_ptr.
		std::type_index _iterator_type{ typeid(iterator_base) };

	private:
		CovariantIterator(std::unique_ptr<iterator_base>&& iterator, std::type_index iterator_type) :
			_iterator(std::move(iterator)), _iterator_type(iterator_type) 
		{ }

	public:
		/// <summary>
		/// Initializes a new iterator instance. Always throws a <see cref="RuntimeException" />.
		/// </summary>
		/// <remarks>
		/// This constructor is only defined to satisfy the `std::ranges::range` constraint for ranges that return this iterator. Attempting to default-initialize a `CovariantInterator`
		/// will result in a runtime error.
		/// </remarks>
		explicit CovariantIterator() {
			// Calling this constructor is not supported. It is only publicly available, to make sure the iterator is `std::semiregular`, which is implicitly required by the `std::ranges::range` concept.
			throw RuntimeException("Default-initializing `CovariantIterator` is not supported!");
		}

		/// <summary>
		/// Initializes a new iterator instance.
		/// </summary>
		/// <typeparam name="TIterator">The type of the iterator that returns the value instances.</typeparam>
		/// <param name="it">The iterator to wrap within the iterator instance.</param>
		template <typename TIterator>
		inline CovariantIterator(const TIterator& it) :
			_iterator(std::make_unique<wrapped_iterator<TIterator>>(it)), _iterator_type(typeid(TIterator))
		{ }

		/// <summary>
		/// Copies another iterator instance.
		/// </summary>
		/// <param name="_other">The iterator to copy.</param>
		inline CovariantIterator(const CovariantIterator& _other) :
			_iterator(_other._iterator->copy()), _iterator_type(_other._iterator_type)
		{ }

		/// <summary>
		/// Takes ownership over another iterator instances.
		/// </summary>
		/// <param name="_other">The iterator instance to take over.</param>
		inline CovariantIterator(CovariantIterator&& _other) noexcept = default;

		/// <summary>
		/// Copies another iterator instance.
		/// </summary>
		/// <param name="_other"></param>
		/// <returns></returns>
		inline CovariantIterator& operator=(const CovariantIterator& _other) {
			_iterator = _other._iterator->copy();
			_iterator_type = _other._iterator_type;
			return *this;
		}

		/// <summary>
		/// Takes ownership over another iterator instances.
		/// </summary>
		/// <param name="_other">The iterator instance to take over.</param>
		/// <returns>A reference to the current iterator instance.</returns>
		inline CovariantIterator& operator=(CovariantIterator&& _other) = default;

		/// <summary>
		/// Releases the iterator.
		/// </summary>
		~CovariantIterator() noexcept = default;

		/// <summary>
		/// Returns a reference of the value at the current iterator position.
		/// </summary>
		/// <returns>A reference of the value at the current iterator position.</returns>
		inline T operator*() const {
			return _iterator->operator*();
		}

		/// <summary>
		/// Returns a pointer to the value at the current iterator position.
		/// </summary>
		/// <remarks>
		/// This operator is only available, if the iterated type is a lvalue reference.
		/// </remarks>
		/// <returns>A pointer to the value at the current iterator position.</returns>
		inline pointer operator->() requires std::is_lvalue_reference_v<T> {
			return &this->operator*();
		}

		/// <summary>
		/// Increments the iterator position by one.
		/// </summary>
		/// <returns>A reference of the current iterator.</returns>
		inline CovariantIterator& operator++() {
			_iterator->operator++();
			return *this;
		}

		/// <summary>
		/// Increments the iterator position by one and returns the previous iterator.
		/// </summary>
		/// <returns>A copy of the previous iterator.</returns>
		inline CovariantIterator operator++(int) {
			return { (*_iterator)++, _iterator_type };
		}

		/// <summary>
		/// Checks if two iterators are equal, i.e. they are pointing to the same value.
		/// </summary>
		/// <param name="_other">The iterator to check against.</param>
		/// <returns><c>true</c>, if the iterators are pointing to the same value.</returns>
		inline bool operator==(const CovariantIterator& _other) const {
			if (this->_iterator_type != _other._iterator_type)
				return false;

			return _iterator->operator==(*_other._iterator);
		}
	};

	/// <summary>
	/// An input range over another range, where the returned values of type <typeparamref name="T" /> are covariants of the values stored by the underlying range.
	/// </summary>
	/// <remarks>
	/// An `Enumerable` is intended to be used as a covariant input range for interfaces that want to expose a range of elements that are also interfaces for the stored elements of the actual 
	/// range. In the context of an `Enumerable`, *covariance* refers to the type <typeparamref name="T" /> either being assignable or constructible from the underlying range value type. This
	/// allows not only derived types (in a stricter definition that the C++ language standard uses), but also unrelated types, such as smart pointers to be used in a covariant fashion.
	/// 
	/// In the following example, the interface `IContainer` returns an `Enumerable&lt;IContained&gt;` from a class `Container`, where the contained elements are of type `Contained`. 
	/// 
	/// <example>
	/// class IContained { };
	/// class Contained : public IContained { };
	/// 
	/// class IContainer {
	/// public:
	///		virtual Enumerable<const IContained&> elements() const noexcept = 0;
	/// };
	/// 
	/// class Container : public IContainer {
	/// private:
	///		std::vector<Contained> _elements;
	/// 
	/// public:
	///		Enumerable<const IContained&> elements() const noexcept override {
	///			return _elements;
	///		}
	/// };
	/// </example>
	/// 
	/// As `Contained` is derived from `IContained`, they are covariant in terms of the language. In the context of `Enumerable`, they are covariant, because a `const IContained&amp;` can be 
	/// constructed from a `const Contained&amp;`. This way, using `Enumeable` in `IContainer` allows to iterate the interface instances without knowing their type when declaring the 
	/// interface. The covariance relation only applies to the elements of the range, not the range itself. In the example above, `std::vector&lt;Contained&gt;` is not covariant to 
	/// `Enumerable&lt;const IContained&gt;`, as the two types are not related. This is important as `Enumerable` has a slight performance impact compared to returning a reference of the underlying 
	/// range directly, both in terms of memory (it stores type information about the original iterators) as well as runtime (it requires a virtual call for iterator increments, dereferencing and 
	/// comparison). If you want to expose covariant ranges in interfaces, but allow for maximum performance when the implementation is called directly, you can hide the interface method using 
	/// private inheritance:
	/// 
	/// <example>
	/// class IContainer {
	/// public:
	///		inline Enumerable<const IContained&> elements() const noexcept {
	///			return this->getElements();
	///		}
	/// 
	/// private:
	///		virtual Enumerable<const IContained&> getElements() const noexcept = 0;
	/// };
	/// 
	/// class Container : public IContainer {
	/// private:
	///		std::vector<Contained> _elements;
	/// 
	/// public:
	///		const std::vector<IContained>& elements() const noexcept {
	///			return _elements;
	///		}
	/// 
	/// private:
	///		Enumerable<const IContained&> getElements() const noexcept override {
	///			return _elements;
	///		}
	/// };
	/// </example>
	/// 
	/// `Enumerable` can be using in two ways: first, it can be initialized with a lvalue reference of the underlying range or view as shown above. In this case, only the begin and end iterators 
	/// of the underlying range are stored. However, when passed a rvalue reference, the `Enumerable` will store the underlying range until it and all copies of it are destroyed. This allows to use
	/// `Enumerable` either for returning a temporary range or a view as shown in the example below.
	/// 
	/// <example>
	/// Enumerable<Foo> temporaryFoos() {
	///		std::vector<Foo> foos {};
	///		foos.emplace_back();
	///		foos.emplace_back();
	///		foos.emplace_back();
	///		
	///		return std::move(foos);
	/// 
	///		// Or better:
	///		//return std::vector<Foo>(3);
	/// }
	/// 
	/// Enumerable<Foo&> filteredFoos(const std::vector<Foo>& foos) {
	///		return foos | std::views::drop(1) | std::views::take(2);
	/// }
	/// </example>
	/// 
	/// Keep in mind that the type parameter <typeparamref name="T" /> dictates what an iterator returns from the `Enumerable`, i.e. if an lvalue or (p)rvalue should be returned and wheather or not
	/// a copy is created accordingly.
	/// </remarks>
	/// <typeparam name="T">The type of the values returned by the enumerable.</typeparam>
	/// <seealso cref="CovariantIterator" />
	template <typename T>
	struct Enumerable {
	public:
		/// <summary>
		/// The type of the value that is contained by the `Enumerable`.
		/// </summary>
		using value_type = std::remove_cvref_t<T>;

		/// <summary>
		/// The type of a pointer returned by the `Enumerable`.
		/// </summary>
		using pointer = std::remove_reference_t<T>*;

		/// <summary>
		/// The type of a reference returned by the `Enumerable`.
		/// </summary>
		using reference = std::remove_reference_t<T>&;

		/// <summary>
		/// The type of the iterator used to iterate the elements of the `Enumerable`.
		/// </summary>
		using iterator = CovariantIterator<T>;

		/// <summary>
		/// The type of the iterator used to iterate constant elements of the `Enumerable`.
		/// </summary>
		using const_iterator = CovariantIterator<const std::remove_const_t<T>>;

	private:
		struct range_holder_base {
		protected:
			range_holder_base() = default;
			range_holder_base(range_holder_base&&) noexcept = delete;
			range_holder_base(const range_holder_base&) = delete;
			range_holder_base& operator=(range_holder_base&&) noexcept = delete;
			range_holder_base& operator=(const range_holder_base&) = delete;

		public:
			virtual ~range_holder_base() noexcept = default;

			virtual iterator begin() noexcept = 0;
			virtual iterator end() noexcept = 0;
			virtual const_iterator cbegin() noexcept = 0;
			virtual const_iterator cend() noexcept = 0;
		};

		template <std::ranges::viewable_range TRange>
		struct range_holder final : public range_holder_base {
		private:
			TRange _stored_range;

		private:
			range_holder() = default;
			range_holder(range_holder&&) noexcept = default;
			range_holder(const range_holder&) = delete;
			range_holder& operator=(range_holder&&) noexcept = default;
			range_holder& operator=(const range_holder&) = delete;

		public:
			inline range_holder(TRange&& range) :
				_stored_range(std::move(range))
			{ }

			inline ~range_holder() noexcept override = default;

			inline iterator begin() noexcept override {
				return { std::ranges::begin(_stored_range) };
			}

			inline iterator end() noexcept override {
				return { std::ranges::end(_stored_range) };
			}

			inline const_iterator cbegin() noexcept override {
				return { std::ranges::begin(_stored_range) };
			}

			inline const_iterator cend() noexcept override {
				return { std::ranges::end(_stored_range) };
			}
		};

		std::shared_ptr<range_holder_base> _range{ };

	public:
		/// <summary>
		/// Creates an enumerable over an empty range.
		/// </summary>
		inline Enumerable() :
			Enumerable(std::array<T, 0> { })
		{ }

		inline Enumerable(const Enumerable& range) = default;
		inline Enumerable(Enumerable&& range) noexcept = default;
		inline Enumerable& operator=(const Enumerable& range) = default;
		inline Enumerable& operator=(Enumerable&& range) noexcept = default;
		~Enumerable() noexcept = default;

		/// <summary>
		/// Creates a new `Enumerable` instance from an underlying range.
		/// </summary>
		/// <typeparam name="TRange">The type of the underlying range.</typeparam>
		/// <typeparam name="enabled">Disables the constructor, if <typeparamref name="TRange" /> is equal to the current type, in which case the move constructor should be called.</typeparam>
		/// <param name="range">A reference of the underlying range.</param>
		template <typename TRange, typename enabled = std::enable_if_t<!std::is_same_v<TRange, Enumerable>>>
		inline Enumerable(TRange&& range) {
			// NOTE: Concept evaluation may fail here, if we provide some other enumerable, in which case the evaluated type may be not complete yet, which is why have to
			//       do a static assert here instead of providing the concept in the template.
			static_assert(std::ranges::viewable_range<TRange>, "The source range does not satisfy std::ranges::viewable_range!");
			_range = std::make_shared<range_holder<std::ranges::views::all_t<decltype(range)>>>(std::forward<TRange>(range)); // NOLINT(cppcoreguidelines-prefer-member-initializer)
		}

		/// <summary>
		/// Returns an iterator pointing to the start of the underlying range.
		/// </summary>
		/// <returns>An iterator pointing to the start of the underlying range.</returns>
		inline auto begin() const noexcept {
			return _range->begin();
		}

		/// <summary>
		/// Returns an iterator pointing to the end of the underlying range.
		/// </summary>
		/// <returns>An iterator pointing to the end of the underlying range.</returns>
		inline auto end() const noexcept {
			return _range->end();
		}

		/// <summary>
		/// Returns a constant iterator pointing to the start of the underlying range.
		/// </summary>
		/// <returns>A constant iterator pointing to the start of the underlying range.</returns>
		inline auto cbegin() const noexcept {
			return _range->cbegin();
		}

		/// <summary>
		/// Returns a constant iterator pointing to the end of the underlying range.
		/// </summary>
		/// <returns>A constant iterator pointing to the end of the underlying range.</returns>
		inline auto cend() const noexcept {
			return _range->cend();
		}

		/// <summary>
		/// Returns `true`, if there are no elements inside the `Enumerable` and `false` otherwise.
		/// </summary>
		/// <returns>`true`, if there are no elements inside the `Enumerable` and `false` otherwise.</returns>
		inline bool empty() const noexcept {
			return this->begin() == this->end();
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
		[[nodiscard]] static inline auto create(TArgs&&... args) -> SharedPtr<T> {
			return std::allocate_shared<T>(Allocator<T>{}, std::forward<TArgs>(args)...);
		}

	public:
		/// <summary>
		/// Returns a shared pointer to the current object instance.
		/// </summary>
		template <typename TSelf>
		[[nodiscard]] auto inline shared_from_this(this TSelf&& self) noexcept
		{
			return std::static_pointer_cast<std::remove_reference_t<TSelf>>(
				std::forward<TSelf>(self).std::template enable_shared_from_this<SharedObject>::shared_from_this());
		}
		
		/// <summary>
		/// Returns a weak pointer to the current object instance.
		/// </summary>
		template <typename TSelf>
		[[nodiscard]] auto inline weak_from_this(this TSelf&& self) noexcept -> WeakPtr<std::remove_reference_t<TSelf>>
		{
			return std::static_pointer_cast<std::remove_reference_t<TSelf>>(
				std::forward<TSelf>(self).std::template enable_shared_from_this<SharedObject>::weak_from_this().lock());
		}
	};
}
