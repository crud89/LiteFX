#pragma once

#include <litefx/backends/vulkan.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
extern PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddress;
extern PFN_vkWriteResourceDescriptorsEXT vkWriteResourceDescriptors;
extern PFN_vkWriteSamplerDescriptorsEXT vkWriteSamplerDescriptors;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

namespace LiteFX::Rendering::Backends {

    inline void writeBufferDescriptors(const VulkanDevice& device, DescriptorType type, const IVulkanBuffer& buffer, UInt32 firstElement, UInt32 elementsPerDescriptor, Format texelFormat, UInt32 structureStride, Span<const VkHostAddressRangeEXT> addressRanges)
    {
        auto descriptors = static_cast<UInt32>(addressRanges.size());
        UInt64 elements = static_cast<UInt64>(descriptors) * elementsPerDescriptor;

        if (firstElement + elements > buffer.elements()) [[unlikely]]
            throw ArgumentOutOfRangeException("addressRanges", "The buffer only has {0} elements, however there are {1} elements starting at element {2} specified.", buffer.elements(), elements, firstElement);

        // Validate the descriptor binding.The stride itself is defined by the shader on Vulkan.
        buffer.validateBinding(type, elementsPerDescriptor, structureStride);

        auto& limits = device.adapter().limits();
        auto alignedSize = buffer.alignedElementSize();
        auto elementSize = buffer.elementSize();
        auto viewSize = (elementsPerDescriptor - 1) * alignedSize + elementSize;

        auto requireAlignment = [&](UInt64 alignment) {
            if (alignment > 1 && (buffer.virtualAddress() % alignment != 0 || alignedSize % alignment != 0)) [[unlikely]]
                throw InvalidArgumentException("buffer", "{0} descriptors require an offset alignment of {1} bytes, but the buffer uses an aligned element size of {2} bytes.", type, alignment, alignedSize);
        };

        VkDescriptorType descriptorType{};
        bool texelBuffer = false;

        switch (type)
        {
        case DescriptorType::ConstantBuffer:
            requireAlignment(limits.minUniformBufferOffsetAlignment);

            if (viewSize > limits.maxUniformBufferRange) [[unlikely]]
                throw InvalidArgumentException("buffer", "Constant buffer views must not exceed {0} bytes, but the view spans {1} bytes.", limits.maxUniformBufferRange, viewSize);

            descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        case DescriptorType::StructuredBuffer:
        case DescriptorType::RWStructuredBuffer:
        case DescriptorType::ByteAddressBuffer:
        case DescriptorType::RWByteAddressBuffer:
            requireAlignment(limits.minStorageBufferOffsetAlignment);

            if (viewSize > limits.maxStorageBufferRange) [[unlikely]]
                throw InvalidArgumentException("buffer", "Storage buffer views must not exceed {0} bytes, but the view spans {1} bytes.", limits.maxStorageBufferRange, viewSize);

            descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            break;
        case DescriptorType::Buffer:
        case DescriptorType::RWBuffer:
        {
            if (texelFormat == Format::None) [[unlikely]]
                throw InvalidArgumentException("texelFormat", "Texel buffer descriptors require a texel format.");

            if (!device.supportsTexelBufferFormat(texelFormat, type == DescriptorType::RWBuffer)) [[unlikely]]
                throw InvalidArgumentException("texelFormat", "The format {0} is not supported for {1} descriptors on this device.", texelFormat, type);

            auto texelSize = getSize(texelFormat);

            if (elementSize % texelSize != 0 || alignedSize % texelSize != 0) [[unlikely]]
                throw InvalidArgumentException("texelFormat", "The buffer element size of {0} bytes (aligned: {1} bytes) is not a multiple of the texel size of {2} bytes.", elementSize, alignedSize, texelSize);

            if (viewSize / texelSize > limits.maxTexelBufferElements) [[unlikely]]
                throw InvalidArgumentException("buffer", "The view spans more texels than supported by the device ({1} > {0}).", limits.maxTexelBufferElements, viewSize / texelSize);

            requireAlignment(limits.minTexelBufferOffsetAlignment);

            descriptorType = type == DescriptorType::RWBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            texelBuffer = true;
            break;
        }
        default: [[unlikely]]
            throw InvalidArgumentException("type", "Invalid descriptor type. The descriptor type {0} does not describe a buffer.", type);
        }

        // The info structures reference each other, so reserve up front to keep the pointers stable.
        Array<VkDeviceAddressRangeEXT> ranges;
        Array<VkTexelBufferDescriptorInfoEXT> texelInfos;
        Array<VkResourceDescriptorInfoEXT> infos;
        ranges.reserve(descriptors);
        texelInfos.reserve(descriptors);
        infos.reserve(descriptors);

        for (UInt32 d{ 0 }; d < descriptors; ++d)
        {
            UInt64 offset = (static_cast<UInt64>(firstElement) + static_cast<UInt64>(d) * elementsPerDescriptor) * alignedSize;
            auto& range = ranges.emplace_back(static_cast<VkDeviceAddress>(buffer.virtualAddress() + offset), static_cast<VkDeviceSize>(viewSize));
            auto& info = infos.emplace_back(VkResourceDescriptorInfoEXT{ .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT, .type = descriptorType });

            if (texelBuffer)
                info.data.pTexelBuffer = &texelInfos.emplace_back(VkTexelBufferDescriptorInfoEXT {
                    .sType = VK_STRUCTURE_TYPE_TEXEL_BUFFER_DESCRIPTOR_INFO_EXT,
                    .format = Vk::getFormat(texelFormat),
                    .addressRange = range
                });
            else
                info.data.pAddressRange = &range;
        }

        raiseIfFailed(vkWriteResourceDescriptors(device.handle(), descriptors, infos.data(), addressRanges.data()), "Unable to write buffer descriptors.");
    }

