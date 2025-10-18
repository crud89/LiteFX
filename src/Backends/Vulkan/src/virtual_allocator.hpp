#pragma once
#include <litefx/backends/vulkan.hpp>
#include "buffer.h"
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Virtual Allocator.
// ------------------------------------------------------------------------------------------------

template<>
struct VirtualAllocator::AllocatorImpl<VulkanBackend> final : public VirtualAllocator::AllocatorImplBase {
private:
	VmaVirtualBlock m_block{};

public:
	AllocatorImpl(UInt64 size, AllocationAlgorithm algorithm) :
		AllocatorImplBase(size, algorithm)
	{
		VmaVirtualBlockCreateInfo blockCreateInfo = {
			.size = size,
			.flags = algorithm == AllocationAlgorithm::Linear ? VMA_VIRTUAL_BLOCK_CREATE_LINEAR_ALGORITHM_BIT : 0u
		};

		raiseIfFailed(::vmaCreateVirtualBlock(&blockCreateInfo, &m_block), "Unable to create virtual allocator.");
	}

	AllocatorImpl(const AllocatorImpl&) = delete;
	AllocatorImpl(AllocatorImpl&&) noexcept = delete;
	AllocatorImpl& operator=(const AllocatorImpl&) = delete;
	AllocatorImpl& operator=(AllocatorImpl&&) noexcept = delete;

	~AllocatorImpl() noexcept override
	{
		::vmaDestroyVirtualBlock(m_block);
	}

	inline Allocation allocate(UInt64 size, UInt32 alignment, AllocationStrategy strategy = AllocationStrategy::OptimizePacking, void* privateData = nullptr) const override
	{
		VmaVirtualAllocationCreateInfo allocationInfo = {
			.size = size,
			.alignment = alignment,
		};

		allocationInfo.flags = strategy == AllocationStrategy::OptimizeTime ?
			VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT :
			VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;

		VmaVirtualAllocation allocation{};
		VkDeviceSize offset{};

		auto result = ::vmaVirtualAllocate(m_block, &allocationInfo, &allocation, &offset);

		if (result == VK_SUCCESS)
		{
			if (privateData != nullptr)
				::vmaSetVirtualAllocationUserData(m_block, allocation, privateData);

			return Allocation{ .Handle = std::bit_cast<UInt64>(allocation), .Size = size, .Offset = static_cast<UInt64>(offset) };
		}
		else [[unlikely]]
			throw RuntimeException("An allocation from a virtual allocator failed.");
	}

	inline void free(Allocation&& allocation) const override
	{
		auto handle = std::bit_cast<VmaVirtualAllocation>(std::move(allocation.Handle)); // NOLINT(performance-move-const-arg)
		::vmaVirtualFree(m_block, handle);
	}

	inline void* privateData(const Allocation& allocation) const override
	{
		VmaVirtualAllocationInfo allocationInfo{};
		::vmaGetVirtualAllocationInfo(m_block, std::bit_cast<VmaVirtualAllocation>(allocation.Handle), &allocationInfo);
		return allocationInfo.pUserData;
	}
};