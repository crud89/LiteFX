#include <litefx/gfx/blitter.hpp>

using namespace LiteFX::Graphics;
using namespace LiteFX::Rendering::Backends;

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND

#include <shader_resources.hpp>

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

template<>
class Blitter<DirectX12Backend>::BlitImpl {
    friend class Blitter<DirectX12Backend>;

private:
	WeakPtr<const DirectX12Device> m_device;
	UniquePtr<DirectX12ComputePipeline> m_pipeline;
	SharedPtr<IDirectX12Sampler> m_sampler;

public:
	BlitImpl(const DirectX12Device& device) :
		m_device(device.weak_from_this())
	{
	}

public:
	void initialize(const DirectX12Device& device)
	{
		// Allocate shader module.
		auto blitShader = LiteFX::Graphics::Shaders::blit_dxi::open();
		Array<UniquePtr<DirectX12ShaderModule>> modules;
		modules.push_back(makeUnique<DirectX12ShaderModule>(device, ShaderStage::Compute, blitShader, LiteFX::Graphics::Shaders::blit_dxi::name(), "main"));
		auto shaderProgram = DirectX12ShaderProgram::create(device, modules | std::views::as_rvalue);

		// Allocate descriptor set layouts.
		// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
		auto bufferLayouts = Array<DirectX12DescriptorLayout>{ DirectX12DescriptorLayout{ DescriptorType::ConstantBuffer, 0, 16 }, DirectX12DescriptorLayout{ DescriptorType::Texture, 1, 0 }, DirectX12DescriptorLayout{ DescriptorType::RWTexture, 2, 0 } };
		auto samplerLayouts = Array<DirectX12DescriptorLayout>{ DirectX12DescriptorLayout{ DescriptorType::Sampler, 0, 0 } };
		auto descriptorSetLayouts = Array<SharedPtr<DirectX12DescriptorSetLayout>>{ DirectX12DescriptorSetLayout::create(device, bufferLayouts, 0, ShaderStage::Compute), DirectX12DescriptorSetLayout::create(device, samplerLayouts, 1, ShaderStage::Compute) };
		// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

		// Create a pipeline layout.
		auto pipelineLayout = DirectX12PipelineLayout::create(device, descriptorSetLayouts, nullptr);

		// Create the pipeline.
		m_pipeline = makeUnique<DirectX12ComputePipeline>(device, pipelineLayout, shaderProgram, "Blit");

		// Create the sampler state.
		m_sampler = device.factory().createSampler(FilterMode::Linear, FilterMode::Linear, BorderMode::ClampToEdge, BorderMode::ClampToEdge, BorderMode::ClampToEdge);
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

template <>
Blitter<DirectX12Backend>::Blitter(const DirectX12Device& device) :
	m_impl(device)
{
	m_impl->initialize(device);
}

template <>
void Blitter<DirectX12Backend>::generateMipMaps(IDirectX12Image& image, DirectX12CommandBuffer& commandBuffer)
{
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Unable to generate mip maps on a device that has been released.");

	struct Parameters {
		Float sizeX;
		Float sizeY;
		Float sRGB;
		Float padding;
	};

	// Create the array of parameter data.
	Array<Parameters> parametersData(image.levels());

	std::ranges::generate(parametersData, [&image, i = 0]() mutable {
		auto level = i++;

		return Parameters{
			.sizeX = 1.f / static_cast<Float>(std::max<size_t>(image.extent(level).width(), 1)),
			.sizeY = 1.f / static_cast<Float>(std::max<size_t>(image.extent(level).height(), 1)),
			.sRGB = DX12::isSRGB(image.format()) ? 1.f : 0.f
		};
	});

	auto parametersBlock = parametersData |
		std::views::transform([](const Parameters& parameters) { return static_cast<const void*>(&parameters); }) |
		std::ranges::to<Array<const void*>>();

	// Set the active pipeline state.
	auto& pipeline = *m_impl->m_pipeline;
	commandBuffer.use(pipeline);

	// Create and bind the parameters.
	const auto& resourceBindingsLayout = pipeline.layout()->descriptorSet(0);
	auto resourceBindings = resourceBindingsLayout.allocate(image.levels() * image.layers());
	const auto& parametersLayout = resourceBindingsLayout.descriptor(0);
	auto parameters = device->factory().createBuffer(parametersLayout.type(), ResourceHeap::Dynamic, parametersLayout.elementSize(), image.levels());
	parameters->map(parametersBlock, sizeof(Parameters));

	// Create and bind the sampler.
	const auto& samplerBindingsLayout = pipeline.layout()->descriptorSet(1);
	auto samplerBindings = samplerBindingsLayout.allocate();
	samplerBindings->update(0, *m_impl->m_sampler);
	commandBuffer.bind(*samplerBindings, pipeline);

	// Transition the texture into a read/write state.
	DirectX12Barrier startBarrier(PipelineStage::All, PipelineStage::Compute);
	startBarrier.transition(image, ResourceAccess::None, ResourceAccess::ShaderReadWrite, ImageLayout::Undefined, ImageLayout::ReadWrite);
	commandBuffer.barrier(startBarrier);
	auto resource = resourceBindings.begin();

	for (UInt32 l(0); l < image.layers(); ++l, ++resource)
	{
		auto size = image.extent();

		for (UInt32 i(1); i < image.levels(); ++i, size /= 2)
		{
			// Update the invocation parameters.
			(*resource)->update(parametersLayout.binding(), *parameters, i, 1);

			// Bind the previous mip map level to the SRV at binding point 1.
			(*resource)->update(1, image, 0, i - 1, 1, l, 1);

			// Bind the current level to the UAV at binding point 2.
			(*resource)->update(2, image, 0, i, 1, l, 1);

			// Dispatch the pipeline.
			commandBuffer.bind(*(*resource), pipeline);
			commandBuffer.dispatch({ std::max<UInt32>(static_cast<UInt32>(size.width()) / 8, 1), std::max<UInt32>(static_cast<UInt32>(size.height()) / 8, 1), 1 }); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

			// Wait for all writes.
			DirectX12Barrier subBarrier(PipelineStage::Compute, PipelineStage::Compute);
			subBarrier.transition(image, i, 1, l, 1, 0, ResourceAccess::ShaderReadWrite, ResourceAccess::ShaderRead, ImageLayout::ReadWrite, ImageLayout::ShaderResource);
			commandBuffer.barrier(subBarrier);
			resource++;
		}

		// Original sub-resource also needs to be transitioned.
		DirectX12Barrier endBarrier(PipelineStage::Compute, PipelineStage::All);
		endBarrier.transition(image, 0, 1, l, 1, 0, ResourceAccess::ShaderReadWrite, ResourceAccess::ShaderRead, ImageLayout::ReadWrite, ImageLayout::ShaderResource);
		commandBuffer.barrier(endBarrier);
	}
}

// ------------------------------------------------------------------------------------------------
// Export definition.
// ------------------------------------------------------------------------------------------------

template class LITEFX_GRAPHICS_API LiteFX::Graphics::Blitter<Backends::DirectX12Backend>;

#endif // LITEFX_BUILD_DIRECTX_12_BACKEND