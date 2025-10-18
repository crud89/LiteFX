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

public:
	BlitImpl(const DirectX12Device& device) :
		m_device(device.weak_from_this())
	{
	}

public:
	void initialize(const DirectX12Device& device)
	{
		// Create the sampler state.
		auto sampler = device.factory().createSampler(FilterMode::Linear, FilterMode::Linear, BorderMode::ClampToEdge, BorderMode::ClampToEdge, BorderMode::ClampToEdge);

		// Allocate shader module.
		auto blitShader = LiteFX::Graphics::Shaders::blit_dxi::open();
		Array<UniquePtr<DirectX12ShaderModule>> modules;
		modules.push_back(makeUnique<DirectX12ShaderModule>(device, ShaderStage::Compute, blitShader, LiteFX::Graphics::Shaders::blit_dxi::name(), "main"));
		auto shaderProgram = DirectX12ShaderProgram::create(device, modules | std::views::as_rvalue);

		// Allocate descriptor set layouts.
		// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
		auto bufferLayouts = Array<DirectX12DescriptorLayout>{ 
			{ DescriptorType::ConstantBuffer, 0, 16 }, 
			{ DescriptorType::Texture, 1, 0 }, 
			{ DescriptorType::RWTexture, 2, 0 },
			{ *sampler, 3 } // Static sampler at s3.
		};
		Array<SharedPtr<DirectX12DescriptorSetLayout>> descriptorSetLayouts;
		descriptorSetLayouts.push_back(DirectX12DescriptorSetLayout::create(device, bufferLayouts, 0, ShaderStage::Compute));
		// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

		// Create a pipeline layout.
		auto pipelineLayout = DirectX12PipelineLayout::create(device, descriptorSetLayouts, nullptr);

		// Create the pipeline.
		m_pipeline = makeUnique<DirectX12ComputePipeline>(device, pipelineLayout, shaderProgram, "Blit");
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
	auto resourceBindings = resourceBindingsLayout.allocate(image.levels() * image.layers()) | std::ranges::to<std::vector>();
	const auto& parametersLayout = resourceBindingsLayout.descriptor(0);
	auto parameters = device->factory().createBuffer(parametersLayout.type(), ResourceHeap::Dynamic, parametersLayout.elementSize(), image.levels());
	parameters->map(parametersBlock, sizeof(Parameters));
	commandBuffer.track(parameters);

	// Transition the texture into a read/write state.
	DirectX12Barrier startBarrier(PipelineStage::All, PipelineStage::Compute);
	startBarrier.transition(image, ResourceAccess::None, ResourceAccess::ShaderReadWrite, ImageLayout::Undefined, ImageLayout::ReadWrite);
	commandBuffer.barrier(startBarrier);
	auto resourceBinding = resourceBindings.begin();

	for (UInt32 l(0); l < image.layers(); ++l)
	{
		auto size = image.extent();

		for (UInt32 i(1); i < image.levels(); ++i, size /= 2)
		{
			auto resource = std::move(*resourceBinding);

			// Update the invocation parameters.
			resource->update(parametersLayout.binding(), *parameters, i, 1);

			// Bind the previous mip map level to the SRV at binding point 1.
			resource->update(1, image, 0, i - 1, 1, l, 1);

			// Bind the current level to the UAV at binding point 2.
			resource->update(2, image, 0, i, 1, l, 1);

			// Dispatch the pipeline.
			commandBuffer.bind(*resource, pipeline);
			commandBuffer.dispatch({ std::max<UInt32>(static_cast<UInt32>(size.width()) / 8, 1), std::max<UInt32>(static_cast<UInt32>(size.height()) / 8, 1), 1 }); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

			// Wait for all writes.
			DirectX12Barrier subBarrier(PipelineStage::Compute, PipelineStage::Compute);
			subBarrier.transition(image, i, 1, l, 1, 0, ResourceAccess::ShaderReadWrite, ResourceAccess::ShaderRead, ImageLayout::ReadWrite, ImageLayout::ShaderResource);
			commandBuffer.barrier(subBarrier);

			// Advance to the next resource binding and track the current one until the command buffer finished.
			commandBuffer.track(std::move(resource));
			resourceBinding++;
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