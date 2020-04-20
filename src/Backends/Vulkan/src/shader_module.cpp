#include <litefx/backends/vulkan.hpp>
#include <sstream>
#include <fstream>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanShaderModule::VulkanShaderModuleImpl {
private:
	const VulkanDevice* m_device;
	ShaderType m_type;
	String m_fileName, m_entryPoint;

public:
	VulkanShaderModuleImpl(const VulkanDevice* device, const ShaderType& type, const String& fileName, const String& entryPoint) noexcept :
		m_device(device), m_fileName(fileName), m_entryPoint(entryPoint), m_type(type) { }

private:
	String readFileContents(const String& fileName) {
		std::ifstream file(m_fileName, std::ios::in | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("Unable to open shader file.");

		std::stringstream buffer;
		buffer << file.rdbuf();

		return buffer.str();
	}

public:
	VkShaderModule initialize(const VulkanShaderModule& parent)
	{
		String fileContents = this->readFileContents(m_fileName);

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = fileContents.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(fileContents.c_str());

		VkShaderModule module;

		if (vkCreateShaderModule(m_device->handle(), &createInfo, nullptr, &module) != VK_SUCCESS)
			throw std::runtime_error("Unable to compile shader file.");

		return module;
	}

public:
	const IGraphicsDevice* getDevice() const noexcept 
	{
		return m_device;
	}

	const ShaderType& getType() const noexcept
	{
		return m_type;
	}

	const String& getFileName() const noexcept
	{
		return m_fileName;
	}

	const String& getEntryPoint() const noexcept
	{
		return m_entryPoint;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderModule::VulkanShaderModule(const VulkanDevice* device, const ShaderType& type, const String& fileName, const String& entryPoint) :
	IResource(nullptr), m_impl(makePimpl<VulkanShaderModuleImpl>(device, type, fileName, entryPoint))
{
	if (device == nullptr)
		throw std::invalid_argument("The argument `device` must be initialized.");

	this->handle() = m_impl->initialize(*this);
}

VulkanShaderModule::~VulkanShaderModule() noexcept = default;

const IGraphicsDevice* VulkanShaderModule::getDevice() const noexcept
{
	return m_impl->getDevice();
}

const ShaderType& VulkanShaderModule::getType() const noexcept
{
	return m_impl->getType();
}

const String& VulkanShaderModule::getFileName() const noexcept
{
	return m_impl->getFileName();
}

const String& VulkanShaderModule::getEntryPoint() const noexcept
{
	return m_impl->getEntryPoint();
}

VkPipelineShaderStageCreateInfo VulkanShaderModule::getShaderStageDefinition() const
{
	VkPipelineShaderStageCreateInfo shaderStageDefinition = {};
	shaderStageDefinition.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageDefinition.module = this->handle();
	shaderStageDefinition.pName = this->getEntryPoint().c_str();

	switch (this->getType())
	{
	case ShaderType::Vertex:
		shaderStageDefinition.stage = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case ShaderType::TessellationControl:
		shaderStageDefinition.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		break;
	case ShaderType::TessellationEvaluation:
		shaderStageDefinition.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		break;
	case ShaderType::Geometry:
		shaderStageDefinition.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		break;
	case ShaderType::Fragment:
		shaderStageDefinition.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case ShaderType::Compute:
		shaderStageDefinition.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		break;
	default:
		throw std::runtime_error("Unsupported shader type detected.");
	}

	return shaderStageDefinition;
}