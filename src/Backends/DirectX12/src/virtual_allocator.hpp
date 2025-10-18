#pragma once
#include <litefx/backends/dx12.hpp>
#include "buffer.h"
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Virtual Allocator.
// ------------------------------------------------------------------------------------------------

template<>
struct VirtualAllocator::AllocatorImpl<DirectX12Backend> final : public VirtualAllocator::AllocatorImplBase {
private:
	D3D12MA::VirtualBlock* m_block{ nullptr };

public:
	AllocatorImpl(UInt64 size, AllocationAlgorithm algorithm) :
		AllocatorImplBase(size, algorithm)
	{
		D3D12MA::VIRTUAL_BLOCK_DESC blockCreateInfo = {
			.Flags = algorithm == AllocationAlgorithm::Linear ?
				D3D12MA::VIRTUAL_BLOCK_FLAG_ALGORITHM_LINEAR :
				D3D12MA::VIRTUAL_BLOCK_FLAG_NONE,
			.Size = size
		};

		raiseIfFailed(D3D12MA::CreateVirtualBlock(&blockCreateInfo, &m_block), "Unable to create virtual allocator.");
	}

	AllocatorImpl(const AllocatorImpl&) = delete;
	AllocatorImpl(AllocatorImpl&&) noexcept = delete;
	AllocatorImpl& operator=(const AllocatorImpl&) = delete;
	AllocatorImpl& operator=(AllocatorImpl&&) noexcept = delete;

	~AllocatorImpl() noexcept override
	{
		m_block->Release();
	}

	inline Allocation allocate(UInt64 size, UInt32 alignment, AllocationStrategy strategy = AllocationStrategy::OptimizePacking, void* privateData = nullptr) const override
	{
		D3D12MA::VIRTUAL_ALLOCATION_DESC allocDesc = {
			.Flags = strategy == AllocationStrategy::OptimizeTime ?
				D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME :
				D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY,
			.Size = size,
			.Alignment = alignment
		};

		D3D12MA::VirtualAllocation allocation{};
		UINT64 offset{};
		auto result = m_block->Allocate(&allocDesc, &allocation, &offset);

		if (FAILED(result)) [[unlikely]]
			throw RuntimeException("An allocation from a virtual allocator failed.");

		if (privateData != nullptr)
			m_block->SetAllocationPrivateData(allocation, privateData);

		return { .Handle = allocation.AllocHandle, .Size = size, .Offset = offset };
	}

	inline void free(Allocation&& allocation) const override
	{
		m_block->FreeAllocation({ .AllocHandle = std::move(allocation.Handle) }); // NOLINT(performance-move-const-arg)
	}

	inline void* privateData(const Allocation& allocation) const override
	{
		D3D12MA::VIRTUAL_ALLOCATION_INFO allocationInfo{};
		m_block->GetAllocationInfo(std::bit_cast<D3D12MA::VirtualAllocation>(allocation.Handle), &allocationInfo);
		return allocationInfo.pPrivateData;
	}
};