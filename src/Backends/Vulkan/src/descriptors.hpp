#pragma once

#include <litefx/backends/vulkan.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
extern PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddress;
extern PFN_vkWriteResourceDescriptorsEXT vkWriteResourceDescriptors;
extern PFN_vkWriteSamplerDescriptorsEXT vkWriteSamplerDescriptors;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

namespace LiteFX::Rendering::Backends {

    inline UInt32 bufferElements(const IBuffer& buffer, UInt32 firstElement, UInt32 elements)
    {
        if (firstElement >= buffer.elements()) [[unlikely]]
            throw ArgumentOutOfRangeException("firstElement", "The buffer only has {0} elements, but element {1} was requested.", buffer.elements(), firstElement);

        UInt32 count = elements > 0 ? elements : buffer.elements() - firstElement;

        if (firstElement + count > buffer.elements()) [[unlikely]]
            throw ArgumentOutOfRangeException("elements", "The buffer only has {0} elements, however there are {1} elements starting at element {2} specified.", buffer.elements(), count, firstElement);

        return count;
    }

    inline void writeBufferDescriptors(const VulkanDevice& device, DescriptorType type, const IVulkanBuffer& buffer, UInt32 firstElement, Format texelFormat, Span<const VkHostAddressRangeEXT> addressRanges)
    {
        auto count = static_cast<UInt32>(addressRanges.size());

        if (firstElement + count > buffer.elements()) [[unlikely]]
            throw ArgumentOutOfRangeException("targets", "The buffer only has {0} elements, however there are {1} elements starting at element {2} specified.", buffer.elements(), count, firstElement);

        VkDescriptorType descriptorType{};

        switch (type)
        {
        case DescriptorType::ConstantBuffer:      descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
        case DescriptorType::ByteAddressBuffer:
        case DescriptorType::RWByteAddressBuffer:
        case DescriptorType::StructuredBuffer:
        case DescriptorType::RWStructuredBuffer:  descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
        case DescriptorType::Buffer:              descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; break;
        case DescriptorType::RWBuffer:            descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER; break;
        default: [[unlikely]]
            throw InvalidArgumentException("type", "Invalid descriptor type. The descriptor type {0} does not describe a buffer.", type);
        }

        bool texelBuffer = type == DescriptorType::Buffer || type == DescriptorType::RWBuffer;

        if (texelBuffer && texelFormat == Format::None) [[unlikely]]
            throw InvalidArgumentException("texelFormat", "Texel buffer descriptors require a texel format.");

        // The info structures reference each other, so reserve up front to keep the pointers stable.
        Array<VkDeviceAddressRangeEXT> ranges;
        Array<VkTexelBufferDescriptorInfoEXT> texelInfos;
        Array<VkResourceDescriptorInfoEXT> infos;
        ranges.reserve(count);
        texelInfos.reserve(count);
        infos.reserve(count);

        for (UInt32 i{ 0 }; i < count; ++i)
        {
            auto& range = ranges.emplace_back(buffer.virtualAddress() + (firstElement + i) * buffer.alignedElementSize(), buffer.alignedElementSize());
            auto& info = infos.emplace_back(VkResourceDescriptorInfoEXT{ .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT, .type = descriptorType });

            if (texelBuffer)
                info.data.pTexelBuffer = &texelInfos.emplace_back(VkTexelBufferDescriptorInfoEXT{
                    .sType = VK_STRUCTURE_TYPE_TEXEL_BUFFER_DESCRIPTOR_INFO_EXT,
                    .format = Vk::getFormat(texelFormat),
                    .addressRange = range
                });
            else
                info.data.pAddressRange = &range;
        }

        raiseIfFailed(vkWriteResourceDescriptors(device.handle(), count, infos.data(), addressRanges.data()), "Unable to write buffer descriptors.");
    }

    inline void writeImageDescriptor(const VulkanDevice& device, DescriptorType type, const IVulkanImage& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers, VkHostAddressRangeEXT addressRange)
    {
        VkDescriptorType descriptorType{};

        switch (type)
        {
        case DescriptorType::Texture:         descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
        case DescriptorType::RWTexture:       descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
        case DescriptorType::InputAttachment: descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; break;
        default: [[unlikely]]
            throw InvalidArgumentException("type", "Invalid descriptor type. The descriptor type {0} does not describe an image.", type);
        }

        const UInt32 numLevels = levels == 0 ? image.levels() - firstLevel : levels;
        const UInt32 numLayers = layers == 0 ? image.layers() - firstLayer : layers;

        if (firstLevel + numLevels > image.levels() || firstLayer + numLayers > image.layers()) [[unlikely]]
            throw ArgumentOutOfRangeException("levels", "The requested sub-resource range (levels {0}-{1}, layers {2}-{3}) exceeds the image ({4} levels, {5} layers).", 
                firstLevel, firstLevel + numLevels, firstLayer, firstLayer + numLayers, image.levels(), image.layers());

        VkImageViewCreateInfo imageViewDesc = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image.handle(),
            .viewType = Vk::getImageViewType(image.dimensions(), numLayers), // TODO: What if we want to bind an array with one layer only, though?!... `DescriptorLayout` should get an "isArray" property.
            .format = Vk::getFormat(image.format()),
            .components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
            .subresourceRange = { .baseMipLevel = firstLevel, .levelCount = numLevels, .baseArrayLayer = firstLayer, .layerCount = numLayers }
        };

        if (!::hasDepth(image.format()) && !::hasStencil(image.format()))
            imageViewDesc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        else
        {
            // TODO: This probably wont work, instead we need separate views here. Maybe we could add a "plane" parameter that addresses the depth/stencil view.
            if (::hasDepth(image.format()))
                imageViewDesc.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;

            if (::hasStencil(image.format()))
                imageViewDesc.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        VkImageDescriptorInfoEXT imageInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT,
            .pView = &imageViewDesc,
            .layout = type == DescriptorType::RWTexture ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        VkResourceDescriptorInfoEXT info{
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