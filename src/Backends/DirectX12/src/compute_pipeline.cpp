#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12ComputePipeline::DirectX12ComputePipelineImpl : public Implement<DirectX12ComputePipeline> {
public:
	friend class DirectX12ComputePipelineBuilder;
	friend class DirectX12ComputePipeline;

private:
	SharedPtr<DirectX12PipelineLayout> m_layout;
	SharedPtr<DirectX12ShaderProgram> m_program;
	String m_name;
	const DirectX12Device& m_device;

public:
	DirectX12ComputePipelineImpl(DirectX12ComputePipeline* parent, const DirectX12Device& device, const String& name, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram) :
		base(parent), m_device(device), m_name(name), m_layout(layout), m_program(shaderProgram)
	{
	}

	DirectX12ComputePipelineImpl(DirectX12ComputePipeline* parent, const DirectX12Device& device) :
		base(parent), m_device(device)
	{
	}

public:
	ComPtr<ID3D12PipelineState> initialize()
	{
		// Define the pipeline state.
		D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDescription = {};

		// Setup shader stages.
		auto modules = m_program->modules();
		LITEFX_TRACE(DIRECTX12_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(m_program.get()), modules.size());

		std::ranges::for_each(modules, [&, i = 0](const DirectX12ShaderModule* shaderModule) mutable {
			LITEFX_TRACE(DIRECTX12_LOG, "\tModule {0}/{1} (\"{2}\") state: {{ Type: {3}, EntryPoint: {4} }}", ++i, modules.size(), shaderModule->fileName(), shaderModule->type(), shaderModule->entryPoint());

			switch (shaderModule->type())
			{
			case ShaderStage::Compute:
				pipelineStateDescription.CS.pShaderBytecode = shaderModule->handle()->GetBufferPointer();
				pipelineStateDescription.CS.BytecodeLength = shaderModule->handle()->GetBufferSize();
				break;
			default:
				throw InvalidArgumentException("Trying to bind shader to unsupported shader stage '{0}'.", shaderModule->type());
			}
		});

		// Create a pipeline state description.
		pipelineStateDescription.pRootSignature = std::as_const(*m_layout).handle().Get();

		// Create the pipeline state instance.
		ComPtr<ID3D12PipelineState> pipelineState;
		raiseIfFailed<RuntimeException>(m_device.handle()->CreateComputePipelineState(&pipelineStateDescription, IID_PPV_ARGS(&pipelineState)), "Unable to create compute pipeline state.");

		return pipelineState;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12ComputePipeline::DirectX12ComputePipeline(const DirectX12Device& device, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram, const String& name) :
	m_impl(makePimpl<DirectX12ComputePipelineImpl>(this, device, name, layout, shaderProgram)), DirectX12PipelineState(nullptr)
{
	this->handle() = m_impl->initialize();
}

DirectX12ComputePipeline::DirectX12ComputePipeline(const DirectX12Device& device) noexcept :
	m_impl(makePimpl<DirectX12ComputePipelineImpl>(this, device)), DirectX12PipelineState(nullptr)
{
}

DirectX12ComputePipeline::~DirectX12ComputePipeline() noexcept = default;

const String& DirectX12ComputePipeline::name() const noexcept
{
	return m_impl->m_name;
}

SharedPtr<const DirectX12ShaderProgram> DirectX12ComputePipeline::program() const noexcept
{
	return m_impl->m_program;
}

SharedPtr<const DirectX12PipelineLayout> DirectX12ComputePipeline::layout() const noexcept
{
	return m_impl->m_layout;
}

void DirectX12ComputePipeline::use(const DirectX12CommandBuffer& commandBuffer) const noexcept
{
	commandBuffer.handle()->SetPipelineState(this->handle().Get());
	commandBuffer.handle()->SetComputeRootSignature(std::as_const(*m_impl->m_layout).handle().Get());
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12ComputePipelineBuilder::DirectX12ComputePipelineBuilderImpl : public Implement<DirectX12ComputePipelineBuilder> {
public:
	friend class DirectX12ComputePipelineBuilderBuilder;
	friend class DirectX12ComputePipelineBuilder;

private:
	UniquePtr<DirectX12PipelineLayout> m_layout;

public:
	DirectX12ComputePipelineBuilderImpl(DirectX12ComputePipelineBuilder* parent) :
		base(parent)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12ComputePipelineBuilder::DirectX12ComputePipelineBuilder(const DirectX12Device& device, const String& name) :
	m_impl(makePimpl<DirectX12ComputePipelineBuilderImpl>(this)), ComputePipelineBuilder(UniquePtr<DirectX12ComputePipeline>(new DirectX12ComputePipeline(device)))
{
	this->instance()->m_impl->m_name = name;
}

DirectX12ComputePipelineBuilder::~DirectX12ComputePipelineBuilder() noexcept = default;

UniquePtr<DirectX12ComputePipeline> DirectX12ComputePipelineBuilder::go()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = std::move(m_impl->m_layout);
	instance->handle() = instance->m_impl->initialize();

	return ComputePipelineBuilder::go();
}

void DirectX12ComputePipelineBuilder::use(UniquePtr<DirectX12PipelineLayout>&& layout)
{
#ifndef NDEBUG
	if (m_impl->m_layout != nullptr)
		LITEFX_WARNING(DIRECTX12_LOG, "Another pipeline layout has already been initialized and will be replaced. A pipeline can only have one pipeline layout.");
#endif

	m_impl->m_layout = std::move(layout);
}


DirectX12ComputePipelineLayoutBuilder DirectX12ComputePipelineBuilder::layout()
{
	return DirectX12ComputePipelineLayoutBuilder(*this);
}
#endif // defined(BUILD_DEFINE_BUILDERS)