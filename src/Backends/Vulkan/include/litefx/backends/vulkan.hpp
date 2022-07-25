#pragma once

#include <litefx/rendering.hpp>

#include "vulkan_api.hpp"
#include "vulkan_formatters.hpp"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Implements a Vulkan vertex buffer layout.
	/// </summary>
	/// <seealso cref="VulkanVertexBuffer" />
	/// <seealso cref="VulkanIndexBufferLayout" />
	/// <seealso cref="VulkanVertexBufferLayoutBuilder" />
	class LITEFX_VULKAN_API VulkanVertexBufferLayout : public IVertexBufferLayout {
		LITEFX_IMPLEMENTATION(VulkanVertexBufferLayoutImpl);
		LITEFX_BUILDER(VulkanVertexBufferLayoutBuilder);

	public:
		/// <summary>
		/// Initializes a new vertex buffer layout.
		/// </summary>
		/// <param name="vertexSize">The size of a single vertex.</param>
		/// <param name="binding">The binding point of the vertex buffers using this layout.</param>
		explicit VulkanVertexBufferLayout(const size_t& vertexSize, const UInt32& binding = 0);
		VulkanVertexBufferLayout(VulkanVertexBufferLayout&&) = delete;
		VulkanVertexBufferLayout(const VulkanVertexBufferLayout&) = delete;
		virtual ~VulkanVertexBufferLayout() noexcept;

		// IVertexBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual Array<const BufferAttribute*> attributes() const noexcept override;

		// IBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual size_t elementSize() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		/// <inheritdoc />
		virtual const BufferType& type() const noexcept override;
	};

	/// <summary>
	/// Implements a Vulkan index buffer layout.
	/// </summary>
	/// <seealso cref="VulkanIndexBuffer" />
	/// <seealso cref="VulkanVertexBufferLayout" />
	class LITEFX_VULKAN_API VulkanIndexBufferLayout : public IIndexBufferLayout {
		LITEFX_IMPLEMENTATION(VulkanIndexBufferLayoutImpl);

	public:
		/// <summary>
		/// Initializes a new index buffer layout
		/// </summary>
		/// <param name="type">The type of the indices within the index buffer.</param>
		explicit VulkanIndexBufferLayout(const IndexType& type);
		VulkanIndexBufferLayout(VulkanIndexBufferLayout&&) = delete;
		VulkanIndexBufferLayout(const VulkanIndexBufferLayout&) = delete;
		virtual ~VulkanIndexBufferLayout() noexcept;

		// IIndexBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual const IndexType& indexType() const noexcept override;

		// IBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual size_t elementSize() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		/// <inheritdoc />
		virtual const BufferType& type() const noexcept override;
	};

	/// <summary>
	/// Implements a Vulkan <see cref="IDescriptorLayout" />
	/// </summary>
	/// <seealso cref="IVulkanBuffer" />
	/// <seealso cref="IVulkanImage" />
	/// <seealso cref="IVulkanSampler" />
	/// <seealso cref="VulkanDescriptorSet" />
	/// <seealso cref="VulkanDescriptorSetLayout" />
	class LITEFX_VULKAN_API VulkanDescriptorLayout : public IDescriptorLayout {
		LITEFX_IMPLEMENTATION(VulkanDescriptorLayoutImpl);

	public:
		/// <summary>
		/// Initializes a new Vulkan descriptor layout.
		/// </summary>
		/// <param name="type">The type of the descriptor.</param>
		/// <param name="binding">The binding point for the descriptor.</param>
		/// <param name="elementSize">The size of the descriptor.</param>
		/// <param name="elementSize">The number of descriptors in the descriptor array.</param>
		explicit VulkanDescriptorLayout(const DescriptorType& type, const UInt32& binding, const size_t& elementSize, const UInt32& descriptors = 1);
		VulkanDescriptorLayout(VulkanDescriptorLayout&&) = delete;
		VulkanDescriptorLayout(const VulkanDescriptorLayout&) = delete;
		virtual ~VulkanDescriptorLayout() noexcept;

		// IDescriptorLayout interface.
	public:
		/// <inheritdoc />
		virtual const DescriptorType& descriptorType() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& descriptors() const noexcept override;

		// IBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual size_t elementSize() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		/// <inheritdoc />
		virtual const BufferType& type() const noexcept override;
	};

	/// <summary>
	/// Represents the base interface for a Vulkan buffer implementation.
	/// </summary>
	/// <seealso cref="VulkanDescriptorSet" />
	/// <seealso cref="IVulkanImage" />
	/// <seealso cref="IVulkanVertexBuffer" />
	/// <seealso cref="IVulkanIndexBuffer" />
	class LITEFX_VULKAN_API IVulkanBuffer : public virtual IBuffer, public virtual IResource<VkBuffer> {
	public:
		virtual ~IVulkanBuffer() noexcept = default;
	};

	/// <summary>
	/// Represents a Vulkan vertex buffer.
	/// </summary>
	/// <seealso cref="VulkanVertexBufferLayout" />
	/// <seealso cref="IVulkanBuffer" />
	class LITEFX_VULKAN_API IVulkanVertexBuffer : public virtual VertexBuffer<VulkanVertexBufferLayout>, public IVulkanBuffer {
	public:
		virtual ~IVulkanVertexBuffer() noexcept = default;
	};

	/// <summary>
	/// Represents a Vulkan index buffer.
	/// </summary>
	/// <seealso cref="VulkanIndexBufferLayout" />
	/// <seealso cref="IVulkanBuffer" />
	class LITEFX_VULKAN_API IVulkanIndexBuffer : public virtual IndexBuffer<VulkanIndexBufferLayout>, public IVulkanBuffer {
	public:
		virtual ~IVulkanIndexBuffer() noexcept = default;
	};

	/// <summary>
	/// Represents a Vulkan sampled image or the base interface for a texture.
	/// </summary>
	/// <seealso cref="VulkanDescriptorLayout" />
	/// <seealso cref="VulkanDescriptorSet" />
	/// <seealso cref="VulkanDescriptorSetLayout" />
	/// <seealso cref="IVulkanBuffer" />
	/// <seealso cref="IVulkanSampler" />
	class LITEFX_VULKAN_API IVulkanImage : public virtual IImage, public virtual IResource<VkImage> {
	public:
		virtual ~IVulkanImage() noexcept = default;

	public:
		/// <summary>
		/// Returns the image resource aspect mask for all sub-resources.
		/// </summary>
		/// <returns>The image resource aspect mask.</returns>
		virtual VkImageAspectFlags aspectMask() const noexcept = 0;

		/// <summary>
		/// Returns the image resource aspect mask for a single sub-resource.
		/// </summary>
		/// <param name="plane">The sub-resource identifier to query the aspect mask from.</param>
		/// <returns>The image resource aspect mask.</returns>
		virtual VkImageAspectFlags aspectMask(const UInt32& plane) const = 0;

		/// <summary>
		/// Computes the <paramref name="plane" />, <paramref name="layer" /> and <paramref name="level" /> indices from the <paramref name="subresource" /> identifier.
		/// </summary>
		/// <param name="subresource">The identifier of the sub-resource.</param>
		/// <param name="plane">The plane of the sub-resource.</param>
		/// <param name="layer">The layer of the sub-resource.</param>
		/// <param name="level">The level of the sub-resource.</param>
		virtual void resolveSubresource(const UInt32& subresource, UInt32& plane, UInt32& layer, UInt32& level) const = 0;

		/// <summary>
		/// Returns the image view for a sub-resource.
		/// </summary>
		/// <param name="plane">The sub-resource index to return the image view for.</param>
		/// <returns>The image view for the sub-resource.</returns>
		virtual const VkImageView& imageView(const UInt32& plane = 0) const = 0;
	};

	/// <summary>
	/// Represents a Vulkan sampler.
	/// </summary>
	/// <seealso cref="VulkanDescriptorLayout" />
	/// <seealso cref="VulkanDescriptorSet" />
	/// <seealso cref="VulkanDescriptorSetLayout" />
	/// <seealso cref="IVulkanImage" />
	class LITEFX_VULKAN_API IVulkanSampler : public ISampler, public virtual IResource<VkSampler> {
	public:
		virtual ~IVulkanSampler() noexcept = default;
	};

	/// <summary>
	/// Implements a Vulkan resource barrier.
	/// </summary>
	/// <seealso cref="VulkanCommandBuffer" />
	/// <seealso cref="IVulkanBuffer" />
	/// <seealso cref="IVulkanImage" />
	/// <seealso cref="Barrier" />
	class LITEFX_VULKAN_API VulkanBarrier : public Barrier<IVulkanBuffer, IVulkanImage> {
		LITEFX_IMPLEMENTATION(VulkanBarrierImpl);

	public:
		/// <summary>
		/// Initializes a new Vulkan barrier.
		/// </summary>
		explicit VulkanBarrier() noexcept;
		VulkanBarrier(const VulkanBarrier&) = delete;
		VulkanBarrier(VulkanBarrier&&) = delete;
		virtual ~VulkanBarrier() noexcept;

		// Barrier interface.
	public:
		/// <inheritdoc />
		virtual void transition(IVulkanBuffer& buffer, const ResourceState& targetState) override;

		/// <inheritdoc />
		virtual void transition(IVulkanBuffer& buffer, const UInt32& element, const ResourceState& targetState) override;

		/// <inheritdoc />
		virtual void transition(IVulkanBuffer& buffer, const ResourceState& sourceState, const ResourceState& targetState) override;

		/// <inheritdoc />
		virtual void transition(IVulkanBuffer& buffer, const ResourceState& sourceState, const UInt32& element, const ResourceState& targetState) override;

		/// <inheritdoc />
		virtual void transition(IVulkanImage& image, const ResourceState& targetState) override;

		/// <inheritdoc />
		virtual void transition(IVulkanImage& image, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) override;

		/// <inheritdoc />
		virtual void transition(IVulkanImage& image, const ResourceState& sourceState, const ResourceState& targetState) override;

		/// <inheritdoc />
		virtual void transition(IVulkanImage& image, const ResourceState& sourceState, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) override;

		/// <inheritdoc />
		virtual void waitFor(const IVulkanBuffer& buffer) override;

		/// <inheritdoc />
		virtual void waitFor(const IVulkanImage& image) override;

	public:
		/// <summary>
		/// Adds the barrier to a command buffer and updates the resource target states.
		/// </summary>
		/// <param name="commandBuffer">The command buffer to add the barriers to.</param>
		virtual void execute(const VulkanCommandBuffer& commandBuffer) const noexcept;

		/// <summary>
		/// Adds the inverse barriers to a command buffers and updates the resource target states.
		/// </summary>
		/// <remarks>
		/// This method can be used to quickly transition all resources back to the source state without requiring to record a new barrier. It performs the opposite transitions to
		/// the ones created with <see cref="execute" />.
		/// </remarks>
		/// <param name="commandBuffer">The command buffer to add the barriers to.</param>
		virtual void executeInverse(const VulkanCommandBuffer& commandBuffer) const noexcept;
	};

	/// <summary>
	/// Implements a Vulkan <see cref="IShaderModule" />.
	/// </summary>
	/// <seealso cref="VulkanShaderProgram" />
	/// <seealso cref="VulkanDevice" />
	class LITEFX_VULKAN_API VulkanShaderModule : public IShaderModule, public Resource<VkShaderModule> {
		LITEFX_IMPLEMENTATION(VulkanShaderModuleImpl);

	public:
		/// <summary>
		/// Initializes a new Vulkan shader module.
		/// </summary>
		/// <param name="device">The parent device, this shader module has been created from.</param>
		/// <param name="type">The shader stage, this module is used in.</param>
		/// <param name="fileName">The file name of the module source.</param>
		/// <param name="entryPoint">The name of the module entry point.</param>
		explicit VulkanShaderModule(const VulkanDevice& device, const ShaderStage& type, const String& fileName, const String& entryPoint = "main");
		VulkanShaderModule(const VulkanShaderModule&) noexcept = delete;
		VulkanShaderModule(VulkanShaderModule&&) noexcept = delete;
		virtual ~VulkanShaderModule() noexcept;

		// ShaderModule interface.
	public:
		/// <inheritdoc />
		virtual const String& fileName() const noexcept override;

		/// <inheritdoc />
		virtual const String& entryPoint() const noexcept override;

		/// <inheritdoc />
		virtual const ShaderStage& type() const noexcept override;

	public:
		/// <summary>
		/// Returns the shader stage creation info for convenience.
		/// </summary>
		/// <returns>The shader stage creation info for convenience.</returns>
		virtual VkPipelineShaderStageCreateInfo shaderStageDefinition() const;
	};

	/// <summary>
	/// Implements a Vulkan <see cref="ShaderProgram" />.
	/// </summary>
	/// <seealso cref="VulkanShaderProgramBuilder" />
	/// <seealso cref="VulkanShaderModule" />
	class LITEFX_VULKAN_API VulkanShaderProgram : public ShaderProgram<VulkanShaderModule> {
		LITEFX_IMPLEMENTATION(VulkanShaderProgramImpl);
		LITEFX_BUILDER(VulkanShaderProgramBuilder);

	public:
		/// <summary>
		/// Initializes a new Vulkan shader program.
		/// </summary>
		/// <param name="modules">The shader modules used by the shader program.</param>
		explicit VulkanShaderProgram(Array<UniquePtr<VulkanShaderModule>>&& modules);
		VulkanShaderProgram(VulkanShaderProgram&&) noexcept = delete;
		VulkanShaderProgram(const VulkanShaderProgram&) noexcept = delete;
		virtual ~VulkanShaderProgram() noexcept;

	private:
		/// <summary>
		/// Initializes a new Vulkan shader program.
		/// </summary>
		explicit VulkanShaderProgram() noexcept;

	public:
		/// <inheritdoc />
		virtual Array<const VulkanShaderModule*> modules() const noexcept override;
	};

	/// <summary>
	/// Implements a Vulkan <see cref="DescriptorSet" />.
	/// </summary>
	/// <seealso cref="VulkanDescriptorSetLayout" />
	class LITEFX_VULKAN_API VulkanDescriptorSet : public DescriptorSet<IVulkanBuffer, IVulkanImage, IVulkanSampler>, public Resource<VkDescriptorSet> {
		LITEFX_IMPLEMENTATION(VulkanDescriptorSetImpl);

	public:
		/// <summary>
		/// Initializes a new descriptor set.
		/// </summary>
		/// <param name="layout">The parent descriptor set layout.</param>
		/// <param name="descriptorSet">The descriptor set handle.</param>
		explicit VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, VkDescriptorSet descriptorSet);
		VulkanDescriptorSet(VulkanDescriptorSet&&) = delete;
		VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;
		virtual ~VulkanDescriptorSet() noexcept;

	public:
		/// <summary>
		/// Returns the parent descriptor set layout.
		/// </summary>
		/// <returns>The parent descriptor set layout.</returns>
		virtual const VulkanDescriptorSetLayout& layout() const noexcept;

	public:
		/// <inheritdoc />
		virtual void update(const UInt32& binding, const IVulkanBuffer& buffer, const UInt32& bufferElement = 0, const UInt32& elements = 1, const UInt32& firstDescriptor = 0) const override;

		/// <inheritdoc />
		virtual void update(const UInt32& binding, const IVulkanImage& texture, const UInt32& descriptor = 0, const UInt32& firstLevel = 0, const UInt32& levels = 0, const UInt32& firstLayer = 0, const UInt32& layers = 0) const override;

		/// <inheritdoc />
		virtual void update(const UInt32& binding, const IVulkanSampler& sampler, const UInt32& descriptor = 0) const override;

		/// <inheritdoc />
		virtual void attach(const UInt32& binding, const IVulkanImage& image) const override;
	};

	/// <summary>
	/// Implements a Vulkan <see cref="DescriptorSetLayout" />.
	/// </summary>
	/// <seealso cref="VulkanDescriptorSet" />
	/// <seealso cref="VulkanRenderPipelineDescriptorSetLayoutBuilder" />
	class LITEFX_VULKAN_API VulkanDescriptorSetLayout : public DescriptorSetLayout<VulkanDescriptorLayout, VulkanDescriptorSet>, public Resource<VkDescriptorSetLayout> {
		LITEFX_IMPLEMENTATION(VulkanDescriptorSetLayoutImpl);
		LITEFX_BUILDER(VulkanDescriptorSetLayoutBuilder);

	public:
		/// <summary>
		/// Initializes a Vulkan descriptor set layout.
		/// </summary>
		/// <param name="device">The parent device, the pipeline layout has been created from.</param>
		/// <param name="descriptorLayouts">The descriptor layouts of the descriptors within the descriptor set.</param>
		/// <param name="space">The space or set id of the descriptor set.</param>
		/// <param name="stages">The shader stages, the descriptor sets are bound to.</param>
		/// <param name="poolSize">The size of a descriptor pool.</param>
		explicit VulkanDescriptorSetLayout(const VulkanDevice& device, Array<UniquePtr<VulkanDescriptorLayout>>&& descriptorLayouts, const UInt32& space, const ShaderStage& stages, const UInt32& poolSize = 1024);
		VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&&) = delete;
		VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
		virtual ~VulkanDescriptorSetLayout() noexcept;

	private:
		/// <summary>
		/// Initializes a Vulkan descriptor set layout.
		/// </summary>
		/// <param name="device">The parent device, the pipeline layout has been created from.</param>
		explicit VulkanDescriptorSetLayout(const VulkanDevice& device) noexcept;

	public:
		/// <summary>
		/// Returns the device, the pipeline layout has been created from.
		/// </summary>
		/// <returns>A reference of the device, the pipeline layout has been created from.</returns>
		virtual const VulkanDevice& device() const noexcept;

	public:
		/// <inheritdoc />
		virtual Array<const VulkanDescriptorLayout*> descriptors() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanDescriptorLayout& descriptor(const UInt32& binding) const override;

		/// <inheritdoc />
		virtual const UInt32& space() const noexcept override;

		/// <inheritdoc />
		virtual const ShaderStage& shaderStages() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 uniforms() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 storages() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 images() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 buffers() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 samplers() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 inputAttachments() const noexcept override;

	public:
		/// <inheritdoc />
		virtual UniquePtr<VulkanDescriptorSet> allocate() const noexcept override;

		/// <inheritdoc />
		virtual Array<UniquePtr<VulkanDescriptorSet>> allocate(const UInt32& descriptorSets) const noexcept override;

		/// <inheritdoc />
		virtual void free(const VulkanDescriptorSet& descriptorSet) const noexcept override;

	public:
		/// <summary>
		/// The size of each descriptor pool.
		/// </summary>
		/// <remarks>
		/// Descriptors are allocated from descriptor pools in Vulkan. Each descriptor pool has a number of descriptor sets it can hand out. Before allocating a new descriptor set
		/// the layout tries to find an unused descriptor set, that it can hand out. If there are no free descriptor sets, the layout tries to allocate a new one. This is only possible
		/// if the descriptor pool is not yet full, in which case a new pool needs to be created. All created pools are cached and destroyed, if the layout itself gets destroyed, 
		/// causing all descriptor sets allocated from the layout to be invalidated. 
		/// 
		/// In general, if the number of required descriptor sets can be pre-calculated, it should be used as a pool size. Otherwise there is a trade-off to be made, based on the 
		/// frequency of which new descriptor sets are required. A small pool size is more memory efficient, but can have a significant runtime cost, as long as new allocations happen
		/// and no descriptor sets can be reused. A large pool size on the other hand is faster, whilst it may leave a large chunk of descriptor sets unallocated. Keep in mind, that the 
		/// layout might not be the only active layout, hence a large portion of descriptor sets might end up not being used.
		/// </remarks>
		/// <returns>The size of one descriptor pool.</returns>
		/// <seealso cref="allocate" />
		/// <seealso cref="free" />
		/// <seealso cref="pools" />
		virtual const UInt32& poolSize() const noexcept;

		/// <summary>
		/// Returns the number of active descriptor pools.
		/// </summary>
		/// <returns>The number of active descriptor pools.</returns>
		/// <seealso cref="allocate" />
		/// <seealso cref="free" />
		/// <seealso cref="poolSize" />
		virtual size_t pools() const noexcept;
	};

	/// <summary>
	/// Implements the Vulkan <see cref="IPushConstantsRange" />.
	/// </summary>
	/// <seealso cref="VulkanPushConstantsLayout" />
	class LITEFX_VULKAN_API VulkanPushConstantsRange : public IPushConstantsRange {
		LITEFX_IMPLEMENTATION(VulkanPushConstantsRangeImpl);

	public:
		/// <summary>
		/// Initializes a new push constants range.
		/// </summary>
		/// <param name="shaderStage">The shader stage, that access the push constants from the range.</param>
		/// <param name="offset">The offset relative to the parent push constants backing memory that marks the beginning of the range.</param>
		/// <param name="size">The size of the push constants range.</param>
		/// <param name="space">The space from which the push constants of the range will be accessible in the shader.</param>
		/// <param name="binding">The register from which the push constants of the range will be accessible in the shader.</param>
		explicit VulkanPushConstantsRange(const ShaderStage& shaderStage, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding);
		VulkanPushConstantsRange(const VulkanPushConstantsRange&) = delete;
		VulkanPushConstantsRange(VulkanPushConstantsRange&&) = delete;
		virtual ~VulkanPushConstantsRange() noexcept;

	public:
		/// <inheritdoc />
		virtual const UInt32& space() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& offset() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& size() const noexcept override;

		/// <inheritdoc />
		virtual const ShaderStage& stage() const noexcept override;
	};

	/// <summary>
	/// Implements the Vulkan <see cref="PushConstantsLayout" />.
	/// </summary>
	/// <seealso cref="VulkanPushConstantsRange" />
	/// <seealso cref="VulkanRenderPipelinePushConstantsLayoutBuilder" />
	/// <seealso cref="VulkanComputePipelinePushConstantsLayoutBuilder" />
	class LITEFX_VULKAN_API VulkanPushConstantsLayout : public PushConstantsLayout<VulkanPushConstantsRange> {
		LITEFX_IMPLEMENTATION(VulkanPushConstantsLayoutImpl);
		LITEFX_BUILDER(VulkanPushConstantsLayoutBuilder);
		friend class VulkanPipelineLayout;

	public:
		/// <summary>
		/// Initializes a new push constants layout.
		/// </summary>
		/// <param name="ranges">The ranges contained by the layout.</param>
		/// <param name="size">The overall size (in bytes) of the push constants backing memory.</param>
		explicit VulkanPushConstantsLayout(Array<UniquePtr<VulkanPushConstantsRange>>&& ranges, const UInt32& size);
		VulkanPushConstantsLayout(const VulkanPushConstantsLayout&) = delete;
		VulkanPushConstantsLayout(VulkanPushConstantsLayout&&) = delete;
		virtual ~VulkanPushConstantsLayout() noexcept;

	private:
		/// <summary>
		/// Initializes a new push constants layout.
		/// </summary>
		/// <param name="size">The overall size (in bytes) of the push constants backing memory.</param>
		explicit VulkanPushConstantsLayout(const UInt32& size);

	public:
		/// <summary>
		/// Returns the parent pipeline layout, the push constants are described for.
		/// </summary>
		/// <returns>A reference of the parent pipeline layout.</returns>
		virtual const VulkanPipelineLayout& pipelineLayout() const;

	private:
		/// <summary>
		/// Sets the parent pipeline layout, the push constants are described for.
		/// </summary>
		/// <param name="pipelineLayout">The parent pipeline layout.</param>
		virtual void pipelineLayout(const VulkanPipelineLayout& pipelineLayout);
	
	public:
		/// <inheritdoc />
		virtual const UInt32& size() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanPushConstantsRange& range(const ShaderStage& stage) const override;

		/// <inheritdoc />
		virtual Array<const VulkanPushConstantsRange*> ranges() const noexcept override;
	};

	/// <summary>
	/// Implements a Vulkan <see cref="PipelineLayout" />.
	/// </summary>
	/// <seealso cref="VulkanRenderPipelineLayoutBuilder" />
	class LITEFX_VULKAN_API VulkanPipelineLayout : public PipelineLayout<VulkanDescriptorSetLayout, VulkanPushConstantsLayout>, public Resource<VkPipelineLayout> {
		LITEFX_IMPLEMENTATION(VulkanPipelineLayoutImpl);
		LITEFX_BUILDER(VulkanPipelineLayoutBuilder);

	public:
		/// <summary>
		/// Initializes a new Vulkan render pipeline layout.
		/// </summary>
		/// <param name="device">The parent device, the layout is created from.</param>
		/// <param name="descriptorSetLayouts">The descriptor set layouts used by the pipeline.</param>
		/// <param name="pushConstantsLayout">The push constants layout used by the pipeline.</param>
		explicit VulkanPipelineLayout(const VulkanDevice& device, Array<UniquePtr<VulkanDescriptorSetLayout>>&& descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout);
		VulkanPipelineLayout(VulkanPipelineLayout&&) noexcept = delete;
		VulkanPipelineLayout(const VulkanPipelineLayout&) noexcept = delete;
		virtual ~VulkanPipelineLayout() noexcept;

	private:
		/// <summary>
		/// Initializes a new Vulkan render pipeline layout.
		/// </summary>
		/// <param name="device">The parent device, the layout is created from.</param>
		explicit VulkanPipelineLayout(const VulkanDevice& device) noexcept;

	public:
		/// <summary>
		/// Returns a reference to the device that provides this layout.
		/// </summary>
		/// <returns>A reference to the layouts parent device.</returns>
		virtual const VulkanDevice& device() const noexcept;

		// PipelineLayout interface.
	public:
		/// <inheritdoc />
		virtual const VulkanDescriptorSetLayout& descriptorSet(const UInt32& space) const override;

		/// <inheritdoc />
		virtual Array<const VulkanDescriptorSetLayout*> descriptorSets() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanPushConstantsLayout* pushConstants() const noexcept override;
	};

	/// <summary>
	/// Implements the Vulkan input assembler state.
	/// </summary>
	/// <seealso cref="VulkanInputAssemblerBuilder" />
	class LITEFX_VULKAN_API VulkanInputAssembler : public InputAssembler<VulkanVertexBufferLayout, VulkanIndexBufferLayout> {
		LITEFX_IMPLEMENTATION(VulkanInputAssemblerImpl);
		LITEFX_BUILDER(VulkanInputAssemblerBuilder);

	public:
		/// <summary>
		/// Initializes a new Vulkan input assembler state.
		/// </summary>
		/// <param name="vertexBufferLayouts">The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.</param>
		/// <param name="indexBufferLayout">The index buffer layout.</param>
		/// <param name="primitiveTopology">The primitive topology.</param>
		explicit VulkanInputAssembler(Array<UniquePtr<VulkanVertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<VulkanIndexBufferLayout>&& indexBufferLayout, const PrimitiveTopology& primitiveTopology = PrimitiveTopology::TriangleList);
		VulkanInputAssembler(VulkanInputAssembler&&) noexcept = delete;
		VulkanInputAssembler(const VulkanInputAssembler&) noexcept = delete;
		virtual ~VulkanInputAssembler() noexcept;

	private:
		/// <summary>
		/// Initializes a new Vulkan input assembler state.
		/// </summary>
		explicit VulkanInputAssembler() noexcept;

	public:
		/// <inheritdoc />
		virtual Array<const VulkanVertexBufferLayout*> vertexBufferLayouts() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanVertexBufferLayout& vertexBufferLayout(const UInt32& binding) const override;

		/// <inheritdoc />
		virtual const VulkanIndexBufferLayout& indexBufferLayout() const override;

		/// <inheritdoc />
		virtual const PrimitiveTopology& topology() const noexcept override;
	};

	/// <summary>
	/// Implements a Vulkan <see cref="IRasterizer" />.
	/// </summary>
	/// <seealso cref="VulkanRasterizerBuilder" />
	class LITEFX_VULKAN_API VulkanRasterizer : public Rasterizer {
		LITEFX_BUILDER(VulkanRasterizerBuilder);

	public:
		/// <summary>
		/// Initializes a new Vulkan rasterizer state.
		/// </summary>
		/// <param name="polygonMode">The polygon mode used by the pipeline.</param>
		/// <param name="cullMode">The cull mode used by the pipeline.</param>
		/// <param name="cullOrder">The cull order used by the pipeline.</param>
		/// <param name="lineWidth">The line width used by the pipeline.</param>
		/// <param name="depthStencilState">The rasterizer depth/stencil state.</param>
		explicit VulkanRasterizer(const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth = 1.f, const DepthStencilState& depthStencilState = {}) noexcept;
		VulkanRasterizer(VulkanRasterizer&&) noexcept = delete;
		VulkanRasterizer(const VulkanRasterizer&) noexcept = delete;
		virtual ~VulkanRasterizer() noexcept;

	private:
		/// <summary>
		/// Initializes a new Vulkan rasterizer state.
		/// </summary>
		explicit VulkanRasterizer() noexcept;

	public:
		/// <summary>
		/// Sets the line width on the rasterizer.
		/// </summary>
		/// <remarks>
		/// Note that updating the line width requires the "wide lines" feature to be available. If it is not, the line width **must** be `1.0`. This
		/// constraint is not enforced by the engine and you are responsible of making sure that it is fulfilled.
		/// 
		/// Furthermore, note that the DirectX 12 back-end does have any representation for the line width concept. Thus you should only use the line 
		/// width, if you plan to only support Vulkan.
		/// </remarks>
		/// <returns>A reference to the line width.</returns>
		/// <seealso href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#features-wideLines" />
		virtual void updateLineWidth(const Float& lineWidth) noexcept;
	};

	/// <summary>
	/// Defines the base class for Vulkan pipeline state objects.
	/// </summary>
	/// <seealso cref="VulkanRenderPipeline" />
	/// <seealso cref="VulkanComputePipeline" />
	class LITEFX_VULKAN_API VulkanPipelineState : public virtual Pipeline<VulkanPipelineLayout, VulkanShaderProgram>, public Resource<VkPipeline> {
	public:
		using Resource<VkPipeline>::Resource;
		virtual ~VulkanPipelineState() noexcept = default;

	public:
		/// <summary>
		/// Sets the current pipeline state on the <paramref name="commandBuffer" />.
		/// </summary>
		/// <param name="commandBuffer">The command buffer to set the current pipeline state on.</param>
		virtual void use(const VulkanCommandBuffer& commandBuffer) const noexcept = 0;

		/// <summary>
		/// Binds a descriptor set on a command buffer.
		/// </summary>
		/// <param name="commandBuffer">The command buffer to issue the bind command on.</param>
		/// <param name="descriptorSet">The descriptor set to bind.</param>
		virtual void bind(const VulkanCommandBuffer& commandBuffer, const VulkanDescriptorSet& descriptorSet) const noexcept = 0;
	};

	/// <summary>
	/// Records commands for a <see cref="VulkanCommandQueue" />
	/// </summary>
	/// <seealso cref="VulkanQueue" />
	class LITEFX_VULKAN_API VulkanCommandBuffer : public CommandBuffer<IVulkanBuffer, IVulkanVertexBuffer, IVulkanIndexBuffer, IVulkanImage, VulkanBarrier, VulkanPipelineState>, public Resource<VkCommandBuffer> {
		LITEFX_IMPLEMENTATION(VulkanCommandBufferImpl);

	public:
		/// <summary>
		/// Initializes a command buffer from a command queue.
		/// </summary>
		/// <param name="queue">The parent command queue, the buffer gets submitted to.</param>
		/// <param name="begin">If set to <c>true</c>, the command buffer automatically starts recording by calling <see cref="begin" />.</param>
		/// <param name="primary"><c>true</c>, if the command buffer is a primary command buffer.</param>
		explicit VulkanCommandBuffer(const VulkanQueue& queue, const bool& begin = false, const bool& primary = true);
		VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer(VulkanCommandBuffer&&) = delete;
		virtual ~VulkanCommandBuffer() noexcept;

		// Vulkan Command Buffer interface.
	public:
		/// <summary>
		/// Begins the command buffer as a secondary command buffer that inherits the state of <paramref name="renderPass" />.
		/// </summary>
		/// <param name="renderPass">The render pass state to inherit.</param>
		virtual void begin(const VulkanRenderPass& renderPass) const noexcept;

		// CommandBuffer interface.
	public:
		/// <inheritdoc />
		virtual void begin() const override;

		/// <inheritdoc />
		virtual void end() const override;

		/// <inheritdoc />
		virtual void generateMipMaps(IVulkanImage& image) noexcept override;

		/// <inheritdoc />
		virtual void barrier(const VulkanBarrier& barrier, const bool& invert = false) const noexcept override;

		/// <inheritdoc />
		virtual void transfer(const IVulkanBuffer& source, const IVulkanBuffer& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual void transfer(const IVulkanBuffer& source, const IVulkanImage& target, const UInt32& sourceElement = 0, const UInt32& firstSubresource = 0, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual void transfer(const IVulkanImage& source, const IVulkanImage& target, const UInt32& sourceSubresource = 0, const UInt32& targetSubresource = 0, const UInt32& subresources = 1) const override;

		/// <inheritdoc />
		virtual void transfer(const IVulkanImage& source, const IVulkanBuffer& target, const UInt32& firstSubresource = 0, const UInt32& targetElement = 0, const UInt32& subresources = 1) const override;

		/// <inheritdoc />
		virtual void use(const VulkanPipelineState& pipeline) const noexcept override;

		/// <inheritdoc />
		virtual void bind(const VulkanDescriptorSet& descriptorSet, const VulkanPipelineState& pipeline) const noexcept override;

		/// <inheritdoc />
		virtual void bind(const IVulkanVertexBuffer& buffer) const noexcept override;

		/// <inheritdoc />
		virtual void bind(const IVulkanIndexBuffer& buffer) const noexcept override;

		/// <inheritdoc />
		virtual void dispatch(const Vector3u& threadCount) const noexcept override;

		/// <inheritdoc />
		virtual void draw(const UInt32& vertices, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const noexcept override;

		/// <inheritdoc />
		virtual void drawIndexed(const UInt32& indices, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const noexcept override;

		/// <inheritdoc />
		virtual void pushConstants(const VulkanPushConstantsLayout& layout, const void* const memory) const noexcept override;
	};

	/// <summary>
	/// Implements a Vulkan <see cref="RenderPipeline" />.
	/// </summary>
	/// <seealso cref="VulkanComputePipeline" />
	/// <seealso cref="VulkanRenderPipelineBuilder" />
	class LITEFX_VULKAN_API VulkanRenderPipeline : public RenderPipeline<VulkanPipelineLayout, VulkanShaderProgram, VulkanInputAssembler, VulkanRasterizer>, public VulkanPipelineState {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineImpl);
		LITEFX_BUILDER(VulkanRenderPipelineBuilder);

	public:
		/// <summary>
		/// Initializes a new Vulkan render pipeline.
		/// </summary>
		/// <param name="renderPass">The parent render pass.</param>
		/// <param name="shaderProgram">The shader program used by the pipeline.</param>
		/// <param name="layout">The layout of the pipeline.</param>
		/// <param name="inputAssembler">The input assembler state of the pipeline.</param>
		/// <param name="rasterizer">The rasterizer state of the pipeline.</param>
		/// <param name="name">The optional debug name of the render pipeline.</param>
		/// <param name="enableAlphaToCoverage">Whether or not to enable Alpha-to-Coverage multi-sampling.</param>
		explicit VulkanRenderPipeline(const VulkanRenderPass& renderPass, SharedPtr<VulkanShaderProgram> shaderProgram, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanInputAssembler> inputAssembler, SharedPtr<VulkanRasterizer> rasterizer, Array<SharedPtr<IViewport>> viewports, Array<SharedPtr<IScissor>> scissors, const bool& enableAlphaToCoverage = false, const String& name = "");
		VulkanRenderPipeline(VulkanRenderPipeline&&) noexcept = delete;
		VulkanRenderPipeline(const VulkanRenderPipeline&) noexcept = delete;
		virtual ~VulkanRenderPipeline() noexcept;

	private:
		/// <summary>
		/// Initializes a new Vulkan render pipeline.
		/// </summary>
		/// <param name="renderPass">The parent render pass.</param>
		VulkanRenderPipeline(const VulkanRenderPass& renderPass) noexcept;

		// Pipeline interface.
	public:
		/// <inheritdoc />
		virtual const String& name() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<const VulkanShaderProgram> program() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<const VulkanPipelineLayout> layout() const noexcept override;

		// RenderPipeline interface.
	public:
		/// <inheritdoc />
		virtual SharedPtr<VulkanInputAssembler> inputAssembler() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<VulkanRasterizer> rasterizer() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IViewport*> viewports() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IScissor*> scissors() const noexcept override;

		/// <inheritdoc />
		virtual UInt32& stencilRef() const noexcept override;

		/// <inheritdoc />
		virtual Vector4f& blendFactors() const noexcept override;

		/// <inheritdoc />
		virtual const bool& alphaToCoverage() const noexcept override;

		// VulkanPipelineState interface.
	public:
		/// <inheritdoc />
		virtual void use(const VulkanCommandBuffer& commandBuffer) const noexcept override;

		/// <inheritdoc />
		virtual void bind(const VulkanCommandBuffer& commandBuffer, const VulkanDescriptorSet& descriptorSet) const noexcept override;
	};

	/// <summary>
	/// Implements a Vulkan <see cref="ComputePipeline" />.
	/// </summary>
	/// <seealso cref="VulkanRenderPipeline" />
	/// <seealso cref="VulkanComputePipelineBuilder" />
	class LITEFX_VULKAN_API VulkanComputePipeline : public ComputePipeline<VulkanPipelineLayout, VulkanShaderProgram>, public VulkanPipelineState {
		LITEFX_IMPLEMENTATION(VulkanComputePipelineImpl);
		LITEFX_BUILDER(VulkanComputePipelineBuilder);

	public:
		/// <summary>
		/// Initializes a new Vulkan compute pipeline.
		/// </summary>
		/// <param name="device">The parent device.</param>
		/// <param name="shaderProgram">The shader program used by the pipeline.</param>
		/// <param name="layout">The layout of the pipeline.</param>
		/// <param name="name">The optional debug name of the render pipeline.</param>
		explicit VulkanComputePipeline(const VulkanDevice& device, SharedPtr<VulkanShaderProgram> shaderProgram, SharedPtr<VulkanPipelineLayout> layout, const String& name = "");
		VulkanComputePipeline(VulkanComputePipeline&&) noexcept = delete;
		VulkanComputePipeline(const VulkanComputePipeline&) noexcept = delete;
		virtual ~VulkanComputePipeline() noexcept;

	private:
		/// <summary>
		/// Initializes a new Vulkan compute pipeline.
		/// </summary>
		/// <param name="device">The parent device.</param>
		VulkanComputePipeline(const VulkanDevice& device) noexcept;

		// Pipeline interface.
	public:
		/// <inheritdoc />
		virtual const String& name() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<const VulkanShaderProgram> program() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<const VulkanPipelineLayout> layout() const noexcept override;

		// VulkanPipelineState interface.
	public:
		/// <inheritdoc />
		virtual void use(const VulkanCommandBuffer& commandBuffer) const noexcept override;

		/// <inheritdoc />
		virtual void bind(const VulkanCommandBuffer& commandBuffer, const VulkanDescriptorSet& descriptorSet) const noexcept override;
	};

	/// <summary>
	/// Implements a Vulkan frame buffer.
	/// </summary>
	/// <seealso cref="VulkanRenderPass" />
	class LITEFX_VULKAN_API VulkanFrameBuffer : public FrameBuffer<VulkanCommandBuffer>, public Resource<VkFramebuffer> {
		LITEFX_IMPLEMENTATION(VulkanFrameBufferImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan frame buffer.
		/// </summary>
		/// <param name="renderPass">The parent render pass of the frame buffer.</param>
		/// <param name="bufferIndex">The index of the frame buffer within the parent render pass.</param>
		/// <param name="renderArea">The initial size of the render area.</param>
		/// <param name="commandBuffers">The number of command buffers, the frame buffer stores.</param>
		VulkanFrameBuffer(const VulkanRenderPass& renderPass, const UInt32& bufferIndex, const Size2d& renderArea, const UInt32& commandBuffers = 1);
		VulkanFrameBuffer(const VulkanFrameBuffer&) noexcept = delete;
		VulkanFrameBuffer(VulkanFrameBuffer&&) noexcept = delete;
		virtual ~VulkanFrameBuffer() noexcept;

		// Vulkan frame buffer interface.
	public:
		/// <summary>
		/// Returns a reference of the semaphore, that can be used to signal, that the frame buffer is finished.
		/// </summary>
		/// <returns>A reference of the semaphore, that can be used to signal, that the frame buffer is finished.</returns>
		virtual const VkSemaphore& semaphore() const noexcept;

		/// <summary>
		/// Returns a reference of the last fence value for the frame buffer.
		/// </summary>
		/// <remarks>
		/// The frame buffer must only be re-used, if this fence is reached in the graphics queue.
		/// </remarks>
		/// <returns>A reference of the last fence value for the frame buffer.</returns>
		virtual UInt64& lastFence() const noexcept;

		// FrameBuffer interface.
	public:
		/// <inheritdoc />
		virtual const UInt32& bufferIndex() const noexcept override;

		/// <inheritdoc />
		virtual const Size2d& size() const noexcept override;

		/// <inheritdoc />
		virtual size_t getWidth() const noexcept override;

		/// <inheritdoc />
		virtual size_t getHeight() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanCommandBuffer& commandBuffer(const UInt32& index) const override;

		/// <inheritdoc />
		virtual Array<const VulkanCommandBuffer*> commandBuffers() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IVulkanImage*> images() const noexcept override;

		/// <inheritdoc />
		virtual const IVulkanImage& image(const UInt32& location) const override;

	public:
		/// <inheritdoc />
		virtual void resize(const Size2d& renderArea) override;
	};

	/// <summary>
	/// Implements a Vulkan render pass.
	/// </summary>
	/// <seealso cref="VulkanRenderPassBuilder" />
	class LITEFX_VULKAN_API VulkanRenderPass : public RenderPass<VulkanRenderPipeline, VulkanFrameBuffer, VulkanInputAttachmentMapping>, public Resource<VkRenderPass> {
		LITEFX_IMPLEMENTATION(VulkanRenderPassImpl);
		LITEFX_BUILDER(VulkanRenderPassBuilder);

	public:
		/// <summary>
		/// Creates and initializes a new Vulkan render pass instance.
		/// </summary>
		/// <param name="device">The parent device instance.</param>
		/// <param name="commandBuffers">The number of command buffers in each frame buffer.</param>
		/// <param name="renderTargets">The render targets that are output by the render pass.</param>
		/// <param name="samples">The number of samples for the render targets in this render pass.</param>
		/// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
		explicit VulkanRenderPass(const VulkanDevice& device, Span<RenderTarget> renderTargets, const UInt32& commandBuffers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, Span<VulkanInputAttachmentMapping> inputAttachments = { });

		/// <summary>
		/// Creates and initializes a new Vulkan render pass instance.
		/// </summary>
		/// <param name="device">The parent device instance.</param>
		/// <param name="name">The name of the render pass state resource.</param>
		/// <param name="commandBuffers">The number of command buffers in each frame buffer.</param>
		/// <param name="renderTargets">The render targets that are output by the render pass.</param>
		/// <param name="samples">The number of samples for the render targets in this render pass.</param>
		/// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
		explicit VulkanRenderPass(const VulkanDevice& device, const String& name, Span<RenderTarget> renderTargets, const UInt32& commandBuffers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, Span<VulkanInputAttachmentMapping> inputAttachments = { });
		
		VulkanRenderPass(const VulkanRenderPass&) = delete;
		VulkanRenderPass(VulkanRenderPass&&) = delete;
		virtual ~VulkanRenderPass() noexcept;

	private:
		/// <summary>
		/// Creates an uninitialized Vulkan render pass instance.
		/// </summary>
		/// <remarks>
		/// This constructor is called by the <see cref="VulkanRenderPassBuilder" /> in order to create a render pass instance without initializing it. The instance 
		/// is only initialized after calling <see cref="VulkanRenderPassBuilder::go" />.
		/// </remarks>
		/// <param name="device">The parent device of the render pass.</param>
		/// <param name="name">The name of the render pass state resource.</param>
		explicit VulkanRenderPass(const VulkanDevice& device, const String& name = "") noexcept;

		// IInputAttachmentMappingSource interface.
	public:
		/// <inheritdoc />
		virtual const VulkanFrameBuffer& frameBuffer(const UInt32& buffer) const override;

		// RenderPass interface.
	public:
		/// <summary>
		/// Returns a reference to the device that provides this queue.
		/// </summary>
		/// <returns>A reference to the queue's parent device.</returns>
		virtual const VulkanDevice& device() const noexcept;

		/// <inheritdoc />
		virtual const VulkanFrameBuffer& activeFrameBuffer() const override;

		/// <inheritdoc />
		virtual Array<const VulkanFrameBuffer*> frameBuffers() const noexcept override;

		/// <inheritdoc />
		virtual Array<const VulkanRenderPipeline*> pipelines() const noexcept override;

		/// <inheritdoc />
		virtual const RenderTarget& renderTarget(const UInt32& location) const override;

		/// <inheritdoc />
		virtual Span<const RenderTarget> renderTargets() const noexcept override;

		/// <inheritdoc />
		virtual bool hasPresentTarget() const noexcept override;

		/// <inheritdoc />
		virtual Span<const VulkanInputAttachmentMapping> inputAttachments() const noexcept override;

		/// <inheritdoc />
		virtual const MultiSamplingLevel& multiSamplingLevel() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void begin(const UInt32& buffer) override;
		
		/// <inheritdoc />
		virtual void end() const override;

		/// <inheritdoc />
		virtual void resizeFrameBuffers(const Size2d& renderArea) override;

		/// <inheritdoc />
		virtual void changeMultiSamplingLevel(const MultiSamplingLevel& samples) override;

		/// <inheritdoc />
		virtual void updateAttachments(const VulkanDescriptorSet& descriptorSet) const override;
	};

	/// <summary>
	/// Implements a <see cref="IInputAttachmentMapping" />.
	/// </summary>
	/// <seealso cref="VulkanRenderPass" />
	/// <seealso cref="VulkanRenderPassBuilder" />
	class LITEFX_VULKAN_API VulkanInputAttachmentMapping : public IInputAttachmentMapping<VulkanRenderPass> {
		LITEFX_IMPLEMENTATION(VulkanInputAttachmentMappingImpl);

	public:
		/// <summary>
		/// Creates a new Vulkan input attachment mapping.
		/// </summary>
		VulkanInputAttachmentMapping() noexcept;

		/// <summary>
		/// Creates a new Vulkan input attachment mapping.
		/// </summary>
		/// <param name="renderPass">The render pass to fetch the input attachment from.</param>
		/// <param name="renderTarget">The render target of the <paramref name="renderPass"/> that is used for the input attachment.</param>
		/// <param name="location">The location to bind the input attachment to.</param>
		VulkanInputAttachmentMapping(const VulkanRenderPass& renderPass, const RenderTarget& renderTarget, const UInt32& location);

		/// <summary>
		/// Copies another input attachment mapping.
		/// </summary>
		VulkanInputAttachmentMapping(const VulkanInputAttachmentMapping&) noexcept;

		/// <summary>
		/// Takes over another input attachment mapping.
		/// </summary>
		VulkanInputAttachmentMapping(VulkanInputAttachmentMapping&&) noexcept;

		virtual ~VulkanInputAttachmentMapping() noexcept;

	public:
		/// <summary>
		/// Copies another input attachment mapping.
		/// </summary>
		inline VulkanInputAttachmentMapping& operator=(const VulkanInputAttachmentMapping&) noexcept;

		/// <summary>
		/// Takes over another input attachment mapping.
		/// </summary>
		inline VulkanInputAttachmentMapping& operator=(VulkanInputAttachmentMapping&&) noexcept;

	public:
		/// <inheritdoc />
		virtual const VulkanRenderPass* inputAttachmentSource() const noexcept override;

		/// <inheritdoc />
		virtual const RenderTarget& renderTarget() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& location() const noexcept override;
	};

	/// <summary>
	/// Implements a Vulkan swap chain.
	/// </summary>
	class LITEFX_VULKAN_API VulkanSwapChain : public SwapChain<IVulkanImage>, public Resource<VkSwapchainKHR> {
		LITEFX_IMPLEMENTATION(VulkanSwapChainImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan swap chain.
		/// </summary>
		/// <param name="device">The device that owns the swap chain.</param>
		/// <param name="format">The initial surface format.</param>
		/// <param name="renderArea">The initial size of the render area.</param>
		/// <param name="buffers">The initial number of buffers.</param>
		explicit VulkanSwapChain(const VulkanDevice& device, const Format& surfaceFormat = Format::B8G8R8A8_SRGB, const Size2d& renderArea = { 800, 600 }, const UInt32& buffers = 3);
		VulkanSwapChain(const VulkanSwapChain&) = delete;
		VulkanSwapChain(VulkanSwapChain&&) = delete;
		virtual ~VulkanSwapChain() noexcept;

		// Vulkan Swap Chain interface.
	public:
		/// <summary>
		/// Returns a reference of the current swap semaphore, a command queue can wait on for presenting.
		/// </summary>
		/// <returns>A reference of the current swap semaphore, a command queue can wait on for presenting.</returns>
		virtual const VkSemaphore& semaphore() const noexcept;

		// SwapChain interface.
	public:
		/// <inheritdoc />
		virtual const Format& surfaceFormat() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& buffers() const noexcept override;

		/// <inheritdoc />
		virtual const Size2d& renderArea() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IVulkanImage*> images() const noexcept override;

	public:
		/// <inheritdoc />
		virtual Array<Format> getSurfaceFormats() const noexcept override;

		/// <inheritdoc />
		virtual void reset(const Format& surfaceFormat, const Size2d& renderArea, const UInt32& buffers) override;

		/// <inheritdoc />
		[[nodiscard]] virtual UInt32 swapBackBuffer() const override;
	};

	/// <summary>
	/// Implements a Vulkan command queue.
	/// </summary>
	/// <seealso cref="VulkanCommandBuffer" />
	class LITEFX_VULKAN_API VulkanQueue : public CommandQueue<VulkanCommandBuffer>, public Resource<VkQueue> {
		LITEFX_IMPLEMENTATION(VulkanQueueImpl);
	
	public:
		/// <summary>
		/// Initializes the Vulkan command queue.
		/// </summary>
		/// <param name="device">The device, commands get send to.</param>
		/// <param name="type">The type of the command queue.</param>
		/// <param name="priority">The priority, of which commands are issued on the device.</param>
		/// <param name="familyId">The ID of the queue family.</param>
		/// <param name="queueId">The ID of the queue.</param>
		explicit VulkanQueue(const VulkanDevice& device, const QueueType& type, const QueuePriority& priority, const UInt32& familyId, const UInt32& queueId);
		VulkanQueue(const VulkanQueue&) = delete;
		VulkanQueue(VulkanQueue&&) = delete;
		virtual ~VulkanQueue() noexcept;

		// VulkanQueue interface.
	public:
		/// <summary>
		/// Returns a reference to the device that provides this queue.
		/// </summary>
		/// <returns>A reference to the queue's parent device.</returns>
		virtual const VulkanDevice& device() const noexcept;

		/// <summary>
		/// Returns a reference of the command pool that is used to allocate commands.
		/// </summary>
		/// <remarks>
		/// Note that the command pool does only exist, if the queue is bound on a device.
		/// </remarks>
		/// <seealso cref="isBound" />
		/// <seealso cref="bind" />
		/// <seealso cref="release" />
		/// <returns>A reference of the command pool that is used to allocate commands</returns>
		virtual const VkCommandPool& commandPool() const noexcept;

		/// <summary>
		/// Returns the queue family ID.
		/// </summary>
		/// <returns>The queue family ID.</returns>
		virtual const UInt32& familyId() const noexcept;

		/// <summary>
		/// Returns the queue ID.
		/// </summary>
		/// <returns>The queue ID.</returns>
		virtual const UInt32& queueId() const noexcept;

		/// <summary>
		/// Returns the internal timeline semaphore used to synchronize the queue execution.
		/// </summary>
		/// <returns>The internal timeline semaphore.</returns>
		virtual const VkSemaphore& timelineSemaphore() const noexcept;

		/// <summary>
		/// Submits a single command buffer and inserts a fence to wait for it.
		/// </summary>
		/// <remarks>
		/// Note that submitting a command buffer that is currently recording will implicitly close the command buffer.
		/// </remarks>
		/// <param name="commandBuffer">The command buffer to submit to the command queue.</param>
		/// <param name="waitForSemaphores">The semaphores to wait for on each pipeline stage. There must be a semaphore for each entry in the <see cref="waitForStages" /> array.</param>
		/// <param name="waitForStages">The pipeline stages of the current render pass to wait for before submitting the command buffer.</param>
		/// <param name="signalSemaphores">The semaphores to signal, when the command buffer is executed.</param>
		/// <returns>The value of the fence, inserted after the command buffer.</returns>
		/// <seealso cref="waitFor" />
		virtual UInt64 submit(const VulkanCommandBuffer& commandBuffer, Span<VkSemaphore> waitForSemaphores, Span<VkPipelineStageFlags> waitForStages, Span<VkSemaphore> signalSemaphores = { }) const;

		/// <summary>
		/// Submits a set of command buffers and inserts a fence to wait for them.
		/// </summary>
		/// <remarks>
		/// Note that submitting a command buffer that is currently recording will implicitly close the command buffer.
		/// </remarks>
		/// <param name="commandBuffers">The command buffers to submit to the command queue.</param>
		/// <param name="waitForSemaphores">The semaphores to wait for on each pipeline stage. There must be a semaphore for each entry in the <see cref="waitForStages" /> array.</param>
		/// <param name="waitForStages">The pipeline stages of the current render pass to wait for before submitting the command buffer.</param>
		/// <param name="signalSemaphores">The semaphores to signal, when the command buffer is executed.</param>
		/// <returns>The value of the fence, inserted after the command buffers.</returns>
		/// <seealso cref="waitFor" />
		virtual UInt64 submit(const Array<const VulkanCommandBuffer*>& commandBuffers, Span<VkSemaphore> waitForSemaphores, Span<VkPipelineStageFlags> waitForStages, Span<VkSemaphore> signalSemaphores = { }) const;

		/// <summary>
		/// Creates a command buffer that can be used to allocate commands on the queue.
		/// </summary>
		/// <param name="secondary">If set to <c>true</c>, the queue will create a secondary command buffer instance.</param>
		/// <param name="beginRecording">If set to <c>true</c>, the command buffer will be initialized in recording state and can receive commands straight away.</param>
		/// <returns>The instance of the command buffer.</returns>
		virtual UniquePtr<VulkanCommandBuffer> createCommandBuffer(const bool& secondary, const bool& beginRecording) const;

		// CommandQueue interface.
	public:
		/// <inheritdoc />
		virtual bool isBound() const noexcept override;

		/// <inheritdoc />
		virtual const QueuePriority& priority() const noexcept override;

		/// <inheritdoc />
		virtual const QueueType& type() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void bind() override;

		/// <inheritdoc />
		virtual void release() override;

		/// <inheritdoc />
		virtual UniquePtr<VulkanCommandBuffer> createCommandBuffer(const bool& beginRecording = false) const override;

		/// <inheritdoc />
		virtual UInt64 submit(const VulkanCommandBuffer& commandBuffer) const override;

		/// <inheritdoc />
		virtual UInt64 submit(const Array<const VulkanCommandBuffer*>& commandBuffers) const override;

		/// <inheritdoc />
		virtual void waitFor(const UInt64& fence) const noexcept override;

		/// <inheritdoc />
		virtual UInt64 currentFence() const noexcept override;
	};

	/// <summary>
	/// A graphics factory that produces objects for a <see cref="VulkanDevice" />.
	/// </summary>
	/// <remarks>
	/// Internally this factory implementation is based on <a href="https://gpuopen.com/vulkan-memory-allocator/" target="_blank">Vulkan Memory Allocator</a>.
	/// </remarks>
	class LITEFX_VULKAN_API VulkanGraphicsFactory : public GraphicsFactory<VulkanDescriptorLayout, IVulkanBuffer, IVulkanVertexBuffer, IVulkanIndexBuffer, IVulkanImage, IVulkanSampler> {
		LITEFX_IMPLEMENTATION(VulkanGraphicsFactoryImpl);

	public:
		/// <summary>
		/// Creates a new graphics factory.
		/// </summary>
		/// <param name="device">The device the factory should produce objects for.</param>
		explicit VulkanGraphicsFactory(const VulkanDevice& device);
		VulkanGraphicsFactory(const VulkanGraphicsFactory&) = delete;
		VulkanGraphicsFactory(VulkanGraphicsFactory&&) = delete;
		virtual ~VulkanGraphicsFactory() noexcept;

	public:
		/// <inheritdoc />
		virtual UniquePtr<IVulkanBuffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements = 1, const bool& allowWrite = false) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanVertexBuffer> createVertexBuffer(const VulkanVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanIndexBuffer> createIndexBuffer(const VulkanIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanImage> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanImage> createTexture(const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& levels = 1, const UInt32& layers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const override;

		/// <inheritdoc />
		virtual Array<UniquePtr<IVulkanImage>> createTextures(const UInt32& elements, const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& levels = 1, const UInt32& layers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanSampler> createSampler(const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const override;

		/// <inheritdoc />
		virtual Array<UniquePtr<IVulkanSampler>> createSamplers(const UInt32& elements, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const override;
	};

	/// <summary>
	/// Implements a Vulkan graphics device.
	/// </summary>
	class LITEFX_VULKAN_API VulkanDevice : public GraphicsDevice<VulkanGraphicsFactory, VulkanSurface, VulkanGraphicsAdapter, VulkanSwapChain, VulkanQueue, VulkanRenderPass, VulkanComputePipeline>, public Resource<VkDevice> {
		LITEFX_IMPLEMENTATION(VulkanDeviceImpl);

	public:
		/// <summary>
		/// Creates a new device instance.
		/// </summary>
		/// <param name="backend">The backend from which the device is created.</param>
		/// <param name="adapter">The adapter the device uses for drawing.</param>
		/// <param name="surface">The surface, the device should draw to.</param>
		/// <param name="extensions">The required extensions the device gets initialized with.</param>
		explicit VulkanDevice(const VulkanBackend& backend, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, Span<String> extensions = { });

		/// <summary>
		/// Creates a new device instance.
		/// </summary>
		/// <param name="backend">The backend from which the device is created.</param>
		/// <param name="adapter">The adapter the device uses for drawing.</param>
		/// <param name="surface">The surface, the device should draw to.</param>
		/// <param name="format">The initial surface format, device uses for drawing.</param>
		/// <param name="frameBufferSize">The initial size of the frame buffers.</param>
		/// <param name="frameBuffers">The initial number of frame buffers.</param>
		/// <param name="extensions">The required extensions the device gets initialized with.</param>
		explicit VulkanDevice(const VulkanBackend& backend, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers, Span<String> extensions = { });

		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		virtual ~VulkanDevice() noexcept;

		// Vulkan Device interface.
	public:
		/// <summary>
		/// Returns the array that stores the extensions that were used to initialize the device.
		/// </summary>
		/// <returns>A reference to the array that stores the extensions that were used to initialize the device.</returns>
		virtual Span<const String> enabledExtensions() const noexcept;

		/// <summary>
		/// Returns a reference of the swap chain.
		/// </summary>
		/// <returns>A reference of the swap chain.</returns>
		virtual VulkanSwapChain& swapChain() noexcept;

		// GraphicsDevice interface.
	public:
		/// <inheritdoc />
		virtual DeviceState& state() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanSwapChain& swapChain() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanSurface& surface() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanGraphicsAdapter& adapter() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanGraphicsFactory& factory() const noexcept override;
		
		/// <inheritdoc />
		virtual const VulkanQueue& graphicsQueue() const noexcept override;
		
		/// <inheritdoc />
		virtual const VulkanQueue& transferQueue() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanQueue& bufferQueue() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanQueue& computeQueue() const noexcept override;

		/// <inheritdoc />
		virtual MultiSamplingLevel maximumMultiSamplingLevel(const Format& format) const noexcept override;

	public:
		/// <inheritdoc />
		virtual void wait() const override;

#if defined(BUILD_DEFINE_BUILDERS)
	public:
		/// <inheritdoc />
		[[nodiscard]] virtual VulkanRenderPassBuilder buildRenderPass(const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const UInt32& commandBuffers = 1) const override;

		/// <inheritdoc />
		[[nodiscard]] virtual VulkanRenderPassBuilder buildRenderPass(const String& name, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const UInt32& commandBuffers = 1) const override;

		/// <inheritdoc />
		//[[nodiscard]] virtual VulkanRenderPipelineBuilder buildRenderPipeline(const String& name) const override;

		/// <inheritdoc />
		[[nodiscard]] virtual VulkanRenderPipelineBuilder buildRenderPipeline(const VulkanRenderPass& renderPass, const String& name) const override;

		/// <inheritdoc />
		[[nodiscard]] virtual VulkanComputePipelineBuilder buildComputePipeline(const String& name) const override;
		
		/// <inheritdoc />
		[[nodiscard]] virtual VulkanPipelineLayoutBuilder buildPipelineLayout() const override;

		/// <inheritdoc />
		[[nodiscard]] virtual VulkanInputAssemblerBuilder buildInputAssembler() const override;

		/// <inheritdoc />
		[[nodiscard]] virtual VulkanRasterizerBuilder buildRasterizer() const override;

		/// <inheritdoc />
		[[nodiscard]] virtual VulkanShaderProgramBuilder buildShaderProgram() const override;
#endif // defined(BUILD_DEFINE_BUILDERS)
	};

	/// <summary>
	/// Defines a rendering backend that creates a Vulkan device.
	/// </summary>
	class LITEFX_VULKAN_API VulkanBackend : public RenderBackend<VulkanBackend, VulkanDevice>, public Resource<VkInstance> {
		LITEFX_IMPLEMENTATION(VulkanBackendImpl);

	public:
		/// <summary>
		/// Initializes a new vulkan rendering backend.
		/// </summary>
		/// <param name="app">An instance of the app that owns the backend.</param>
		/// <param name="extensions">A set of instance extensions to enable on the backend instance.</param>
		/// <param name="validationLayers">A set of validation layers to enable on the rendering backend.</param>
		explicit VulkanBackend(const App& app, const Span<String> extensions = { }, const Span<String> validationLayers = { });
		VulkanBackend(const VulkanBackend&) noexcept = delete;
		VulkanBackend(VulkanBackend&&) noexcept = delete;
		virtual ~VulkanBackend() noexcept;

		// Vulkan Backend interface.
	public:
		/// <summary>
		/// A callback that creates a surface from a Vulkan instance.
		/// </summary>
		typedef std::function<VkSurfaceKHR(const VkInstance&)> surface_callback;

	public:
		/// <summary>
		/// Returns the validation layers that are enabled on the backend.
		/// </summary>
		/// <returns>An array of validation layers that are enabled on the backend.</returns>
		virtual Span<const String> getEnabledValidationLayers() const noexcept;

		/// <summary>
		/// Creates a surface using the <paramref name="predicate" /> callback.
		/// </summary>
		/// <param name="predicate">A callback that gets called with the backend instance handle and creates the surface instance</param>
		/// <returns>The instance of the created surface.</returns>
		/// <seealso cref="surface_callback" />
		UniquePtr<VulkanSurface> createSurface(surface_callback predicate) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
		/// <summary>
		/// Creates a surface on a window handle.
		/// </summary>
		/// <param name="hwnd">The window handle on which the surface should be created.</param>
		/// <returns>The instance of the created surface.</returns>
		UniquePtr<VulkanSurface> createSurface(const HWND& hwnd) const;
#endif

	public:
		/// <summary>
		/// Returns <c>true</c>, if all elements of <paramref cref="extensions" /> are contained by the a list of available extensions.
		/// </summary>
		/// <returns><c>true</c>, if all elements of <paramref cref="extensions" /> are contained by the a list of available extensions.</returns>
		/// <seealso cref="getAvailableInstanceExtensions" />
		static bool validateInstanceExtensions(Span<const String> extensions) noexcept;

		/// <summary>
		/// Returns a list of available extensions.
		/// </summary>
		/// <returns>A list of available extensions.</returns>
		/// <seealso cref="validateInstanceExtensions" />
		static Array<String> getAvailableInstanceExtensions() noexcept;

		/// <summary>
		/// Returns <c>true</c>, if all elements of <paramref cref="validationLayers" /> are contained by the a list of available validation layers.
		/// </summary>
		/// <returns><c>true</c>, if all elements of <paramref cref="validationLayers" /> are contained by the a list of available validation layers.</returns>
		/// <seealso cref="getInstanceValidationLayers" />
		static bool validateInstanceLayers(const Span<const String> validationLayers) noexcept;

		/// <summary>
		/// Returns a list of available validation layers.
		/// </summary>
		/// <returns>A list of available validation layers.</returns>
		/// <seealso cref="validateInstanceLayers" />
		static Array<String> getInstanceValidationLayers() noexcept;

		// IBackend interface.
	public:
		/// <inheritdoc />
		virtual BackendType type() const noexcept override;

	protected:
		/// <inheritdoc />
		virtual void activate() override;

		/// <inheritdoc />
		virtual void deactivate() override;

		// RenderBackend interface.
	public:
		/// <inheritdoc />
		virtual Array<const VulkanGraphicsAdapter*> listAdapters() const override;

		/// <inheritdoc />
		virtual const VulkanGraphicsAdapter* findAdapter(const Optional<UInt32>& adapterId = std::nullopt) const override;

		/// <inheritdoc />
		virtual void registerDevice(String name, UniquePtr<VulkanDevice>&& device) override;

		/// <inheritdoc />
		virtual void releaseDevice(const String& name) override;

		/// <inheritdoc />
		virtual VulkanDevice* device(const String& name) noexcept override;

		/// <inheritdoc />
		virtual const VulkanDevice* device(const String& name) const noexcept override;
	};
}