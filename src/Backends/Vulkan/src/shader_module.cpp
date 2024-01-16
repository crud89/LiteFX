#include <litefx/backends/vulkan.hpp>
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
	String m_fileName, m_entryPoint, m_bytecode;
	const VulkanDevice& m_device;

public:
	VulkanShaderModuleImpl(VulkanShaderModule* parent, const VulkanDevice& device, ShaderStage type, const String& fileName, const String& entryPoint) :
		base(parent), m_device(device), m_fileName(fileName), m_entryPoint(entryPoint), m_type(type) 
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
		createInfo.pCode = reinterpret_cast<const uint32_t*>(fileContents.c_str());

		VkShaderModule module;

		if (::vkCreateShaderModule(m_device.handle(), &createInfo, nullptr, &module) != VK_SUCCESS)
			throw std::runtime_error("Unable to compile shader file.");

#ifndef NDEBUG
		m_device.setDebugName(*reinterpret_cast<const UInt64*>(&module), VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, fmt::format("{0}: {1}", m_fileName, m_entryPoint));
#endif

		m_bytecode = fileContents;
		return module;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderModule::VulkanShaderModule(const VulkanDevice& device, ShaderStage type, const String& fileName, const String& entryPoint) :
	Resource<VkShaderModule>(VK_NULL_HANDLE), m_impl(makePimpl<VulkanShaderModuleImpl>(this, device, type, fileName, entryPoint))
{
	this->handle() = m_impl->initialize();
}

VulkanShaderModule::VulkanShaderModule(const VulkanDevice& device, ShaderStage type, std::istream& stream, const String& name, const String& entryPoint) :
	Resource<VkShaderModule>(VK_NULL_HANDLE), m_impl(makePimpl<VulkanShaderModuleImpl>(this, device, type, name, entryPoint))
{
	this->handle() = m_impl->initialize(stream);
}

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