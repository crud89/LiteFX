#pragma once

#include "rendering_api.hpp"

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::GraphicsAdapterType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::GraphicsAdapterType t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t) {
		case LiteFX::Rendering::GraphicsAdapterType::CPU: name = "CPU"; break;
		case LiteFX::Rendering::GraphicsAdapterType::GPU: name = "GPU"; break;
		case LiteFX::Rendering::GraphicsAdapterType::Other: name = "Other"; break;
		case LiteFX::Rendering::GraphicsAdapterType::None: name = "None"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::QueueType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::QueueType t, FormatContext& ctx) {
		Array<String> names;

		if (t == LiteFX::Rendering::QueueType::None)
			names.push_back("None");
		else if(t == LiteFX::Rendering::QueueType::Other)
			names.push_back("Other");
		else 
		{
			if ((t & LiteFX::Rendering::QueueType::Compute) == LiteFX::Rendering::QueueType::Compute)
				names.push_back("Compute");
			if ((t & LiteFX::Rendering::QueueType::Graphics) == LiteFX::Rendering::QueueType::Graphics)
				names.push_back("Graphics");
			if ((t & LiteFX::Rendering::QueueType::Transfer) == LiteFX::Rendering::QueueType::Transfer)
				names.push_back("Transfer");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::QueuePriority> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::QueuePriority t, FormatContext& ctx) {
		String name;

		switch (t)
		{
		default:
		case LiteFX::Rendering::QueuePriority::Normal: name = "Normal"; break;
		case LiteFX::Rendering::QueuePriority::High: name = "High"; break;
		case LiteFX::Rendering::QueuePriority::Realtime: name = "Realtime"; break;
		}
		
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::Format> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::Format t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t) {
		case LiteFX::Rendering::Format::R4G4_UNORM: name = "R4G4_UNORM"; break;
		case LiteFX::Rendering::Format::R4G4B4A4_UNORM: name = "R4G4B4A4_UNORM"; break;
		case LiteFX::Rendering::Format::B4G4R4A4_UNORM: name = "B4G4R4A4_UNORM"; break;
		case LiteFX::Rendering::Format::R5G6B5_UNORM: name = "R5G6B5_UNORM"; break;
		case LiteFX::Rendering::Format::B5G6R5_UNORM: name = "B5G6R5_UNORM"; break;
		case LiteFX::Rendering::Format::R5G5B5A1_UNORM: name = "R5G5B5A1_UNORM"; break;
		case LiteFX::Rendering::Format::B5G5R5A1_UNORM: name = "B5G5R5A1_UNORM"; break;
		case LiteFX::Rendering::Format::A1R5G5B5_UNORM: name = "A1R5G5B5_UNORM"; break;
		case LiteFX::Rendering::Format::R8_UNORM: name = "R8_UNORM"; break;
		case LiteFX::Rendering::Format::R8_SNORM: name = "R8_SNORM"; break;
		case LiteFX::Rendering::Format::R8_USCALED: name = "R8_USCALED"; break;
		case LiteFX::Rendering::Format::R8_SSCALED: name = "R8_SSCALED"; break;
		case LiteFX::Rendering::Format::R8_UINT: name = "R8_UINT"; break;
		case LiteFX::Rendering::Format::R8_SINT: name = "R8_SINT"; break;
		case LiteFX::Rendering::Format::R8_SRGB: name = "R8_SRGB"; break;
		case LiteFX::Rendering::Format::R8G8_UNORM: name = "R8G8_UNORM"; break;
		case LiteFX::Rendering::Format::R8G8_SNORM: name = "R8G8_SNORM"; break;
		case LiteFX::Rendering::Format::R8G8_USCALED: name = "R8G8_USCALED"; break;
		case LiteFX::Rendering::Format::R8G8_SSCALED: name = "R8G8_SSCALED"; break;
		case LiteFX::Rendering::Format::R8G8_UINT: name = "R8G8_UINT"; break;
		case LiteFX::Rendering::Format::R8G8_SINT: name = "R8G8_SINT"; break;
		case LiteFX::Rendering::Format::R8G8_SRGB: name = "R8G8_SRGB"; break;
		case LiteFX::Rendering::Format::R8G8B8_UNORM: name = "R8G8B8_UNORM"; break;
		case LiteFX::Rendering::Format::R8G8B8_SNORM: name = "R8G8B8_SNORM"; break;
		case LiteFX::Rendering::Format::R8G8B8_USCALED: name = "R8G8B8_USCALED"; break;
		case LiteFX::Rendering::Format::R8G8B8_SSCALED: name = "R8G8B8_SSCALED"; break;
		case LiteFX::Rendering::Format::R8G8B8_UINT: name = "R8G8B8_UINT"; break;
		case LiteFX::Rendering::Format::R8G8B8_SINT: name = "R8G8B8_SINT"; break;
		case LiteFX::Rendering::Format::R8G8B8_SRGB: name = "R8G8B8_SRGB"; break;
		case LiteFX::Rendering::Format::B8G8R8_UNORM: name = "B8G8R8_UNORM"; break;
		case LiteFX::Rendering::Format::B8G8R8_SNORM: name = "B8G8R8_SNORM"; break;
		case LiteFX::Rendering::Format::B8G8R8_USCALED: name = "B8G8R8_USCALED"; break;
		case LiteFX::Rendering::Format::B8G8R8_SSCALED: name = "B8G8R8_SSCALED"; break;
		case LiteFX::Rendering::Format::B8G8R8_UINT: name = "B8G8R8_UINT"; break;
		case LiteFX::Rendering::Format::B8G8R8_SINT: name = "B8G8R8_SINT"; break;
		case LiteFX::Rendering::Format::B8G8R8_SRGB: name = "B8G8R8_SRGB"; break;
		case LiteFX::Rendering::Format::R8G8B8A8_UNORM: name = "R8G8B8A8_UNORM"; break;
		case LiteFX::Rendering::Format::R8G8B8A8_SNORM: name = "R8G8B8A8_SNORM"; break;
		case LiteFX::Rendering::Format::R8G8B8A8_USCALED: name = "R8G8B8A8_USCALED"; break;
		case LiteFX::Rendering::Format::R8G8B8A8_SSCALED: name = "R8G8B8A8_SSCALED"; break;
		case LiteFX::Rendering::Format::R8G8B8A8_UINT: name = "R8G8B8A8_UINT"; break;
		case LiteFX::Rendering::Format::R8G8B8A8_SINT: name = "R8G8B8A8_SINT"; break;
		case LiteFX::Rendering::Format::R8G8B8A8_SRGB: name = "R8G8B8A8_SRGB"; break;
		case LiteFX::Rendering::Format::B8G8R8A8_UNORM: name = "B8G8R8A8_UNORM"; break;
		case LiteFX::Rendering::Format::B8G8R8A8_SNORM: name = "B8G8R8A8_SNORM"; break;
		case LiteFX::Rendering::Format::B8G8R8A8_USCALED: name = "B8G8R8A8_USCALED"; break;
		case LiteFX::Rendering::Format::B8G8R8A8_SSCALED: name = "B8G8R8A8_SSCALED"; break;
		case LiteFX::Rendering::Format::B8G8R8A8_UINT: name = "B8G8R8A8_UINT"; break;
		case LiteFX::Rendering::Format::B8G8R8A8_SINT: name = "B8G8R8A8_SINT"; break;
		case LiteFX::Rendering::Format::B8G8R8A8_SRGB: name = "B8G8R8A8_SRGB"; break;
		case LiteFX::Rendering::Format::A8B8G8R8_UNORM: name = "A8B8G8R8_UNORM"; break;
		case LiteFX::Rendering::Format::A8B8G8R8_SNORM: name = "A8B8G8R8_SNORM"; break;
		case LiteFX::Rendering::Format::A8B8G8R8_USCALED: name = "A8B8G8R8_USCALED"; break;
		case LiteFX::Rendering::Format::A8B8G8R8_SSCALED: name = "A8B8G8R8_SSCALED"; break;
		case LiteFX::Rendering::Format::A8B8G8R8_UINT: name = "A8B8G8R8_UINT"; break;
		case LiteFX::Rendering::Format::A8B8G8R8_SINT: name = "A8B8G8R8_SINT"; break;
		case LiteFX::Rendering::Format::A8B8G8R8_SRGB: name = "A8B8G8R8_SRGB"; break;
		case LiteFX::Rendering::Format::A2R10G10B10_UNORM: name = "A2R10G10B10_UNORM"; break;
		case LiteFX::Rendering::Format::A2R10G10B10_SNORM: name = "A2R10G10B10_SNORM"; break;
		case LiteFX::Rendering::Format::A2R10G10B10_USCALED: name = "A2R10G10B10_USCALED"; break;
		case LiteFX::Rendering::Format::A2R10G10B10_SSCALED: name = "A2R10G10B10_SSCALED"; break;
		case LiteFX::Rendering::Format::A2R10G10B10_UINT: name = "A2R10G10B10_UINT"; break;
		case LiteFX::Rendering::Format::A2R10G10B10_SINT: name = "A2R10G10B10_SINT"; break;
		case LiteFX::Rendering::Format::A2B10G10R10_UNORM: name = "A2B10G10R10_UNORM"; break;
		case LiteFX::Rendering::Format::A2B10G10R10_SNORM: name = "A2B10G10R10_SNORM"; break;
		case LiteFX::Rendering::Format::A2B10G10R10_USCALED: name = "A2B10G10R10_USCALED"; break;
		case LiteFX::Rendering::Format::A2B10G10R10_SSCALED: name = "A2B10G10R10_SSCALED"; break;
		case LiteFX::Rendering::Format::A2B10G10R10_UINT: name = "A2B10G10R10_UINT"; break;
		case LiteFX::Rendering::Format::A2B10G10R10_SINT: name = "A2B10G10R10_SINT"; break;
		case LiteFX::Rendering::Format::R16_UNORM: name = "R16_UNORM"; break;
		case LiteFX::Rendering::Format::R16_SNORM: name = "R16_SNORM"; break;
		case LiteFX::Rendering::Format::R16_USCALED: name = "R16_USCALED"; break;
		case LiteFX::Rendering::Format::R16_SSCALED: name = "R16_SSCALED"; break;
		case LiteFX::Rendering::Format::R16_UINT: name = "R16_UINT"; break;
		case LiteFX::Rendering::Format::R16_SINT: name = "R16_SINT"; break;
		case LiteFX::Rendering::Format::R16_SFLOAT: name = "R16_SFLOAT"; break;
		case LiteFX::Rendering::Format::R16G16_UNORM: name = "R16G16_UNORM"; break;
		case LiteFX::Rendering::Format::R16G16_SNORM: name = "R16G16_SNORM"; break;
		case LiteFX::Rendering::Format::R16G16_USCALED: name = "R16G16_USCALED"; break;
		case LiteFX::Rendering::Format::R16G16_SSCALED: name = "R16G16_SSCALED"; break;
		case LiteFX::Rendering::Format::R16G16_UINT: name = "R16G16_UINT"; break;
		case LiteFX::Rendering::Format::R16G16_SINT: name = "R16G16_SINT"; break;
		case LiteFX::Rendering::Format::R16G16_SFLOAT: name = "R16G16_SFLOAT"; break;
		case LiteFX::Rendering::Format::R16G16B16_UNORM: name = "R16G16B16_UNORM"; break;
		case LiteFX::Rendering::Format::R16G16B16_SNORM: name = "R16G16B16_SNORM"; break;
		case LiteFX::Rendering::Format::R16G16B16_USCALED: name = "R16G16B16_USCALED"; break;
		case LiteFX::Rendering::Format::R16G16B16_SSCALED: name = "R16G16B16_SSCALED"; break;
		case LiteFX::Rendering::Format::R16G16B16_UINT: name = "R16G16B16_UINT"; break;
		case LiteFX::Rendering::Format::R16G16B16_SINT: name = "R16G16B16_SINT"; break;
		case LiteFX::Rendering::Format::R16G16B16_SFLOAT: name = "R16G16B16_SFLOAT"; break;
		case LiteFX::Rendering::Format::R16G16B16A16_UNORM: name = "R16G16B16A16_UNORM"; break;
		case LiteFX::Rendering::Format::R16G16B16A16_SNORM: name = "R16G16B16A16_SNORM"; break;
		case LiteFX::Rendering::Format::R16G16B16A16_USCALED: name = "R16G16B16A16_USCALED"; break;
		case LiteFX::Rendering::Format::R16G16B16A16_SSCALED: name = "R16G16B16A16_SSCALED"; break;
		case LiteFX::Rendering::Format::R16G16B16A16_UINT: name = "R16G16B16A16_UINT"; break;
		case LiteFX::Rendering::Format::R16G16B16A16_SINT: name = "R16G16B16A16_SINT"; break;
		case LiteFX::Rendering::Format::R16G16B16A16_SFLOAT: name = "R16G16B16A16_SFLOAT"; break;
		case LiteFX::Rendering::Format::R32_UINT: name = "R32_UINT"; break;
		case LiteFX::Rendering::Format::R32_SINT: name = "R32_SINT"; break;
		case LiteFX::Rendering::Format::R32_SFLOAT: name = "R32_SFLOAT"; break;
		case LiteFX::Rendering::Format::R32G32_UINT: name = "R32G32_UINT"; break;
		case LiteFX::Rendering::Format::R32G32_SINT: name = "R32G32_SINT"; break;
		case LiteFX::Rendering::Format::R32G32_SFLOAT: name = "R32G32_SFLOAT"; break;
		case LiteFX::Rendering::Format::R32G32B32_UINT: name = "R32G32B32_UINT"; break;
		case LiteFX::Rendering::Format::R32G32B32_SINT: name = "R32G32B32_SINT"; break;
		case LiteFX::Rendering::Format::R32G32B32_SFLOAT: name = "R32G32B32_SFLOAT"; break;
		case LiteFX::Rendering::Format::R32G32B32A32_UINT: name = "R32G32B32A32_UINT"; break;
		case LiteFX::Rendering::Format::R32G32B32A32_SINT: name = "R32G32B32A32_SINT"; break;
		case LiteFX::Rendering::Format::R32G32B32A32_SFLOAT: name = "R32G32B32A32_SFLOAT"; break;
		case LiteFX::Rendering::Format::R64_UINT: name = "R64_UINT"; break;
		case LiteFX::Rendering::Format::R64_SINT: name = "R64_SINT"; break;
		case LiteFX::Rendering::Format::R64_SFLOAT: name = "R64_SFLOAT"; break;
		case LiteFX::Rendering::Format::R64G64_UINT: name = "R64G64_UINT"; break;
		case LiteFX::Rendering::Format::R64G64_SINT: name = "R64G64_SINT"; break;
		case LiteFX::Rendering::Format::R64G64_SFLOAT: name = "R64G64_SFLOAT"; break;
		case LiteFX::Rendering::Format::R64G64B64_UINT: name = "R64G64B64_UINT"; break;
		case LiteFX::Rendering::Format::R64G64B64_SINT: name = "R64G64B64_SINT"; break;
		case LiteFX::Rendering::Format::R64G64B64_SFLOAT: name = "R64G64B64_SFLOAT"; break;
		case LiteFX::Rendering::Format::R64G64B64A64_UINT: name = "R64G64B64A64_UINT"; break;
		case LiteFX::Rendering::Format::R64G64B64A64_SINT: name = "R64G64B64A64_SINT"; break;
		case LiteFX::Rendering::Format::R64G64B64A64_SFLOAT: name = "R64G64B64A64_SFLOAT"; break;
		case LiteFX::Rendering::Format::B10G11R11_UFLOAT: name = "B10G11R11_UFLOAT"; break;
		case LiteFX::Rendering::Format::E5B9G9R9_UFLOAT: name = "E5B9G9R9_UFLOAT"; break;
		case LiteFX::Rendering::Format::D16_UNORM: name = "D16_UNORM"; break;
		case LiteFX::Rendering::Format::X8_D24_UNORM: name = "X8_D24_UNORM"; break;
		case LiteFX::Rendering::Format::D32_SFLOAT: name = "D32_SFLOAT"; break;
		case LiteFX::Rendering::Format::S8_UINT: name = "S8_UINT"; break;
		case LiteFX::Rendering::Format::D16_UNORM_S8_UINT: name = "D16_UNORM_S8_UINT"; break;
		case LiteFX::Rendering::Format::D24_UNORM_S8_UINT: name = "D24_UNORM_S8_UINT"; break;
		case LiteFX::Rendering::Format::D32_SFLOAT_S8_UINT: name = "D32_SFLOAT_S8_UINT"; break;
		case LiteFX::Rendering::Format::BC1_RGB_UNORM: name = "BC1_RGB_UNORM"; break;
		case LiteFX::Rendering::Format::BC1_RGB_SRGB: name = "BC1_RGB_SRGB"; break;
		case LiteFX::Rendering::Format::BC1_RGBA_UNORM: name = "BC1_RGBA_UNORM"; break;
		case LiteFX::Rendering::Format::BC1_RGBA_SRGB: name = "BC1_RGBA_SRGB"; break;
		case LiteFX::Rendering::Format::BC2_UNORM: name = "BC2_UNORM"; break;
		case LiteFX::Rendering::Format::BC2_SRGB: name = "BC2_SRGB"; break;
		case LiteFX::Rendering::Format::BC3_UNORM: name = "BC3_UNORM"; break;
		case LiteFX::Rendering::Format::BC3_SRGB: name = "BC3_SRGB"; break;
		case LiteFX::Rendering::Format::BC4_UNORM: name = "BC4_UNORM"; break;
		case LiteFX::Rendering::Format::BC4_SNORM: name = "BC4_SNORM"; break;
		case LiteFX::Rendering::Format::BC5_UNORM: name = "BC5_UNORM"; break;
		case LiteFX::Rendering::Format::BC5_SNORM: name = "BC5_SNORM"; break;
		case LiteFX::Rendering::Format::BC6H_UFLOAT: name = "BC6H_UFLOAT"; break;
		case LiteFX::Rendering::Format::BC6H_SFLOAT: name = "BC6H_SFLOAT"; break;
		case LiteFX::Rendering::Format::BC7_UNORM: name = "BC7_UNORM"; break;
		case LiteFX::Rendering::Format::BC7_SRGB: name = "BC7_SRGB"; break;
		case LiteFX::Rendering::Format::None: name = "None"; break;
		case LiteFX::Rendering::Format::Other: name = "Other"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::DescriptorType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::DescriptorType t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t)
		{
		case LiteFX::Rendering::DescriptorType::Sampler: name = "Sampler"; break;
		case LiteFX::Rendering::DescriptorType::Storage: name = "Uniform"; break;
		case LiteFX::Rendering::DescriptorType::Uniform: name = "Storage"; break;
		case LiteFX::Rendering::DescriptorType::Image:   name = "Image"; break;
		case LiteFX::Rendering::DescriptorType::InputAttachment: name = "Input Attachment"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::BufferType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::BufferType t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t)
		{
		case LiteFX::Rendering::BufferType::Index:      name = "Index";      break;
		case LiteFX::Rendering::BufferType::Vertex:     name = "Vertex";     break;
		case LiteFX::Rendering::BufferType::Uniform:    name = "Uniform"; break;
		case LiteFX::Rendering::BufferType::Storage:    name = "Storage"; break;
		case LiteFX::Rendering::BufferType::Other:      name = "Other"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::BufferUsage> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::BufferUsage t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t)
		{
		case LiteFX::Rendering::BufferUsage::Staging:  name = "Staging";  break;
		case LiteFX::Rendering::BufferUsage::Resource: name = "Resource"; break;
		case LiteFX::Rendering::BufferUsage::Dynamic:  name = "Dynamic";  break;
		case LiteFX::Rendering::BufferUsage::Readback: name = "Readback"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::IndexType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::IndexType t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t)
		{
		case LiteFX::Rendering::IndexType::UInt16: name = "UInt16"; break;
		case LiteFX::Rendering::IndexType::UInt32: name = "UInt32"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::ShaderStage> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::ShaderStage t, FormatContext& ctx) {
		Array<String> names;

		if (t == LiteFX::Rendering::ShaderStage::Other)
			names.push_back("Other");
		else
		{
			if ((t & LiteFX::Rendering::ShaderStage::Vertex) == LiteFX::Rendering::ShaderStage::Vertex)
				names.push_back("Vertex");
			if ((t & LiteFX::Rendering::ShaderStage::TessellationControl) == LiteFX::Rendering::ShaderStage::TessellationControl)
				names.push_back("Tessellation Control");
			if ((t & LiteFX::Rendering::ShaderStage::TessellationEvaluation) == LiteFX::Rendering::ShaderStage::TessellationEvaluation)
				names.push_back("Tessellation Evaluation");
			if ((t & LiteFX::Rendering::ShaderStage::Geometry) == LiteFX::Rendering::ShaderStage::Geometry)
				names.push_back("Geometry");
			if ((t & LiteFX::Rendering::ShaderStage::Fragment) == LiteFX::Rendering::ShaderStage::Fragment)
				names.push_back("Fragment");
			if ((t & LiteFX::Rendering::ShaderStage::Compute) == LiteFX::Rendering::ShaderStage::Compute)
				names.push_back("Compute");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::BufferFormat> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::BufferFormat t, FormatContext& ctx) {
		Array<String> names;

		switch (::getBufferFormatChannels(t))
		{
		case 1:
			names.push_back("X");
			break;
		case 2:
			names.push_back("XY");
			break;
		case 3:
			names.push_back("XYZ");
			break;
		case 4:
			names.push_back("XYZW");
			break;
		default:
			return formatter<string_view>::format("Invalid", ctx);
		}

		switch (::getBufferFormatChannelSize(t))
		{
		case 8:
			names.push_back("8");
			break;
		case 16:
			names.push_back("16");
			break;
		case 32:
			names.push_back("32");
			break;
		case 64:
			names.push_back("64");
			break;
		default:
			return formatter<string_view>::format("Invalid", ctx);
		}

		switch (::getBufferFormatType(t))
		{
		case 0x01:
			names.push_back("F");
			break;
		case 0x02:
			names.push_back("I");
			break;
		case 0x04:
			names.push_back("S");
			break;
		default:
			return formatter<string_view>::format("Invalid", ctx);
		}

		String name = Join(names);
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::PolygonMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::PolygonMode t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::PolygonMode::Solid: name = "Solid"; break;
		case LiteFX::Rendering::PolygonMode::Wireframe: name = "Wireframe"; break;
		case LiteFX::Rendering::PolygonMode::Point: name = "Point"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::CullMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::CullMode t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::CullMode::FrontFaces: name = "FrontFaces"; break;
		case LiteFX::Rendering::CullMode::BackFaces: name = "BackFaces"; break;
		case LiteFX::Rendering::CullMode::Both: name = "Both"; break;
		case LiteFX::Rendering::CullMode::Disabled: name = "Disabled"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::CullOrder> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::CullOrder t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::CullOrder::ClockWise: name = "ClockWise"; break;
		case LiteFX::Rendering::CullOrder::CounterClockWise: name = "CounterClockWise"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::RenderTargetType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::RenderTargetType t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::RenderTargetType::Color: name = "Color"; break;
		case LiteFX::Rendering::RenderTargetType::Depth: name = "Depth"; break;
		case LiteFX::Rendering::RenderTargetType::Present: name = "Present"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::MultiSamplingLevel> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::MultiSamplingLevel t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::MultiSamplingLevel::x1: name = "1"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x2: name = "2"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x4: name = "4"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x8: name = "8"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x16: name = "16"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x32: name = "32"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x64: name = "64"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::FilterMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::FilterMode t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::FilterMode::Nearest: name = "Nearest"; break;
		case LiteFX::Rendering::FilterMode::Linear: name = "Linear"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::MipMapMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::MipMapMode t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::MipMapMode::Nearest: name = "Nearest"; break;
		case LiteFX::Rendering::MipMapMode::Linear: name = "Linear"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::BorderMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::BorderMode t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::BorderMode::Repeat: name = "Repeat"; break;
		case LiteFX::Rendering::BorderMode::ClampToEdge: name = "ClampToEdge"; break;
		case LiteFX::Rendering::BorderMode::ClampToBorder: name = "ClampToBorder"; break;
		case LiteFX::Rendering::BorderMode::RepeatMirrored: name = "RepeatMirrored"; break;
		case LiteFX::Rendering::BorderMode::ClampToEdgeMirrored: name = "ClampToEdgeMirrored"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};