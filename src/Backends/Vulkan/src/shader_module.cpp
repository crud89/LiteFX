#include <litefx/backends/vulkan.hpp>
#include <fstream>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanShaderModule::VulkanShaderModuleImpl {
public:
	friend class VulkanShaderModule;

private:
	const VulkanDevice& m_device;
	ShaderStage m_type;
	String m_fileName, m_entryPoint, m_bytecode;
	Optional<DescriptorBindingPoint> m_shaderLocalDescriptor;

public:
	VulkanShaderModuleImpl(VulkanShaderModule* parent, const VulkanDevice& device, ShaderStage type, const String& fileName, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor) :
		m_device(device), m_type(type), m_fileName(fileName), m_entryPoint(entryPoint), m_shaderLocalDescriptor(shaderLocalDescriptor)
	{
	}

private:
	String readFileContents(const String& fileName) 
	{
		std::ifstream file(fileName, std::ios::in | std::ios::binary);
		
		if (!file.is_open())
			throw std::runtime_error("Unable to open shader file.");

		return this->readStreamContents(file);
	}

	String readStreamContents(std::istream& stream)
	{
		return String(std::istreambuf_iterator<char>(stream), {});
	}

public:
	VkShaderModule initialize()
	{
		return this->initialize(this->readFileContents(m_fileName));
	}

	VkShaderModule initialize(std::istream& stream)
	{
		return this->initialize(this->readStreamContents(stream));
	}

	VkShaderModule initialize(String fileContents)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = fileContents.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(fileContents.c_str()); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

		VkShaderModule module{};

		if (::vkCreateShaderModule(m_device.handle(), &createInfo, nullptr, &module) != VK_SUCCESS)
			throw std::runtime_error("Unable to compile shader file.");

#ifndef NDEBUG
		m_device.setDebugName(*reinterpret_cast<const UInt64*>(&module), VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, std::format("{0}: {1}", m_fileName, m_entryPoint));
#endif

		m_bytecode = fileContents;
		return module;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderModule::VulkanShaderModule(const VulkanDevice& device, ShaderStage type, const String& fileName, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor) :
	Resource<VkShaderModule>(VK_NULL_HANDLE), m_impl(device, type, fileName, entryPoint, shaderLocalDescriptor)
{
	this->handle() = m_impl->initialize();
}

VulkanShaderModule::VulkanShaderModule(const VulkanDevice& device, ShaderStage type, std::istream& stream, const String& name, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor) :
	Resource<VkShaderModule>(VK_NULL_HANDLE), m_impl(device, type, name, entryPoint, shaderLocalDescriptor)
{
	this->handle() = m_impl->initialize(stream);
}

VulkanShaderModule::VulkanShaderModule(VulkanShaderModule&&) noexcept = default;
VulkanShaderModule& VulkanShaderModule::operator=(VulkanShaderModule&&) noexcept = default;
VulkanShaderModule::~VulkanShaderModule() noexcept
{
	::vkDestroyShaderModule(m_impl->m_device.handle(), this->handle(), nullptr);
}

ShaderStage VulkanShaderModule::type() const noexcept
{
	return m_impl->m_type;
}

const String& VulkanShaderModule::fileName() const noexcept
{
	return m_impl->m_fileName;
}

const String& VulkanShaderModule::entryPoint() const noexcept
{
	return m_impl->m_entryPoint;
}

const String& VulkanShaderModule::bytecode() const noexcept
{
	return m_impl->m_bytecode;
}

VkPipelineShaderStageCreateInfo VulkanShaderModule::shaderStageDefinition() const
{
	VkPipelineShaderStageCreateInfo shaderStageDefinition = {};
	shaderStageDefinition.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageDefinition.module = this->handle();
	shaderStageDefinition.pName = this->entryPoint().c_str();
	shaderStageDefinition.stage = Vk::getShaderStage(this->type());

	return shaderStageDefinition;
}

const Optional<DescriptorBindingPoint>& VulkanShaderModule::shaderLocalDescriptor() const noexcept 
{
	return m_impl->m_shaderLocalDescriptor;
}