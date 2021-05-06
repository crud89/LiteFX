#include <litefx/backends/dx12.hpp>
#include <sstream>
#include <fstream>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12ShaderModule::DirectX12ShaderModuleImpl : public Implement<DirectX12ShaderModule> {
public:
	friend class DirectX12ShaderModule;

private:
	ShaderStage m_type;
	String m_fileName, m_entryPoint, m_fileContents;

public:
	DirectX12ShaderModuleImpl(DirectX12ShaderModule* parent, const ShaderStage& type, const String& fileName, const String& entryPoint) :
		base(parent), m_fileName(fileName), m_entryPoint(entryPoint), m_type(type) { }

private:
	String readFileContents(const String& fileName) {
		std::ifstream file(m_fileName, std::ios::in | std::ios::binary);

		if (!file.is_open())
			throw RuntimeException("Unable to open shader file.");

		std::stringstream buffer;
		buffer << file.rdbuf();

		return buffer.str();
	}

public:
	D3D12_SHADER_BYTECODE initialize()
	{
		m_fileContents = this->readFileContents(m_fileName);
		
		D3D12_SHADER_BYTECODE bytecode = {};
		bytecode.pShaderBytecode = reinterpret_cast<const void*>(m_fileContents.c_str());
		bytecode.BytecodeLength = m_fileContents.size();

		return bytecode;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderModule::DirectX12ShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint) :
	IResource({ }), m_impl(makePimpl<DirectX12ShaderModuleImpl>(this, type, fileName, entryPoint))
{
	this->handle() = m_impl->initialize();
}

DirectX12ShaderModule::~DirectX12ShaderModule() noexcept = default;

const ShaderStage& DirectX12ShaderModule::getType() const noexcept
{
	return m_impl->m_type;
}

const String& DirectX12ShaderModule::getFileName() const noexcept
{
	return m_impl->m_fileName;
}

const String& DirectX12ShaderModule::getEntryPoint() const noexcept
{
	return m_impl->m_entryPoint;
}