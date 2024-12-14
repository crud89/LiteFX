#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class TimingEvent::TimingEventImpl {
public:
	friend class TimingEvent;

private:
	String m_name;
	WeakPtr<const IGraphicsDevice> m_device;

public:
	TimingEventImpl(const ISwapChain& swapChain, StringView name) :
		m_name(name), m_device(swapChain.device().weak_from_this())
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

TimingEvent::TimingEvent(const ISwapChain& swapChain, StringView name) :
	m_impl(swapChain, name)
{
}

TimingEvent::~TimingEvent() noexcept = default;

StringView TimingEvent::name() const noexcept
{
	return m_impl->m_name;
}

UInt64 TimingEvent::readTimestamp() const
{
	if (auto device = m_impl->m_device.lock()) [[likely]]
		return device->swapChain().readTimingEvent(this->shared_from_this());
	else
		throw RuntimeException("Unable to read timing query value from released device instance.");
}

UInt32 TimingEvent::queryId() const
{
	if (auto device = m_impl->m_device.lock()) [[likely]]
		return device->swapChain().resolveQueryId(this->shared_from_this());
	else
		throw RuntimeException("Unable to obtain timing query ID from released device instance.");
}