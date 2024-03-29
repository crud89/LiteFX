#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Surface::DirectX12Surface(const HWND& handle) noexcept :
	Resource<HWND>(handle)
{
}

DirectX12Surface::~DirectX12Surface() noexcept
{
}