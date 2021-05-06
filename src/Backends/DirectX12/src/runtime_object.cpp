#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RuntimeObject::DirectX12RuntimeObjectImpl : public Implement<DirectX12RuntimeObject> {
public:
	friend class DirectX12RuntimeObject;

private:
	const DirectX12Device* m_device;

public:
	DirectX12RuntimeObjectImpl(DirectX12RuntimeObject* parent, const DirectX12Device* device) : base(parent), m_device(device)
	{
		if (device == nullptr)
			throw std::invalid_argument("The device must be initialized.");
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RuntimeObject::DirectX12RuntimeObject(const DirectX12Device* device) :
	m_impl(makePimpl<DirectX12RuntimeObjectImpl>(this, device))
{
}

DirectX12RuntimeObject::~DirectX12RuntimeObject() noexcept = default;

const DirectX12Device* DirectX12RuntimeObject::getDevice() const noexcept
{
	return m_impl->m_device;
}