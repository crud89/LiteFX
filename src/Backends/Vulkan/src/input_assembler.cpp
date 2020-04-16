#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanInputAssembler::VulkanInputAssemblerImpl {
private:
    UniquePtr<BufferLayout> m_layout;

public: 
    VulkanInputAssemblerImpl() noexcept = default;

public:
    const BufferLayout* getLayout() const
    {
        return m_layout.get();
    }

    void use(UniquePtr<BufferLayout>&& layout) const
    {
        m_layout = std::move(layout);
    }
};

VulkanInputAssembler::VulkanInputAssembler() noexcept : 
    m_impl(makePimpl<VulkanInputAssemblerImpl>())
{
}

VulkanInputAssembler::VulkanInputAssembler(UniquePtr<BufferLayout>&& _other) noexcept :
    m_impl(makePimpl<VulkanInputAssemblerImpl>())
{
    this->use(std::move(_other));
}

VulkanInputAssembler::~VulkanInputAssembler() noexcept = default;

const BufferLayout* VulkanInputAssembler::getLayout() const override
{
    return m_impl->getLayout();
}

void VulkanInputAssembler::use(UniquePtr<BufferLayout>&& layout) const override
{
    m_impl->use(std::move(layout));
}