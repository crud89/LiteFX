#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

extern PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelines;
extern PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandles;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRayTracingPipeline::VulkanRayTracingPipelineImpl : public Implement<VulkanRayTracingPipeline> {
public:
	friend class VulkanRayTracingPipelineBuilder;
	friend class VulkanRayTracingPipeline;

private:
	SharedPtr<VulkanPipelineLayout> m_layout;
	SharedPtr<const VulkanShaderProgram> m_program;
	const ShaderRecordCollection m_shaderRecordCollection;
	UInt32 m_maxRecursionDepth { 10 };
	const VulkanDevice& m_device;

public:
	VulkanRayTracingPipelineImpl(VulkanRayTracingPipeline* parent, const VulkanDevice& device, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanShaderProgram> shaderProgram, UInt32 maxRecursionDepth, ShaderRecordCollection&& shaderRecords) :
		base(parent), m_device(device), m_layout(layout), m_program(shaderProgram), m_shaderRecordCollection(std::move(shaderRecords)), m_maxRecursionDepth(maxRecursionDepth)
	{
	}

	VulkanRayTracingPipelineImpl(VulkanRayTracingPipeline* parent, const VulkanDevice& device, ShaderRecordCollection&& shaderRecords) :
		base(parent), m_device(device), m_shaderRecordCollection(std::move(shaderRecords))
	{
		m_program = std::dynamic_pointer_cast<const VulkanShaderProgram>(m_shaderRecordCollection.program());
	}

public:
	VkPipeline initialize()
	{
		if (m_program == nullptr) [[unlikely]]
			throw ArgumentNotInitializedException("shaderProgram", "The shader program must be initialized.");
		if (m_layout == nullptr) [[unlikely]]
			throw ArgumentNotInitializedException("layout", "The pipeline layout must be initialized.");
		if (m_program != m_shaderRecordCollection.program()) [[unlikely]]
			throw InvalidArgumentException("shaderRecords", "The ray tracing pipeline shader program must be the same as used to build the shader record collection.");

		LITEFX_TRACE(VULKAN_LOG, "Creating ray-tracing pipeline (\"{1}\") for layout {0} (records: {2})...", fmt::ptr(reinterpret_cast<void*>(m_layout.get())), m_parent->name(), m_shaderRecordCollection.shaderRecords().size());
	
		// Validate shader stage usage.
		auto modules = m_program->modules() | std::ranges::to<std::vector>();
		bool hasComputeShaders    = std::ranges::find_if(modules, [](const auto& module) { return LITEFX_FLAG_IS_SET(ShaderStage::Compute, module->type()); }) != modules.end();
		bool hasRayTracingShaders = std::ranges::find_if(modules, [](const auto& module) { return LITEFX_FLAG_IS_SET(ShaderStage::RayTracingPipeline, module->type()); }) != modules.end();
		bool hasMeshShaders       = std::ranges::find_if(modules, [](const auto& module) { return LITEFX_FLAG_IS_SET(ShaderStage::MeshPipeline, module->type()); }) != modules.end();
		bool hasDirectShaders     = std::ranges::find_if(modules, [](const auto& module) { return LITEFX_FLAG_IS_SET(ShaderStage::RasterizationPipeline, module->type()); }) != modules.end();

		if (hasComputeShaders) [[unlikely]]
			throw InvalidArgumentException("shaderProgram", "The shader program contains a compute shader, which is not supported in a ray-tracing pipeline.");
		else if (hasDirectShaders) [[unlikely]]
			throw InvalidArgumentException("shaderProgram", "The shader program contains a graphics shader, which is not supported in a ray-tracing pipeline");
		else if (hasMeshShaders) [[unlikely]]
			throw InvalidArgumentException("shaderProgram", "The shader program contains a mesh shader, which is not supported in a ray-tracing pipeline");

		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(reinterpret_cast<const void*>(m_program.get())), modules.size());

		Array<VkPipelineShaderStageCreateInfo> shaderStages = modules |
			std::views::transform([](const VulkanShaderModule* shaderModule) { return shaderModule->shaderStageDefinition(); }) |
			std::ranges::to<Array<VkPipelineShaderStageCreateInfo>>();

		// Associate each shader module with an index for faster lookup when building the shader binding table (SBT).
		auto moduleIds = modules | 
			std::views::enumerate | 
			std::views::transform([](const auto& tuple) { return std::make_tuple(static_cast<const IShaderModule*>(std::get<1>(tuple)), std::get<0>(tuple)); }) | 
			std::ranges::to<std::map>();

		// Create an array of shader group records.
		auto shaderGroups = m_shaderRecordCollection.shaderRecords() | std::views::transform([&moduleIds](auto& record) {
			VkRayTracingShaderGroupCreateInfoKHR group = { .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };

			switch (record->type())
			{
			case ShaderRecordType::RayGeneration:
			case ShaderRecordType::Miss:
			case ShaderRecordType::Callable:
				group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
				group.generalShader = moduleIds.at(std::get<const IShaderModule*>(record->shaderGroup()));
				group.anyHitShader = group.closestHitShader = group.intersectionShader = VK_SHADER_UNUSED_KHR;
				break;
			case ShaderRecordType::Intersection:
				group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
				group.intersectionShader = moduleIds.at(std::get<const IShaderModule*>(record->shaderGroup()));
				group.anyHitShader = group.closestHitShader = group.generalShader = VK_SHADER_UNUSED_KHR;
				break;
			case ShaderRecordType::HitGroup:
			{
				const auto& hitGroup = std::get<IShaderRecord::MeshGeometryHitGroup>(record->shaderGroup());
				group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
				group.closestHitShader = hitGroup.ClosestHitShader != nullptr ? moduleIds.at(hitGroup.ClosestHitShader) : VK_SHADER_UNUSED_KHR;
				group.anyHitShader = hitGroup.AnyHitShader != nullptr ? moduleIds.at(hitGroup.AnyHitShader) : VK_SHADER_UNUSED_KHR;
				group.intersectionShader = group.generalShader = VK_SHADER_UNUSED_KHR;
				break;
			}
			default: throw InvalidArgumentException("shaderRecords", "At least one record in the shader record collection is not a valid ray-tracing shader.");
			}

			return group;
		}) | std::ranges::to<Array<VkRayTracingShaderGroupCreateInfoKHR>>();

		// Setup dynamic state.
		Array<VkDynamicState> dynamicStates { 
			VkDynamicState::VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<UInt32>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		// Setup pipeline.
		VkRayTracingPipelineCreateInfoKHR pipelineInfo = {
			.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
			.stageCount = static_cast<UInt32>(shaderStages.size()),
			.pStages = shaderStages.data(),
			.groupCount = static_cast<UInt32>(shaderGroups.size()),
			.pGroups = shaderGroups.data(),
			.maxPipelineRayRecursionDepth = m_maxRecursionDepth,
			.pDynamicState = &dynamicState,
			.layout = std::as_const(*m_layout.get()).handle()
		};

		VkPipeline pipeline;
		raiseIfFailed(::vkCreateRayTracingPipelines(m_device.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create render pipeline.");

#ifndef NDEBUG
		m_device.setDebugName(*reinterpret_cast<const UInt64*>(&pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, m_parent->name());
#endif

		return pipeline;
	}

	UniquePtr<IVulkanBuffer> allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups)
	{
		// NOTE: It is assumed that the shader record collection did not change between pipeline creation and SBT allocation (hence its const-ness)!
		offsets = { };
		
		// Get the physical device properties, as they dictate alignment rules.
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProperties { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
		VkPhysicalDeviceProperties2 deviceProperties { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &rayTracingProperties };
		::vkGetPhysicalDeviceProperties2(m_device.adapter().handle(), &deviceProperties);

		// Find the maximum payload size amongst the included shader records.
		auto filterByGroupType = [groups](auto& record) -> bool {
			switch (record->type())
			{
			case ShaderRecordType::RayGeneration: return LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::RayGeneration);
			case ShaderRecordType::Miss:          return LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::Miss);
			case ShaderRecordType::Callable:      return LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::Callable);
			case ShaderRecordType::Intersection:
			case ShaderRecordType::HitGroup:      return LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::HitGroup);
			default: std::unreachable(); // Must be caught during pipeline creation!
			}
		};

		auto localDataSize = std::ranges::max(m_shaderRecordCollection.shaderRecords() | std::views::filter(filterByGroupType) | std::views::transform([](auto& record) { return record->localDataSize(); }));

		// Compute the record size by aligning the handle and payload sizes.
		auto recordSize = Math::align<UInt64>(rayTracingProperties.shaderGroupHandleSize + localDataSize, rayTracingProperties.shaderGroupBaseAlignment);

		// Count the shader records that go into the SBT.
		auto totalRecordCount = std::ranges::distance(m_shaderRecordCollection.shaderRecords() | std::views::filter(filterByGroupType));

		// Map the records to their indices.
		auto shaderRecordIds = m_shaderRecordCollection.shaderRecords() | std::views::enumerate | 
			std::views::transform([](const auto& tuple) { return std::make_tuple(std::get<1>(tuple).get(), std::get<0>(tuple)); }) | 
			std::ranges::to<std::map>();

		// Allocate a buffer for the shader binding table.
		// NOTE: Updating the SBT to change payloads is currently unsupported. Instead, bind-less resources should be used.
		auto result = m_device.factory().createBuffer(BufferType::ShaderBindingTable, ResourceHeap::Dynamic, recordSize, totalRecordCount, ResourceUsage::TransferSource);

		// Write each record group by group.
		UInt32 record{ 0 };
		Array<Byte> recordData(recordSize, 0x00);

		// Write each shader binding group that should be included.
		for (auto group : { ShaderBindingGroup::RayGeneration, ShaderBindingGroup::Miss, ShaderBindingGroup::Callable, ShaderBindingGroup::HitGroup })
		{
			auto groupFilter = [group](auto& record) -> bool { 
				switch (group)
				{
				case ShaderBindingGroup::RayGeneration: return record->type() == ShaderRecordType::RayGeneration;
				case ShaderBindingGroup::Miss:          return record->type() == ShaderRecordType::Miss;
				case ShaderBindingGroup::Callable:      return record->type() == ShaderRecordType::Callable;
				case ShaderBindingGroup::HitGroup:      return record->type() == ShaderRecordType::HitGroup || record->type() == ShaderRecordType::Intersection;
				default: std::unreachable(); // Same as above.
				}
			};

			if (LITEFX_FLAG_IS_SET(groups, group))
			{
				// Get the number of shaders in the group.
				auto filteredRecords = m_shaderRecordCollection.shaderRecords() | std::views::filter(groupFilter);
				auto recordCount = std::ranges::distance(filteredRecords);

				// Store the group offset and size.
				switch (group)
				{
				case ShaderBindingGroup::RayGeneration:
					offsets.RayGenerationGroupOffset = record * recordSize;
					offsets.RayGenerationGroupSize = recordCount * recordSize;
					offsets.RayGenerationGroupStride = recordSize;
					break;
				case ShaderBindingGroup::Miss:
					offsets.MissGroupOffset = record * recordSize;
					offsets.MissGroupSize = recordCount * recordSize;
					offsets.MissGroupStride = recordSize;
					break;
				case ShaderBindingGroup::Callable:
					offsets.CallableGroupOffset = record * recordSize;
					offsets.CallableGroupSize = recordCount * recordSize;
					offsets.CallableGroupStride = recordSize;
					break;
				case ShaderBindingGroup::HitGroup:
					offsets.HitGroupOffset = record * recordSize;
					offsets.HitGroupSize = recordCount * recordSize;
					offsets.HitGroupStride = recordSize;
					break;
				default:
					std::unreachable();
				}

				// Write each record and its payload into the buffer.
				for (auto& currentRecord : filteredRecords)
				{
					// Get the shader group handle for the current record.
					auto id = shaderRecordIds.at(currentRecord.get());
					raiseIfFailed(::vkGetRayTracingShaderGroupHandles(m_device.handle(), m_parent->handle(), id, 1, rayTracingProperties.shaderGroupHandleSize, recordData.data()), "Unable to query shader record handle.");

					// Write the payload and map everything into the buffer.
					std::memcpy(recordData.data() + rayTracingProperties.shaderGroupHandleSize, currentRecord->localData(), currentRecord->localDataSize());
					result->map(recordData.data(), recordSize, record++);
				}
			}
		}

		return result;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRayTracingPipeline::VulkanRayTracingPipeline(const VulkanDevice& device, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanShaderProgram> shaderProgram, ShaderRecordCollection&& shaderRecords, UInt32 maxRecursionDepth, const String& name) :
	m_impl(makePimpl<VulkanRayTracingPipelineImpl>(this, device, layout, shaderProgram, maxRecursionDepth, std::move(shaderRecords))), VulkanPipelineState(VK_NULL_HANDLE)
{
	if (!name.empty())
		this->name() = name;

	this->handle() = m_impl->initialize();
}

VulkanRayTracingPipeline::VulkanRayTracingPipeline(const VulkanDevice& device, ShaderRecordCollection&& shaderRecords) noexcept :
	m_impl(makePimpl<VulkanRayTracingPipelineImpl>(this, device, std::move(shaderRecords))), VulkanPipelineState(VK_NULL_HANDLE)
{
}

VulkanRayTracingPipeline::~VulkanRayTracingPipeline() noexcept
{
	::vkDestroyPipeline(m_impl->m_device.handle(), this->handle(), nullptr);
}

SharedPtr<const VulkanShaderProgram> VulkanRayTracingPipeline::program() const noexcept
{
	return m_impl->m_program;
}

SharedPtr<const VulkanPipelineLayout> VulkanRayTracingPipeline::layout() const noexcept 
{
	return m_impl->m_layout;
}

const ShaderRecordCollection& VulkanRayTracingPipeline::shaderRecords() const noexcept
{
	return m_impl->m_shaderRecordCollection;
}

UInt32 VulkanRayTracingPipeline::maxRecursionDepth() const noexcept
{
	return m_impl->m_maxRecursionDepth;
}

UniquePtr<IVulkanBuffer> VulkanRayTracingPipeline::allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups) const noexcept
{
	return m_impl->allocateShaderBindingTable(offsets, groups);
}

