#include <litefx/backends/dx12.hpp>
#include <d3dcompiler.h>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12ShaderModule::DirectX12ShaderModuleImpl : public Implement<DirectX12ShaderModule> {
public:
	friend class DirectX12ShaderModule;

private:
	ShaderStage m_type;
	String m_fileName, m_entryPoint;

public:
	DirectX12ShaderModuleImpl(DirectX12ShaderModule* parent, const ShaderStage& type, const String& fileName, const String& entryPoint) :
		base(parent), m_fileName(fileName), m_entryPoint(entryPoint), m_type(type) 
	{
	}

public:
	ComPtr<ID3DBlob> initialize()
	{
		ComPtr<ID3DBlob> blob;
		raiseIfFailed<RuntimeException>(::D3DReadFileToBlob(::Widen(m_fileName).c_str(), &blob), "Unable to load shader: {0}.", m_fileName.c_str());
		return blob;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderModule::DirectX12ShaderModule(const DirectX12Device& device, const ShaderStage& type, const String& fileName, const String& entryPoint) :
	DirectX12RuntimeObject(device, &device), m_impl(makePimpl<DirectX12ShaderModuleImpl>(this, type, fileName, entryPoint)), ComResource<ID3DBlob>(nullptr)
{
	this->handle() = m_impl->initialize();
}

DirectX12ShaderModule::~DirectX12ShaderModule() noexcept = default;

const ShaderStage& DirectX12ShaderModule::type() const noexcept
{
	return m_impl->m_type;
}

const String& DirectX12ShaderModule::fileName() const noexcept
{
	return m_impl->m_fileName;
}

const String& DirectX12ShaderModule::entryPoint() const noexcept
{
	return m_impl->m_entryPoint;
}