#include <litefx/rendering_api.hpp>

using namespace LiteFX::Rendering;

const IShaderModule* ShaderRecordCollection::findShaderModule(StringView name) const noexcept
{
	return m_program[name];
}

const Array<UniquePtr<const IShaderRecord>>& ShaderRecordCollection::shaderRecords() const noexcept
{
	return m_records;
}

void ShaderRecordCollection::addShaderRecord(UniquePtr<const IShaderRecord>&& record)
{
	if (record == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("record", "The shader record was not initialized.");

	const auto& group = record->shaderGroup();
	
	if (std::holds_alternative<IShaderRecord::MeshGeometryHitGroup>(group))
	{
		auto& hitGroup = std::get<IShaderRecord::MeshGeometryHitGroup>(group);
		
		if (hitGroup.AnyHitShader == nullptr && hitGroup.ClosestHitShader == nullptr) [[unlikely]]
			throw InvalidArgumentException("record", "The record contains an empty mesh geometry hit group.");

		if (hitGroup.AnyHitShader != nullptr && hitGroup.AnyHitShader->type() != ShaderStage::AnyHit) [[unlikely]]
			throw InvalidArgumentException("record", "The record contains a mesh geometry hit group, but the any hit shader in it has the wrong type.");

		if (hitGroup.AnyHitShader != nullptr && !m_program.contains(*hitGroup.AnyHitShader)) [[unlikely]]
			throw InvalidArgumentException("record", "The record contains a mesh geometry hit group, but the any hit shader does not belong to the shader record collection parent program (module: {0}).", hitGroup.AnyHitShader->fileName());

		if (hitGroup.ClosestHitShader != nullptr && hitGroup.ClosestHitShader->type() != ShaderStage::ClosestHit) [[unlikely]]
			throw InvalidArgumentException("record", "The record contains a mesh geometry hit group, but the closest hit shader in it has the wrong type.");

		if (hitGroup.ClosestHitShader != nullptr && !m_program.contains(*hitGroup.ClosestHitShader)) [[unlikely]]
			throw InvalidArgumentException("record", "The record contains a mesh geometry hit group, but the closest hit shader does not belong to the shader record collection parent program (module: {0}).", hitGroup.ClosestHitShader->fileName());
	}
	else if (std::holds_alternative<const IShaderModule*>(group))
	{
		auto module = std::get<const IShaderModule*>(group);

		if (module == nullptr) [[unlikely]]
			throw InvalidArgumentException("record", "The record does not contain a shader module.");
		
		if (module->type() != ShaderStage::RayGeneration && module->type() != ShaderStage::Intersection && module->type() != ShaderStage::Miss && module->type() != ShaderStage::Callable) [[unlikely]]
			throw InvalidArgumentException("record", "The record does contain a shader module, but it has not a valid ray-tracing type.");
		
		if (!m_program.contains(*module)) [[unlikely]]
			throw InvalidArgumentException("record", "The record does contain a shader module, but it does not belong to the shader record collection parent program (module: {0}).", module->fileName());
	}
	else
	{
		// ... as long as there's no new shader group.
		std::unreachable();
	}

	m_records.push_back(std::move(record));
}

void ShaderRecordCollection::computeShaderTableSizes(UInt64& rayGenGroupSize, UInt64& hitGroupSize, UInt64& missGroupSize, UInt64& callableGroupSize) const noexcept
{
	throw;
}