void VulkanRayTracingPipeline::use(const VulkanCommandBuffer& commandBuffer) const noexcept
{
	::vkCmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, this->handle());
}

void VulkanRayTracingPipeline::bind(const VulkanCommandBuffer& commandBuffer, const VulkanDescriptorSet& descriptorSet) const noexcept
{
	// DescriptorSet -> DescriptorSetLayout
	::vkCmdBindDescriptorSets(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, std::as_const(*m_impl->m_layout).handle(), descriptorSet.layout().space(), 1, &descriptorSet.handle(), 0, nullptr);
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

constexpr VulkanRayTracingPipelineBuilder::VulkanRayTracingPipelineBuilder(const VulkanDevice& device, ShaderRecordCollection&& shaderRecords, const String& name) :
	RayTracingPipelineBuilder(UniquePtr<VulkanRayTracingPipeline>(new VulkanRayTracingPipeline(device, std::move(shaderRecords))))
{
	this->instance()->name() = name;
}

constexpr VulkanRayTracingPipelineBuilder::~VulkanRayTracingPipelineBuilder() noexcept = default;

void VulkanRayTracingPipelineBuilder::build()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = m_state.pipelineLayout;
	instance->m_impl->m_maxRecursionDepth = m_state.maxRecursionDepth;
	instance->handle() = instance->m_impl->initialize();
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)