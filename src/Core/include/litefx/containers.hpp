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

	/// @brief Represents a handle type.
	using Handle = void*;

	/// @brief Represents a dictionary that maps a key to a certain value.
	///
	/// @tparam TKey The type of the key.
	/// @tparam TVal The type of the value.
	template<class TKey, class TVal>
	using Dictionary = std::unordered_map<TKey, TVal>;

	/// @brief Represents a dynamic array.
	///
	/// @tparam T The type of the array elements.
	template<class T>
	using Array = std::vector<T>;

	/// @brief Represents a queue.
	///
	/// @tparam T The type of the queue elements.
	template<class T>
	using Queue = std::queue<T>;

	/// @brief Represents a view of an array.
	///
	/// @tparam T The type of the array elements.
	template<class T>
	using Span = std::span<T>;

	/// @brief Represents an optional value.
	///
	/// @tparam T The type of the optional value.
	template<class T>
	using Optional = std::optional<T>;

	/// @brief Represents a unique pointer, that expresses exclusive ownership.
	///
	/// @tparam T The type of the object the pointer points to.
	/// @tparam TDeleter The deleter for the pointed object.
	template<class T, class TDeleter = std::default_delete<T>>
	using UniquePtr = std::unique_ptr<T, TDeleter>;

	/// @brief Represents a shared pointer, that expresses non-exclusive ownership.
	///
	/// @tparam T The type of the object the pointer points to.
	template <class T>
	using SharedPtr = std::shared_ptr<T>;

	/// @brief Represents a weak pointer, that expresses a reference to a shared pointer instance.
	///
	/// @tparam T The type of the object the pointer points to.
	template <class T>
	using WeakPtr = std::weak_ptr<T>;

	/// @brief Represents a tuple of multiple objects.
	///
	/// @tparam ...T The types of the objects, contained by the tuple.
	template <class... T>
	using Tuple = std::tuple<T...>;

	/// @brief Represents a variant of objects.
	///
	/// @tparam ...T The types of the objects, that can be contained by the tuple.
	template <class... T>
	using Variant = std::variant<T...>;

	/// @brief A switch that can be used to select a callable from a parameter type.
	template<class... TArgs> 
	struct type_switch : TArgs... { 
		using TArgs::operator()...; 
	};

	/// @brief Represents a copyable and assignable reference wrapper.
	///
	/// @tparam T The base type of the reference.
	template <class T>
	using Ref = std::reference_wrapper<T>;

	/// @brief Creates a new unique pointer.
	///
	/// @tparam T The type of the object, the pointer points to.
	/// @return A new unique pointer.
	template <class T>
	[[nodiscard]] constexpr UniquePtr<T> makeUnique() {
		return std::make_unique<T>();
	}

	/// @brief Creates a new unique pointer.
	///
	/// @tparam T The type of the object, the pointer points to.
	/// @return A new unique pointer.
	template <class T, class... TArgs>
	[[nodiscard]] constexpr UniquePtr<T> makeUnique(TArgs&&... _args) {
		return std::make_unique<T>(std::forward<TArgs>(_args)...);
	}

	/// @brief Creates a new shared pointer.
	///
	/// @tparam T The type of the object, the pointer points to.
	/// @return A new shared pointer.
	template <class T>
	[[nodiscard]] constexpr SharedPtr<T> makeShared() {
		return std::make_shared<T>();
	}

	/// @brief Creates a new shared pointer.
	///
	/// @tparam T The type of the object, the pointer points to.
	/// @return A new shared pointer.
	template <class T, class... TArgs>
	[[nodiscard]] constexpr SharedPtr<T> makeShared(TArgs&&... _args) {
		return std::make_shared<T>(std::forward<TArgs>(_args)...);
	}

	/// @brief Transfers a unique pointer to a shared pointer. The unique pointer will be released during this process.
	///
	/// @tparam T The type of the object, the pointer points to.
	/// @param ptr The unique pointer that should be turned into a shared pointer.
	/// @return A new shared pointer.
	template <class T>
	[[nodiscard]] constexpr SharedPtr<T> asShared(UniquePtr<T>&& ptr) {
		SharedPtr<T> shared = std::move(ptr);
		return shared;
	}

	/// @brief Describes an intermediate container for elements of type @p T.
	///
	/// @tparam T The type of the container elements.
	/// @tparam TVal The type of the yielded value.
	template <typename T, typename TVal = void>
	using Generator = std::generator<T, TVal>;

	/// @brief Evaluates if a type @p TCovariant behaves covariant to a value type @p TValue. In this context, covariance is expressed as the type @p TCovariant can be assigned a value of type @p TValue or
	/// can be constructed from a value of @p TValue.
	///
	/// @tparam TValue The type of the value.
	/// @tparam TCovariant The covariant type to check against @p TValue.
	template <typename TValue, typename TCovariant>
	concept is_covariant = std::is_assignable_v<TCovariant, TValue> || std::is_constructible_v<TCovariant, TValue>;

	/// @brief Evaluates, if an iterator of type @p TIterator iterates values that are covariant to @p TValue.
	///
	/// @tparam TIterator The iterator to evaluate.
	/// @tparam TValue The type that the iterated values should be covariant to.
	template <typename TIterator, typename TValue>
	concept covariant_forward_iterator = std::forward_iterator<TIterator> && is_covariant<decltype(*std::declval<TIterator>()), TValue>;

	/// @brief Wraps an iterator and returns covariants of type @p T of the iterated value.
	///
	/// This iterator uses type erasure to hide the actual iterated types from the interface. This allows to iterate a range of class instances as a range of base class instances. However, due to the type
	/// erasure, each iteration requires a virtual indirection, resulting in slightly lower performance. Limit the use of this iterator to base class interfaces and return a reference to the actual underlying
	/// range in child classes instead for most performance.
	///
	/// @tparam T The type returned by the iterator, that is covariant to the actual iterated type.
	/// @see Enumerable
	template <typename T>
	struct CovariantIterator {
	public:
		/// @brief The type of the value that is iterated.
		using value_type = std::remove_cvref_t<T>;

		/// @brief The category of the iterator.
		using iterator_category = std::forward_iterator_tag;

		/// @brief The type that expresses the difference between two iterators.
		using difference_type = std::ptrdiff_t;

		/// @brief The type of a pointer returned by the iterator.
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
		/// @brief Initializes a new iterator instance. Always throws a @ref RuntimeException.
		///
		/// This constructor is only defined to satisfy the `std::ranges::range` constraint for ranges that return this iterator. Attempting to default-initialize a `CovariantInterator` will result in a runtime
		/// error.
		explicit CovariantIterator() {
			// Calling this constructor is not supported. It is only publicly available, to make sure the iterator is `std::semiregular`, which is implicitly required by the `std::ranges::range` concept.
			throw RuntimeException("Default-initializing `CovariantIterator` is not supported!");
		}

		/// @brief Initializes a new iterator instance.
		///
		/// @tparam TIterator The type of the iterator that returns the value instances.
		/// @param it The iterator to wrap within the iterator instance.
		template <typename TIterator>
		inline CovariantIterator(const TIterator& it) :
			_iterator(std::make_unique<wrapped_iterator<TIterator>>(it)), _iterator_type(typeid(TIterator))
		{ }

		/// @brief Copies another iterator instance.
		///
		/// @param _other The iterator to copy.
		inline CovariantIterator(const CovariantIterator& _other) :
			_iterator(_other._iterator->copy()), _iterator_type(_other._iterator_type)
		{ }

		/// @brief Takes ownership over another iterator instances.
		///
		/// @param _other The iterator instance to take over.
		inline CovariantIterator(CovariantIterator&& _other) noexcept = default;

		/// @brief Copies another iterator instance.
		///
		/// @param _other
		/// @return
		inline CovariantIterator& operator=(const CovariantIterator& _other) {
			_iterator = _other._iterator->copy();
			_iterator_type = _other._iterator_type;
			return *this;
		}

		/// @brief Takes ownership over another iterator instances.
		///
		/// @param _other The iterator instance to take over.
		/// @return A reference to the current iterator instance.
		inline CovariantIterator& operator=(CovariantIterator&& _other) = default;

		/// @brief Releases the iterator.
		~CovariantIterator() noexcept = default;

		/// @brief Returns a reference of the value at the current iterator position.
		///
		/// @return A reference of the value at the current iterator position.
		inline T operator*() const {
			return _iterator->operator*();
		}

		/// @brief Returns a pointer to the value at the current iterator position.
		///
		/// This operator is only available, if the iterated type is a lvalue reference.
		///
		/// @return A pointer to the value at the current iterator position.
		inline pointer operator->() requires std::is_lvalue_reference_v<T> {
			return &this->operator*();
		}

		/// @brief Increments the iterator position by one.
		///
		/// @return A reference of the current iterator.
		inline CovariantIterator& operator++() {
			_iterator->operator++();
			return *this;
		}

		/// @brief Increments the iterator position by one and returns the previous iterator.
		///
		/// @return A copy of the previous iterator.
		inline CovariantIterator operator++(int) {
			return { (*_iterator)++, _iterator_type };
		}

		/// @brief Checks if two iterators are equal, i.e. they are pointing to the same value.
		///
		/// @param _other The iterator to check against.
		/// @return `true`, if the iterators are pointing to the same value.
		inline bool operator==(const CovariantIterator& _other) const {
			if (this->_iterator_type != _other._iterator_type)
				return false;

			return _iterator->operator==(*_other._iterator);
		}
	};

	/// @brief An input range over another range, where the returned values of type @p T are covariants of the values stored by the underlying range.
	///
	/// An `Enumerable` is intended to be used as a covariant input range for interfaces that want to expose a range of elements that are also interfaces for the stored elements of the actual range. In the
	/// context of an `Enumerable`, *covariance* refers to the type @p T either being assignable or constructible from the underlying range value type. This allows not only derived types (in a stricter
	/// definition that the C++ language standard uses), but also unrelated types, such as smart pointers to be used in a covariant fashion.
	///
	/// In the following example, the interface `IContainer` returns an `Enumerable<IContained>` from a class `Container`, where the contained elements are of type `Contained`.
	///
	/// @par Example
	/// class IContained { }; class Contained : public IContained { };
	/// class IContainer { public: virtual Enumerable<const IContained&> elements() const noexcept = 0; };
	/// class Container : public IContainer { private: std::vector<Contained> _elements;
	/// public: Enumerable<const IContained&> elements() const noexcept override { return _elements; } };
	///
	/// As `Contained` is derived from `IContained`, they are covariant in terms of the language. In the context of `Enumerable`, they are covariant, because a `const IContained&` can be constructed from a
	/// `const Contained&`. This way, using `Enumeable` in `IContainer` allows to iterate the interface instances without knowing their type when declaring the interface. The covariance relation only applies
	/// to the elements of the range, not the range itself. In the example above, `std::vector<Contained>` is not covariant to `Enumerable<const IContained>`, as the two types are not related. This is
	/// important as `Enumerable` has a slight performance impact compared to returning a reference of the underlying range directly, both in terms of memory (it stores type information about the original
	/// iterators) as well as runtime (it requires a virtual call for iterator increments, dereferencing and comparison). If you want to expose covariant ranges in interfaces, but allow for maximum
	/// performance when the implementation is called directly, you can hide the interface method using private inheritance:
	///
	/// @par Example
	/// class IContainer { public: inline Enumerable<const IContained&> elements() const noexcept { return this->getElements(); }
	/// private: virtual Enumerable<const IContained&> getElements() const noexcept = 0; };
	/// class Container : public IContainer { private: std::vector<Contained> _elements;
	/// public: const std::vector<IContained>& elements() const noexcept { return _elements; }
	/// private: Enumerable<const IContained&> getElements() const noexcept override { return _elements; } };
	///
	/// `Enumerable` can be using in two ways: first, it can be initialized with a lvalue reference of the underlying range or view as shown above. In this case, only the begin and end iterators of the
	/// underlying range are stored. However, when passed a rvalue reference, the `Enumerable` will store the underlying range until it and all copies of it are destroyed. This allows to use `Enumerable`
	/// either for returning a temporary range or a view as shown in the example below.
	///
	/// @par Example
	/// Enumerable<Foo> temporaryFoos() { std::vector<Foo> foos {}; foos.emplace_back(); foos.emplace_back(); foos.emplace_back();
	/// return std::move(foos);
	/// // Or better: //return std::vector<Foo>(3); }
	/// Enumerable<Foo&> filteredFoos(const std::vector<Foo>& foos) { return foos | std::views::drop(1) | std::views::take(2); }
	///
	/// Keep in mind that the type parameter @p T dictates what an iterator returns from the `Enumerable`, i.e. if an lvalue or (p)rvalue should be returned and wheather or not a copy is created accordingly.
	///
	/// @tparam T The type of the values returned by the enumerable.
	/// @see CovariantIterator
	template <typename T>
	struct Enumerable {
	public:
		/// @brief The type of the value that is contained by the `Enumerable`.
		using value_type = std::remove_cvref_t<T>;

		/// @brief The type of a pointer returned by the `Enumerable`.
		using pointer = std::remove_reference_t<T>*;

		/// @brief The type of a reference returned by the `Enumerable`.
		using reference = std::remove_reference_t<T>&;

		/// @brief The type of the iterator used to iterate the elements of the `Enumerable`.
		using iterator = CovariantIterator<T>;

		/// @brief The type of the iterator used to iterate constant elements of the `Enumerable`.
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
		/// @brief Creates an enumerable over an empty range.
		inline Enumerable() :
			Enumerable(std::array<T, 0> { })
		{ }

		inline Enumerable(const Enumerable& range) = default;
		inline Enumerable(Enumerable&& range) noexcept = default;
		inline Enumerable& operator=(const Enumerable& range) = default;
		inline Enumerable& operator=(Enumerable&& range) noexcept = default;
		~Enumerable() noexcept = default;

		/// @brief Creates a new `Enumerable` instance from an underlying range.
		///
		/// @tparam TRange The type of the underlying range.
		/// @tparam enabled Disables the constructor, if @p TRange is equal to the current type, in which case the move constructor should be called.
		/// @param range A reference of the underlying range.
		template <typename TRange, typename enabled = std::enable_if_t<!std::is_same_v<TRange, Enumerable>>>
		inline Enumerable(TRange&& range) {
			// NOTE: Concept evaluation may fail here, if we provide some other enumerable, in which case the evaluated type may be not complete yet, which is why have to
			//       do a static assert here instead of providing the concept in the template.
			static_assert(std::ranges::viewable_range<TRange>, "The source range does not satisfy std::ranges::viewable_range!");
			_range = std::make_shared<range_holder<std::ranges::views::all_t<decltype(range)>>>(std::forward<TRange>(range)); // NOLINT(cppcoreguidelines-prefer-member-initializer)
		}

		/// @brief Returns an iterator pointing to the start of the underlying range.
		///
		/// @return An iterator pointing to the start of the underlying range.
		inline auto begin() const noexcept {
			return _range->begin();
		}

		/// @brief Returns an iterator pointing to the end of the underlying range.
		///
		/// @return An iterator pointing to the end of the underlying range.
		inline auto end() const noexcept {
			return _range->end();
		}

		/// @brief Returns a constant iterator pointing to the start of the underlying range.
		///
		/// @return A constant iterator pointing to the start of the underlying range.
		inline auto cbegin() const noexcept {
			return _range->cbegin();
		}

		/// @brief Returns a constant iterator pointing to the end of the underlying range.
		///
		/// @return A constant iterator pointing to the end of the underlying range.
		inline auto cend() const noexcept {
			return _range->cend();
		}

		/// @brief Returns `true`, if there are no elements inside the `Enumerable` and `false` otherwise.
		///
		/// @return `true`, if there are no elements inside the `Enumerable` and `false` otherwise.
		inline bool empty() const noexcept {
			return this->begin() == this->end();
		}
	};

