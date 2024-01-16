#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

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
	const DirectX12Device& m_device;

public:
	DirectX12ComputePipelineImpl(DirectX12ComputePipeline* parent, const DirectX12Device& device, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram) :
		base(parent), m_device(device), m_layout(layout), m_program(shaderProgram)
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
				throw InvalidArgumentException("shaderModule", "Trying to bind shader to unsupported shader stage '{0}'.", shaderModule->type());
			}
		});

		// Create a pipeline state description.
		pipelineStateDescription.pRootSignature = std::as_const(*m_layout).handle().Get();

		// Create the pipeline state instance.
		ComPtr<ID3D12PipelineState> pipelineState;
		raiseIfFailed(m_device.handle()->CreateComputePipelineState(&pipelineStateDescription, IID_PPV_ARGS(&pipelineState)), "Unable to create compute pipeline state.");

#ifndef NDEBUG
		pipelineState->SetName(Widen(m_parent->name()).c_str());
#endif

		return pipelineState;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12ComputePipeline::DirectX12ComputePipeline(const DirectX12Device& device, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram, const String& name) :
	m_impl(makePimpl<DirectX12ComputePipelineImpl>(this, device, layout, shaderProgram)), DirectX12PipelineState(nullptr)
{
	if (!name.empty())
		this->name() = name;

	this->handle() = m_impl->initialize();
}

DirectX12ComputePipeline::DirectX12ComputePipeline(const DirectX12Device& device) noexcept :
	m_impl(makePimpl<DirectX12ComputePipelineImpl>(this, device)), DirectX12PipelineState(nullptr)
{
}

DirectX12ComputePipeline::~DirectX12ComputePipeline() noexcept = default;

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

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

constexpr DirectX12ComputePipelineBuilder::DirectX12ComputePipelineBuilder(const DirectX12Device& device, const String& name) :
	ComputePipelineBuilder(UniquePtr<DirectX12ComputePipeline>(new DirectX12ComputePipeline(device)))
{
	this->instance()->name() = name;
}

constexpr DirectX12ComputePipelineBuilder::~DirectX12ComputePipelineBuilder() noexcept = default;

void DirectX12ComputePipelineBuilder::build()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = m_state.pipelineLayout;
	instance->m_impl->m_program = m_state.shaderProgram;
	instance->handle() = instance->m_impl->initialize();
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)