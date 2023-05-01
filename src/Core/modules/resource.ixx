module;

export module LiteFX.Core:Resource;

export namespace LiteFX {

	/// <summary>
	/// Provides access to a resource managed by the class.
	/// </summary>
	/// <seealso cref="Resource" />
	/// <typeparam name="THandle">The type of the resource.</typeparam>
	template <class THandle>
	class IResource {
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
	private:
		THandle m_handle;

	protected:
		/// <summary>
		/// Initializes the managed resource.
		/// </summary>
		/// <param name="handle">The managed resource handle.</param>
		explicit Resource(const THandle handle) noexcept : m_handle(handle) { }

	public:
		Resource(const Resource&) = delete;
		Resource(Resource&&) = delete;
		virtual ~Resource() noexcept = default;

	protected:
		/// <inheritdoc />
		THandle& handle() noexcept override { return m_handle; }

	public:
		/// <inheritdoc />
		const THandle& handle() const noexcept override { return m_handle; }
	};

}