#if (defined(BUILD_LITEFX_PIMPL) && BUILD_LITEFX_PIMPL) || (!defined(BUILD_LITEFX_PIMPL)) && !defined(LITEFX_IMPLEMENTATION)
	/// @brief A smart pointer that manages an implementation instance for a public interface class.
	///
	/// @tparam pImpl The type of the implementation class.
	template <class pImpl>
	class PimplPtr final {
	private:
		/// @brief Stores the shared pointer to the implementation.
		SharedPtr<pImpl> m_ptr;

	public:
		/// @brief Initializes a new pointer to an implementation instance.
		constexpr PimplPtr() /*requires std::is_default_constructible_v<pImpl>*/ :
			m_ptr(makeShared<pImpl>()) { }

		/// @brief Initializes a new pointer of an implementation.
		///
		/// @tparam ...TArgs The types of the arguments passed to the implementation constructor.
		/// @param ...args The arguments passed to the implementation constructor.
		template <typename... TArgs>
		constexpr PimplPtr(TArgs&&... args) /*requires std::constructible_from<pImpl, TArgs...>*/ :
			m_ptr(makeShared<pImpl>(std::forward<TArgs>(args)...)) { } // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)

		/// @brief Initializes a new pointer to a copy of the implementation instance managed by @p src.
		///
		/// @param src The source pointer to copy the implementation instance from.
		constexpr PimplPtr(const PimplPtr& src) /*requires std::copy_constructible<pImpl>*/ : 
			m_ptr(makeShared<pImpl>(*src.m_ptr)) { }

		/// @brief Initializes a new pointer by taking over the implementation instance managed by @p src.
		///
		/// @param src The source pointer to take over.
		constexpr PimplPtr(PimplPtr&& src) noexcept = default;

		/// @brief Initializes a new pointer to a copy of the implementation instance managed by @p src.
		///
		/// Note that this will share ownership between this instance and @p src. Only use this method, if you @ref release either of both implementation pointers manually!
		///
		/// @param src The source pointer to copy the implementation instance from.
		/// @return A new pointer to the provided implementation instance.
		constexpr PimplPtr& operator=(const PimplPtr& src) /*requires std::copy_constructible<pImpl>*/
		{
			if (&src != this)
				m_ptr = makeShared<pImpl>(*src.m_ptr);

			return *this; 
		}

		/// @brief Initializes a new pointer by taking over the implementation instance managed by @p src.
		///
		/// @param src The source pointer to take over.
		/// @return A new pointer to the provided implementation instance.
		constexpr PimplPtr& operator=(PimplPtr&& src) noexcept = default;

		constexpr ~PimplPtr() noexcept = default;

	public:
		/// @brief Returns a reference to the managed implementation instance.
		///
		/// @return A reference to the managed implementation instance.
		constexpr pImpl& operator* () const noexcept { 
			return *m_ptr; 
		}

		/// @brief Returns a pointer to the managed implementation instance.
		///
		/// @return A pointer to the managed implementation instance.
		constexpr pImpl* operator-> () const noexcept { 
			return m_ptr.get(); 
		}
	};

	// NOLINTBEGIN(cppcoreguidelines-macro-usage)

	/// @brief Declares the implementation for the public interface of a class.
	///
	/// A class can access the instance of the implementation instance using the pointer `m_impl` after declaring the implementation using this macro.
	///
	/// @see Implement
