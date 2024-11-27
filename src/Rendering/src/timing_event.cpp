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
	const ISwapChain& m_swapChain; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

public:
	TimingEventImpl(const ISwapChain& swapChain, StringView name) :
		m_name(name), m_swapChain(swapChain)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

TimingEvent::TimingEvent(const ISwapChain& swapChain, StringView name) noexcept :
	m_impl(swapChain, name)
{
}

TimingEvent::~TimingEvent() noexcept = default;

StringView TimingEvent::name() const noexcept
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