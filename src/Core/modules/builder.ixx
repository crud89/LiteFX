module;

#include <memory>

export module LiteFX.Core:Builder;
import :Alias;

export namespace LiteFX {

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
	/// The derived type <typeparamref name="TDerived" /> marks the actual implementation of the builder, which itself is derived from this class in 
	/// CRTP-fashion. It is typically used to define builder methods that return references to `this`.
	/// 
	/// Builders create the object instances they manage in form of smart pointers. The <typeparamref name="TPointer" /> can either be set to any smart
	/// pointer type that wraps <typeparamref name="T" /> for convenience.
	/// </remarks>
	/// <typeparam name="TDerived">The concrete implementation of the builder itself.</typeparam>
	/// <typeparam name="T">The type of the object the builder builds.</typeparam>
	/// <typeparam name="TParent">The type of the parent builder or `std::nullptr_t`.</typeparam>
	/// <typeparam name="TPointer">The type of the pointer, used to access the instance of <typeparamref name="T" /> this builder builds.</typeparam>
	template <typename TDerived, typename T, typename TParent = std::nullptr_t, typename TPointer = UniquePtr<T>>
	class Builder;

	/// <summary>
	/// Describes a root builder.
	/// </summary>
	/// <typeparam name="TDerived">The concrete implementation of the builder itself.</typeparam>
	/// <typeparam name="T">The type of the object the builder builds.</typeparam>
	/// <typeparam name="TPointer">The type of the pointer, used to access the instance of <typeparamref name="T" /> this builder builds.</typeparam>
	/// <seealso href="https://github.com/crud89/LiteFX/wiki/Builders" />
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
		/// <summary>
		/// Returns a pointer to the current instance of the object that is built by the builder.
		/// </summary>
		/// <returns>A pointer to the current object instance.</returns>
		const T* instance() const noexcept { return m_instance.get(); }

	protected:
		/// <summary>
		/// Returns a pointer to the current instance of the object that is built by the builder.
		/// </summary>
		/// <returns>A pointer to the current object instance.</returns>
		T* instance() noexcept { return m_instance.get(); }

	public:
		/// <summary>
		/// Initializes the builder instance.
		/// </summary>
		/// <param name="instance">The instance of the object to build.</param>
		explicit Builder(TPointer&& instance) noexcept : m_instance(std::move(instance)) { }

		/// <summary>
		/// Initializes the builder instance by taking over another instance.
		/// </summary>
		/// <param name="_other">The instance of another builder object to take over.</param>
		Builder(builder_type&& _other) noexcept : m_instance(std::move(_other.m_instance)) { }

		Builder(const builder_type&) = delete;
		virtual ~Builder() noexcept = default;

	protected:
		/// <summary>
		/// Can be overwritten to perform any pre-construction work before the builder returns the final object instance.
		/// </summary>
		virtual void build() { };

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
		void use(pointer_type&&) = delete;

		/// <summary>
		/// Calls <see cref="build" /> and returns the instance.
		/// </summary>
		[[nodiscard]]
		operator TPointer && () {
			this->build();
			return std::move(m_instance);
		}
	};

	/// <summary>
	/// Describes a child builder.
	/// </summary>
	/// <typeparam name="TDerived">The concrete implementation of the builder itself.</typeparam>
	/// <typeparam name="T">The type of the object the builder builds.</typeparam>
	/// <typeparam name="TPointer">The type of the pointer, used to access the instance of <typeparamref name="T" /> this builder builds.</typeparam>
	/// <seealso href="https://github.com/crud89/LiteFX/wiki/Builders" />
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
		/// <summary>
		/// Returns a pointer to the current instance of the object that is built by the builder.
		/// </summary>
		/// <returns>A pointer to the current object instance.</returns>
		const T* instance() const noexcept { return m_instance.get(); }

		/// <summary>
		/// Returns a reference of the parent builder.
		/// </summary>
		/// <returns>A reference of the parent builder.</returns>
		const TParent& parent() const noexcept { return m_parent; }

	protected:
		/// <summary>
		/// Returns a pointer to the current instance of the object that is built by the builder.
		/// </summary>
		/// <returns>A pointer to the current object instance.</returns>
		T* instance() noexcept { return m_instance.get(); }

	public:
		/// <summary>
		/// Initializes the builder instance.
		/// </summary>
		/// <param name="parent">The instance of the parent builder.</param>
		/// <param name="instance">The instance of the object to build.</param>
		explicit Builder(TParent& parent, TPointer&& instance) noexcept : m_parent(parent), m_instance(std::move(instance)) { }

		/// <summary>
		/// Initializes the builder instance by taking over another instance.
		/// </summary>
		/// <param name="_other">The instance of another builder object to take over.</param>
		Builder(builder_type&& _other) noexcept : m_instance(std::move(_other.m_instance)), m_parent(_other.m_parent) { }

		Builder(const builder_type&) = delete;
		virtual ~Builder() noexcept = default;

	protected:
		/// <summary>
		/// Can be overwritten to perform any pre-construction work before the builder returns the final object instance.
		/// </summary>
		virtual void build() { };

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
		void use(pointer_type&&) = delete;

		/// <summary>
		/// First, calls <see cref="build" />, then `use` on the parent builder using the current object instance and finally returns the parent builder.
		/// </summary>
		[[nodiscard]]
		TParent& add() {
			this->build();
			m_parent.use(std::move(m_instance));
			return m_parent;
		}
	};

}