#include <litefx/backends/vulkan.hpp>
#include <sstream>
#include <fstream>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanShaderModule::VulkanShaderModuleImpl : public Implement<VulkanShaderModule> {
public:
	friend class VulkanShaderModule;

private:
	ShaderStage m_type;
	String m_fileName, m_entryPoint;
	const VulkanDevice& m_device;

public:
	VulkanShaderModuleImpl(VulkanShaderModule* parent, const VulkanDevice& device, const ShaderStage& type, const String& fileName, const String& entryPoint) :
		base(parent), m_device(device), m_fileName(fileName), m_entryPoint(entryPoint), m_type(type) 
	{
	}

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
	VkShaderModule initialize()
	{
		String fileContents = this->readFileContents(m_fileName);

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = fileContents.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(fileContents.c_str());

		VkShaderModule module;

		if (::vkCreateShaderModule(m_device.handle(), &createInfo, nullptr, &module) != VK_SUCCESS)
			throw std::runtime_error("Unable to compile shader file.");

		return module;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderModule::VulkanShaderModule(const VulkanDevice& device, const ShaderStage& type, const String& fileName, const String& entryPoint) :
	Resource<VkShaderModule>(VK_NULL_HANDLE), m_impl(makePimpl<VulkanShaderModuleImpl>(this, device, type, fileName, entryPoint))
{
	this->handle() = m_impl->initialize();
}

VulkanShaderModule::~VulkanShaderModule() noexcept
{
	::vkDestroyShaderModule(m_impl->m_device.handle(), this->handle(), nullptr);
}

const ShaderStage& VulkanShaderModule::type() const noexcept
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

VkPipelineShaderStageCreateInfo VulkanShaderModule::shaderStageDefinition() const
{
	VkPipelineShaderStageCreateInfo shaderStageDefinition = {};
	shaderStageDefinition.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageDefinition.module = this->handle();
	shaderStageDefinition.pName = this->entryPoint().c_str();
	shaderStageDefinition.stage = Vk::getShaderStage(this->type());

	return shaderStageDefinition;
}