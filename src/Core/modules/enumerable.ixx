module;

#include <forward_list>
#include <ranges>
#include <array>

export module LiteFX.Core:Enumerable;
import :Concepts;

export namespace LiteFX {

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
		constexpr Enumerable(std::ranges::input_range auto&& input) noexcept requires
			std::convertible_to<std::ranges::range_value_t<decltype(input)>, T>
		{
			auto it = m_elements.before_begin();
			m_size = 0;

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
		template <typename... TArgs> requires are_same<T, TArgs...>
		constexpr explicit inline Enumerable(TArgs&&... args) noexcept
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
		constexpr Enumerable(Enumerable<T>&& _other) = default;

		/// <summary>
		/// Initializes the `Enumerable` by taking over <paramref name="_other" />.
		/// </summary>
		/// <remarks>
		/// Note that this constructor can only be used of <typeparamref name="T" /> is movable.
		/// </remarks>
		/// <param name="_other">The `Enumerable` to take over.</param>
		/// <returns>A reference of the `Enumerable` after the move.</returns>
		constexpr Enumerable<T>& operator=(Enumerable<T>&& _other) = default;

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

}