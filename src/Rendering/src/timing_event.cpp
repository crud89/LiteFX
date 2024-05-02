#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class TimingEvent::TimingEventImpl : public Implement<TimingEvent> {
public:
	friend class TimingEvent;

private:
	String m_name;
	const ISwapChain& m_swapChain;

public:
	TimingEventImpl(TimingEvent* parent, const ISwapChain& swapChain, StringView name) :
		base(parent), m_swapChain(swapChain), m_name(name)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

TimingEvent::TimingEvent(const ISwapChain& swapChain, StringView name) noexcept :
	m_impl(makePimpl<TimingEventImpl>(this, swapChain, name))
{
}

TimingEvent::~TimingEvent() noexcept = default;

String TimingEvent::name() const noexcept
{
	return m_impl->m_name;
}

UInt64 TimingEvent::readTimestamp() const noexcept
{
	return m_impl->m_swapChain.readTimingEvent(this->shared_from_this());
}

UInt32 TimingEvent::queryId() const
{
	return m_impl->m_swapChain.resolveQueryId(this->shared_from_this());
}