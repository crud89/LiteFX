#include <litefx/logging.hpp>
#include <spdlog/sinks/base_sink.h>

using namespace LiteFX::Logging;

// ------------------------------------------------------------------------------------------------
// Sink implementation.
// ------------------------------------------------------------------------------------------------

class termination_sink : public spdlog::sinks::base_sink<std::mutex> {
private:
    spdlog::level::level_enum m_minLevel;
    int m_status;

public:
    termination_sink(int status, const spdlog::level::level_enum& minLevel = spdlog::level::level_enum::err);
    termination_sink(const termination_sink&) = delete;
    termination_sink(termination_sink&&) = delete;
    virtual ~termination_sink() = default;

public:
    virtual void sink_it_(const spdlog::details::log_msg& msg) override;
    virtual void flush_() override;
};

termination_sink::termination_sink(int status, const spdlog::level::level_enum& minLevel) :
    m_status(status), m_minLevel(minLevel)
{
}

void termination_sink::sink_it_(const spdlog::details::log_msg& msg)
{
    if (msg.level >= m_minLevel)
    {
        ::OutputDebugString(msg.payload.data());
        ::exit(m_status);
    }
}

void termination_sink::flush_()
{
    // Nothing to do here.
}

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class TerminationSink::TerminationSinkImpl : public Implement<TerminationSink> {
public:
    friend class TerminationSink;

private:
    String m_pattern;
    LogLevel m_level;
    SharedPtr<termination_sink> m_sink;

public:
    TerminationSinkImpl(TerminationSink* parent, const LogLevel& level, int status) :
        base(parent), m_level(level), m_sink(makeShared<termination_sink>(status))
    {
        m_sink->set_level(static_cast<spdlog::level::level_enum>(level));
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

TerminationSink::TerminationSink(const LogLevel& level, int status) :
    m_impl(makePimpl<TerminationSinkImpl>(this, level, status))
{
}

TerminationSink::~TerminationSink() noexcept = default;

String TerminationSink::getName() const
{
    return "::termination_sink";
}

LogLevel TerminationSink::getLevel() const
{
    return m_impl->m_level;
}

String TerminationSink::getPattern() const
{
    return "";
}

spdlog::sink_ptr TerminationSink::get() const
{
    return m_impl->m_sink;
}