    inline void writeImageDescriptor(const VulkanDevice& device, DescriptorType type, const IVulkanImage& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers, VkHostAddressRangeEXT addressRange)
    {
        // Validate the descriptor binding.
        image.validateBinding(type, firstLevel, levels, firstLayer, layers);

        VkDescriptorType descriptorType{};

        switch (type)
        {
        case DescriptorType::Texture:         descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
        case DescriptorType::RWTexture:       descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
        case DescriptorType::InputAttachment: descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; break;
        default: [[unlikely]]
            throw InvalidArgumentException("type", "Invalid descriptor type. The descriptor type {0} does not describe an image.", type);
        }

        if (type == DescriptorType::RWTexture && !device.supportsStorageImageFormat(image.format())) [[unlikely]]
            throw InvalidArgumentException("image", "The device does not support the format {0} to be bound onto a writable image descriptor.", image.format());

        const UInt32 numLevels = levels == 0 ? image.levels() - firstLevel : levels;
        const UInt32 numLayers = layers == 0 ? image.layers() - firstLayer : layers;
        const auto imageLayers = image.layers();
        const auto imageDimensions = image.dimensions();

        VkImageViewCreateInfo imageViewDesc = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image.handle(),
            .viewType = Vk::getImageViewType(imageDimensions, imageLayers),
            .format = Vk::getFormat(image.format()),
            .components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
            .subresourceRange = { 
                .baseMipLevel = firstLevel, 
                .levelCount = type == DescriptorType::RWTexture ? 1u : numLevels, // Storage images only support a single mip map level (in both backends).
                .baseArrayLayer = imageDimensions == ImageDimensions::DIM_3 ? 0u : firstLayer,
                .layerCount = imageDimensions == ImageDimensions::DIM_3 ? 1u : numLayers
            }
        };

        // Views used in descriptors must not combine depth and stencil. Read the depth plane only, as in the DX12 backend.
        imageViewDesc.subresourceRange.aspectMask = ::hasDepth(image.format()) ? VK_IMAGE_ASPECT_DEPTH_BIT : (::hasStencil(image.format()) ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT);

        VkImageDescriptorInfoEXT imageInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT,
            .pView = &imageViewDesc,
            .layout = VK_IMAGE_LAYOUT_GENERAL
        };

        VkResourceDescriptorInfoEXT info {
            .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
            .type = descriptorType,
            .data = { .pImage = &imageInfo }
        };

        raiseIfFailed(vkWriteResourceDescriptors(device.handle(), 1u, &info, &addressRange), "Unable to write image descriptor.");
    }

    inline void writeSamplerDescriptor(const VulkanDevice& device, const IVulkanSampler& sampler, VkHostAddressRangeEXT target)
    {
        raiseIfFailed(vkWriteSamplerDescriptors(device.handle(), 1u, &sampler.createInfo(), &target), "Unable to write sampler descriptor.");
    }

    inline void writeAccelerationStructureDescriptor(const VulkanDevice& device, const IVulkanAccelerationStructure& accelerationStructure, VkHostAddressRangeEXT addressRange)
    {
        if (accelerationStructure.buffer() == nullptr || accelerationStructure.handle() == VK_NULL_HANDLE) [[unlikely]]
            throw InvalidArgumentException("accelerationStructure", "The acceleration structure buffer has not yet been allocated.");

        VkAccelerationStructureDeviceAddressInfoKHR addressInfo{
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
            .accelerationStructure = accelerationStructure.handle()
        };

        VkDeviceAddressRangeEXT range{
            .address = vkGetAccelerationStructureDeviceAddress(device.handle(), &addressInfo),
            .size = 0   // Unused for acceleration structures.
        };

        VkResourceDescriptorInfoEXT info{
            .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
            .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
            .data = { .pAddressRange = &range }
        };

        raiseIfFailed(vkWriteResourceDescriptors(device.handle(), 1u, &info, &addressRange), "Unable to write acceleration structure descriptor.");
    }

}