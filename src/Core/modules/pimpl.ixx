module;

#include <utility>

export module LiteFX.Core:Pimpl;
import :Alias;
import :Exceptions;

export namespace LiteFX {

	/// <summary>
	/// A smart pointer that manages an implementation instance for a public interface class.
	/// </summary>
	/// <typeparam name="pImpl">The type of the implementaion class.</typeparam>
	template <class pImpl>
	class PimplPtr {
	private:
		UniquePtr<pImpl> m_ptr;

	public:
		/// <summary>
		/// Initializes a new pointer to an uninitialized implementation instance.
		/// </summary>
		PimplPtr() noexcept = default;

		/// <summary>
		/// Initializes a new pointer to a copy of the implementation instance managed by <paramref name="src" />.
		/// </summary>
		/// <remarks>
		/// Note that this will share ownership between this instance and <paramref name="src" />. Only use this method, if you <see cref="release" /> either
		/// of both implementation pointers manually!
		/// </remarks>
		/// <param name="src">The source pointer to copy the implementation instance from.</param>
		PimplPtr(const PimplPtr& src) noexcept : m_ptr(new pImpl(*src.m_ptr)) { }

		/// <summary>
		/// Initializes a new pointer by taking over the implementation instance managed by <paramref name="src" />.
		/// </summary>
		/// <param name="src">The source pointer to take over.</param>
		PimplPtr(PimplPtr&& src) noexcept = default;

		/// <summary>
		/// Initializes a new pointer to a copy of the implementation instance managed by <paramref name="src" />.
		/// </summary>
		/// <remarks>
		/// Note that this will share ownership between this instance and <paramref name="src" />. Only use this method, if you <see cref="release" /> either
		/// of both implementation pointers manually!
		/// </remarks>
		/// <param name="src">The source pointer to copy the implementation instance from.</param>
		/// <returns>A new pointer to the provided implementation instance.</returns>
		PimplPtr& operator= (const PimplPtr& src) noexcept { m_ptr.reset(new pImpl(*src.m_ptr)); return *this; }

		/// <summary>
		/// Initializes a new pointer by taking over the implementation instance managed by <paramref name="src" />.
		/// </summary>
		/// <param name="src">The source pointer to take over.</param>
		/// /// <returns>A new pointer to the provided implementation instance.</returns>
		PimplPtr& operator= (PimplPtr&& src) noexcept = default;

		~PimplPtr() noexcept = default;

	private:
		/// <summary>
		/// Initializes a new pointer from the raw pointer provided with <paramref name="pimpl" />.
		/// </summary>
		/// <param name="pimpl">The raw pointer to take ownership over.</param>
		PimplPtr(pImpl* pimpl) noexcept : m_ptr(pimpl) { }

	public:
		/// <summary>
		/// Destroys the implementation instance managed by this pointer.
		/// </summary>
		void destroy() { m_ptr = nullptr; }

		/// <summary>
		/// Releases the implementation instance managed by this pointer and returns it.
		/// </summary>
		/// <returns>The pointer to the managed implementation instance.</returns>
		pImpl* release() noexcept { m_ptr.release(); }

	public:
		/// <summary>
		/// Returns a reference to the managed implementation instance.
		/// </summary>
		/// <returns>A reference to the managed implementation instance.</returns>
		pImpl& operator* () const noexcept { return *m_ptr; }

		/// <summary>
		/// Returns a pointer to the managed implementation instance.
		/// </summary>
		/// <returns>A pointer to the managed implementation instance.</returns>
		pImpl* operator-> () const noexcept { return m_ptr.get(); }

	public:
		template <class T, class... Arg>
		friend PimplPtr<T> makePimpl(Arg&&... arg);
	};

	/// <summary>
	/// Creates a pointer to an implementation.
	/// </summary>
	/// <typeparam name="T">The type of the implementation class.</typeparam>
	/// <typeparam name="...Arg">The variadic argument types forwarded to the implementation classes' constructor.</typeparam>
	/// <param name="...arg">The arguments forwarded to the implementation classes' constructor.</param>
	/// <returns>The pointer to the implementation class instance.</returns>
	template <class T, class... Arg>
	PimplPtr<T> makePimpl(Arg&&... arg) {
		return PimplPtr<T>(new T(std::forward<Arg>(arg)...));
	}

	/// <summary>
	/// Base class for an implementation of a public interface class.
	/// </summary>
	/// <seealso cref="LITEFX_IMPLEMENTATION" />
	/// <typeparam name="TInterface">The public interface class that should be implemented.</typeparam>
	template <class TInterface>
	class Implement {
	public:
		using interface_type = TInterface;
		using base = Implement<interface_type>;

	protected:
		TInterface* m_parent{ nullptr };

	public:
		/// <summary>
		/// Initializes the implementation instance.
		/// </summary>
		/// <param name="parent">The pointer to the parent public interface instance.</param>
		Implement(TInterface* parent) : m_parent(parent) {
			if (parent == nullptr)
				throw std::runtime_error("Initializing an implementation requires the parent to be provided.");
		}

		Implement(Implement<TInterface>&&) = delete;
		Implement(const Implement<TInterface>&) = delete;
		virtual ~Implement() = default;
	};

}