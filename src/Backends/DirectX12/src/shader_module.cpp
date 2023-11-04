#include <litefx/backends/dx12.hpp>

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
	const DirectX12Device& m_device;

public:
	DirectX12ShaderModuleImpl(DirectX12ShaderModule* parent, const DirectX12Device& device, ShaderStage type, const String& fileName, const String& entryPoint) :
		base(parent), m_device(device), m_fileName(fileName), m_entryPoint(entryPoint), m_type(type) 
	{
	}

public:
	ComPtr<IDxcBlob> initialize()
	{
		// TODO: We share the library handle over the whole api by moving them to the device level at least.
		ComPtr<IDxcLibrary> library;
		raiseIfFailed<RuntimeException>(::DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library)), "Unable to access DirectX shader compiler library.");

		// Read the blob.
		ComPtr<IDxcBlobEncoding> blob;
		raiseIfFailed<RuntimeException>(library->CreateBlobFromFile(::Widen(m_fileName).c_str(), CP_ACP, &blob), "Unable to load shader: {0}.", m_fileName.c_str());

		return blob;
	}

	ComPtr<IDxcBlob> initialize(std::istream& stream)
	{
		// TODO: We share the library handle over the whole api by moving them to the device level at least.
		ComPtr<IDxcLibrary> library;
		raiseIfFailed<RuntimeException>(::DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library)), "Unable to access DirectX shader compiler library.");

		// Create a blob by copying the buffer.
		ComPtr<IDxcBlobEncoding> blob;
		String buffer(std::istreambuf_iterator<char>(stream), {});
		raiseIfFailed<RuntimeException>(library->CreateBlobWithEncodingOnHeapCopy(buffer.data(), buffer.size(), CP_ACP, &blob), "Unable to load shader from stream: {0}.", m_fileName.c_str());

		return blob;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderModule::DirectX12ShaderModule(const DirectX12Device& device, ShaderStage type, const String& fileName, const String& entryPoint) :
	m_impl(makePimpl<DirectX12ShaderModuleImpl>(this, device, type, fileName, entryPoint)), ComResource<IDxcBlob>(nullptr)
{
	this->handle() = m_impl->initialize();
}

DirectX12ShaderModule::DirectX12ShaderModule(const DirectX12Device& device, ShaderStage type, std::istream& stream, const String& name, const String& entryPoint) :
	m_impl(makePimpl<DirectX12ShaderModuleImpl>(this, device, type, name, entryPoint)), ComResource<IDxcBlob>(nullptr)
{
	this->handle() = m_impl->initialize(stream);
}

DirectX12ShaderModule::~DirectX12ShaderModule() noexcept = default;

ShaderStage DirectX12ShaderModule::type() const noexcept
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