#  define LITEFX_IMPLEMENTATION(impl) private: \
	class impl; \
	PimplPtr<impl> m_impl; \
	friend class PimplPtr<impl>; \
	friend class impl;

	// NOLINTEND(cppcoreguidelines-macro-usage)
#endif

	/// @brief Provides access to a resource managed by the class.
	///
	/// @tparam THandle The type of the resource.
	/// @see Resource
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
		/// @brief Returns the resource managed by the class.
		///
		/// @return The resource managed by the class.
		virtual THandle& handle() noexcept = 0;

	public:
		/// @brief Returns the resource managed by the class.
		///
		/// @return The resource managed by the class.
		virtual const THandle& handle() const noexcept = 0;
	};

	/// @brief Implements the @ref IResource interface.
	///
	/// @tparam THandle The type of the resource.
	template <class THandle>
	class Resource : public virtual IResource<THandle> {
	public:
        using handle_type = THandle;

	private:
		THandle m_handle;

	protected:
		/// @brief Initializes the managed resource.
		///
		/// @param handle The managed resource handle.
		explicit Resource(THandle handle) noexcept : m_handle(std::move(handle)) { }
		Resource(const Resource&) = delete;
		Resource(Resource&&) noexcept = default;
		Resource& operator=(const Resource&) = delete;
		Resource& operator=(Resource&&) noexcept = default;

	public:
		~Resource() noexcept override = default;

	protected:
		/// @copydoc IResource::handle()
		THandle& handle() noexcept override { return m_handle; }

	public:
		/// @copydoc IResource::handle() const
		const THandle& handle() const noexcept override { return m_handle; }
	};

	/// @brief Describes an generic builder type.
	///
	/// Builders are helper classes to create objects or object hierarchies. A builder is called *root builder*, if @p TParent is set to `std::nullptr_t`, otherwise it is called *child builder* and @p TParent
	/// is set to the type of the parent builder in the hierarchy. A builder can be a child builder to either a root builder or another child builder. However, at the top of the hierarchy there needs to be
	/// one root builder and each hierarchy can only have one root builder.
	///
	/// Building a child object using a child builder can be terminated by calling `add` on it. This will call `use` on the parent builder and pass the object instance to it. If any work needs to be done by
	/// the builder before passing it to the parent builder, it is possible to overwrite the `build` method. For root builders no additional call is required. Instead a root builder provides a move-assignment
	/// operator for @p T. Assigning the builder instance to an instance of @p TPointer will return the instance object. Similar to child builders, it is possible to overwrite the `build` method, to perform
	/// any additional pre-construction work.
	///
	/// Builders create the object instances they manage in form of smart pointers. The @p TPointer can either be set to any smart pointer type that wraps @p T for convenience.
	///
	/// @tparam T The type of the object the builder builds.
	/// @tparam TParent The type of the parent builder or `std::nullptr_t`.
	/// @tparam TPointer The type of the pointer, used to access the instance of @p T this builder builds.
	template <typename T, typename TParent = std::nullptr_t, typename TPointer = UniquePtr<T>>
	class Builder;

	/// @brief Describes a root builder.
	///
	/// @tparam T The type of the object the builder builds.
	/// @tparam TPointer The type of the pointer, used to access the instance of @p T this builder builds.
	/// @see https://github.com/crud89/LiteFX/wiki/Builders
	template <typename T, typename TPointer>
	class Builder<T, std::nullptr_t, TPointer> {
	private:
		TPointer m_instance;

	public:
		using instance_type = T;
		using parent_type = std::nullptr_t;
		using pointer_type = TPointer;

	public:
		/// @brief Returns a pointer to the current instance of the object that is built by the builder.
		///
		/// @return A pointer to the current object instance.
		constexpr const T* instance() const noexcept { return m_instance.get(); }

	protected:
		/// @brief Returns a pointer to the current instance of the object that is built by the builder.
		///
		/// @return A pointer to the current object instance.
		constexpr T* instance() noexcept { return m_instance.get(); }

	public:
		/// @brief Initializes the builder instance.
		///
		/// @param instance The instance of the object to build.
		constexpr explicit Builder(TPointer&& instance) noexcept : m_instance(std::move(instance)) { }

		/// @brief Initializes the builder instance by taking over another instance.
		///
		/// @param _other The instance of another builder object to take over.
		constexpr Builder(Builder&& _other) noexcept : m_instance(std::move(_other.m_instance)) { }

		Builder(const Builder&) = delete;
		auto operator=(const Builder&) = delete;
		auto operator=(const Builder&&) noexcept = delete;

		constexpr virtual ~Builder() noexcept = default;

	protected:
		/// @brief Can be overwritten to perform any pre-construction work before the builder returns the final object instance.
		constexpr virtual void build() { };

	public:
		// TODO: Provide concept (`is_buildable<TBuilder>`)
		/// @brief Called by child builders to pass a constructed object back to the parent builder.
		///
		/// This method must be implemented for each child builder, a builder can create. This introduces a hard dependency between child and parent builders. It is not possible to define a child builder without
		/// implementing the counter part in the parent builder.
		template <typename TInstance>
		void use(pointer_type&&) noexcept = delete;

		/// @brief Calls @ref build and returns the instance.
		[[nodiscard]] constexpr operator TPointer&& () {
			this->build();
			return std::move(m_instance);
		}
	};

	/// @brief Describes a child builder.
	///
	/// @tparam T The type of the object the builder builds.
	/// @tparam TPointer The type of the pointer, used to access the instance of @p T this builder builds.
	/// @see https://github.com/crud89/LiteFX/wiki/Builders
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
		/// @brief Returns a pointer to the current instance of the object that is built by the builder.
		///
		/// @return A pointer to the current object instance.
		constexpr const T* instance() const noexcept { return m_instance.get(); }

		/// @brief Returns a reference of the parent builder.
		///
		/// @return A reference of the parent builder.
		constexpr const TParent& parent() const noexcept { return *m_parent; }

	protected:
		/// @brief Returns a pointer to the current instance of the object that is built by the builder.
		///
		/// @return A pointer to the current object instance.
		constexpr T* instance() noexcept { return m_instance.get(); }

	public:
		/// @brief Initializes the builder instance.
		///
		/// @param parent The instance of the parent builder.
		/// @param instance The instance of the object to build.
		constexpr explicit Builder(TParent& parent, TPointer&& instance) noexcept : m_instance(std::move(instance)), m_parent(&parent) { }
		
		/// @brief Initializes the builder instance by taking over another instance.
		///
		/// @param _other The instance of another builder object to take over.
		constexpr Builder(Builder&& _other) noexcept : m_instance(std::move(_other.m_instance)), m_parent(_other.m_parent) { }

		constexpr Builder(const Builder&) = delete;
		auto operator=(const Builder&) = delete;
		auto operator=(Builder&&) noexcept = delete;
		constexpr virtual ~Builder() noexcept = default;

	protected:
		/// @brief Can be overwritten to perform any pre-construction work before the builder returns the final object instance.
		constexpr virtual void build() { };

	public:
		// TODO: Provide concept (`is_buildable<TBuilder>`)
		/// @brief Called by child builders to pass a constructed object back to the parent builder.
		///
		/// This method must be implemented for each child builder, a builder can create. This introduces a hard dependency between child and parent builders. It is not possible to define a child builder without
		/// implementing the counter part in the parent builder.
		template <typename TInstance>
		void use(pointer_type&&) noexcept = delete;

		/// @brief First, calls @ref build, then `use` on the parent builder using the current object instance and finally returns the parent builder.
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

	/// @brief Base class for an object that can be shared.
	///
	/// This is an improved version of `std::enable_shared_from_this` that supports inheritance. When inheriting from this class, follow the same practices as you would for `std::enable_shared_from_this`: do
	/// not provide any public constructors; instead provide a private constructor and a publicly accessible static factory method, that returns a shared pointer.
	///
	/// Note that the above rule does not apply for objects that are stored within a @ref PimplPtr, as those are handled correctly by the pointer implementation.
	///
	/// You may want to create objects by creating a static factory method that calls the protected @ref SharedObject::create method. This has the advantage of allocating a single memory block for both, the
	/// object and the shared pointers control block. To do this, make sure to declare friendship to @ref SharedAllocator in your class, as shown in the example below.
	///
	/// @par Example
	/// @code
	/// class Foo : public SharedObject {
	///     friend struct SharedObject::Allocator>Foo<;
	///
	/// private:
	///     explicit Foo(int a, std::string b) { }
	///
	/// public:
	///     static inline auto create(int a, std::string b) {
	///         return SharedObject::create>Foo<(a, b);
	///     }
	/// }
	/// @endcode
	///
	/// @see https://en.cppreference.com/w/cpp/memory/enable_shared_from_this
	class SharedObject : public std::enable_shared_from_this<SharedObject> {
	protected:
		/// @brief Initializes a new shared object.
		SharedObject() noexcept = default;
		SharedObject(SharedObject&&) noexcept = default;
		SharedObject(const SharedObject&) = default;
		SharedObject& operator=(SharedObject&&) noexcept = default;
		SharedObject& operator=(const SharedObject&) = default;

	public:
		/// @brief Destroys the shared object.
		virtual ~SharedObject() noexcept = default;

	protected:
		/// @brief An allocator used to allocate the shared object.
		///
		/// @tparam T The type of the class that inherits from @ref SharedObject.
		template <typename T>
		struct Allocator : public std::allocator<T> {
			template<typename TParent, typename... TArgs>
			void construct(TParent* parent, TArgs&&... args) {
				::new(static_cast<void*>(parent)) TParent(std::forward<TArgs>(args)...);
			}
		};

		/// @brief Generic factory method used to create instances of the shared object.
		///
		/// @tparam T The type of the class that inherits from @ref SharedObject.
		/// @tparam TArgs The types of the arguments passed to the shared object's constructor.
		/// @param args The arguments that are forwarded to the shared object's constructor.
		/// @return A shared pointer of the shared object.
		/// @see Allocator
		template <typename T, typename... TArgs>
		[[nodiscard]] static inline auto create(TArgs&&... args) -> SharedPtr<T> {
			return std::allocate_shared<T>(Allocator<T>{}, std::forward<TArgs>(args)...);
		}

	public:
		/// @brief Returns a shared pointer to the current object instance.
		template <typename TSelf>
		[[nodiscard]] auto inline shared_from_this(this TSelf&& self)
		{
			// TODO: In C++26 we should be able to use `std::is_virtual_base_of<SharedObject, TSelf>` here to prefer a `static_pointer_cast`, if possible.

			return std::dynamic_pointer_cast<std::remove_reference_t<TSelf>>(
				std::forward<TSelf>(self).std::template enable_shared_from_this<SharedObject>::shared_from_this());
		}
		
		/// @brief Returns a weak pointer to the current object instance.
		template <typename TSelf>
		[[nodiscard]] auto inline weak_from_this(this TSelf&& self) noexcept -> WeakPtr<std::remove_reference_t<TSelf>>
		{
			// TODO: In C++26 we should be able to use `std::is_virtual_base_of<SharedObject, TSelf>` here to prefer a `static_pointer_cast`, if possible.

			return std::dynamic_pointer_cast<std::remove_reference_t<TSelf>>(
				std::forward<TSelf>(self).std::template enable_shared_from_this<SharedObject>::weak_from_this().lock());
		}
	};
}
