#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12ComputePipeline::DirectX12ComputePipelineImpl {
public:
	friend class DirectX12ComputePipelineBuilder;
	friend class DirectX12ComputePipeline;

private:
	SharedPtr<const DirectX12Device> m_device;
	SharedPtr<DirectX12PipelineLayout> m_layout;
	SharedPtr<DirectX12ShaderProgram> m_program;

public:
	DirectX12ComputePipelineImpl(const DirectX12Device& device, const SharedPtr<DirectX12PipelineLayout>& layout, const SharedPtr<DirectX12ShaderProgram>& shaderProgram) :
		m_device(device.shared_from_this()), m_layout(layout), m_program(shaderProgram)
	{
	}

	DirectX12ComputePipelineImpl(const DirectX12Device& device) :
		m_device(device.shared_from_this())
	{
	}

public:
	ComPtr<ID3D12PipelineState> initialize([[maybe_unused]] const DirectX12ComputePipeline& pipeline)
	{
		// Define the pipeline state.
		D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDescription = {};

		// Setup shader stages.
		auto modules = m_program->modules() | std::ranges::to<std::vector>();
		LITEFX_TRACE(DIRECTX12_LOG, "Using shader program {0} with {1} modules...", static_cast<void*>(m_program.get()), modules.size());

		std::ranges::for_each(modules, [&, i = 0](const auto& shaderModule) mutable {
#ifdef NDEBUG
			(void)i; // Required as [[maybe_unused]] is not supported in captures.
#else
			LITEFX_TRACE(DIRECTX12_LOG, "\tModule {0}/{1} (\"{2}\") state: {{ Type: {3}, EntryPoint: {4} }}", ++i, modules.size(), shaderModule.fileName(), shaderModule.type(), shaderModule.entryPoint());
#endif

			switch (shaderModule.type())
			{
			case ShaderStage::Compute:
				pipelineStateDescription.CS.pShaderBytecode = shaderModule.handle()->GetBufferPointer();
				pipelineStateDescription.CS.BytecodeLength = shaderModule.handle()->GetBufferSize();
				break;
			default:
				throw InvalidArgumentException("shaderModule", "Trying to bind shader to unsupported shader stage '{0}'.", shaderModule.type());
			}
		});

		// Create a pipeline state description.
		pipelineStateDescription.pRootSignature = std::as_const(*m_layout).handle().Get();

		// Create the pipeline state instance.
		ComPtr<ID3D12PipelineState> pipelineState;
		raiseIfFailed(m_device->handle()->CreateComputePipelineState(&pipelineStateDescription, IID_PPV_ARGS(&pipelineState)), "Unable to create compute pipeline state.");

#ifndef NDEBUG
		pipelineState->SetName(Widen(pipeline.name()).c_str());
#endif

		return pipelineState;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12ComputePipeline::DirectX12ComputePipeline(const DirectX12Device& device, const SharedPtr<DirectX12PipelineLayout>& layout, const SharedPtr<DirectX12ShaderProgram>& shaderProgram, const String& name) :
	DirectX12PipelineState(nullptr), m_impl(device, layout, shaderProgram)
{
	if (!name.empty())
		this->name() = name;

	this->handle() = m_impl->initialize(*this);
}

DirectX12ComputePipeline::DirectX12ComputePipeline(const DirectX12Device& device) noexcept :
	DirectX12PipelineState(nullptr), m_impl(device)
{
}

DirectX12ComputePipeline::DirectX12ComputePipeline(DirectX12ComputePipeline&&) noexcept = default;
DirectX12ComputePipeline& DirectX12ComputePipeline::operator=(DirectX12ComputePipeline&&) noexcept = default;
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

DirectX12ComputePipelineBuilder::DirectX12ComputePipelineBuilder(const DirectX12Device& device, const String& name) :
	ComputePipelineBuilder(UniquePtr<DirectX12ComputePipeline>(new DirectX12ComputePipeline(device)))
{
	this->instance()->name() = name;
}

DirectX12ComputePipelineBuilder::~DirectX12ComputePipelineBuilder() noexcept = default;

void DirectX12ComputePipelineBuilder::build()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = this->state().pipelineLayout;
	instance->m_impl->m_program = this->state().shaderProgram;
	instance->handle() = instance->m_impl->initialize(*instance);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)