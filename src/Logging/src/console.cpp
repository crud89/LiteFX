#include <litefx/logging.hpp>
#include <spdlog/sinks/ansicolor_sink-inl.h>

using namespace LiteFX::Logging;

class ConsoleSink::ConsoleSinkImpl : public Implement<ConsoleSink> {
public:
    friend class ConsoleSink;

private:
    String m_pattern;
    LogLevel m_level;
    SharedPtr<spdlog::sinks::ansicolor_stdout_sink_mt> m_sink;

public:
    ConsoleSinkImpl(ConsoleSink* parent, const LogLevel& level, const String& pattern) : 
        base(parent), m_pattern(pattern), m_level(level), m_sink(makeShared<spdlog::sinks::ansicolor_stdout_sink_mt>()) 
    { 
        m_sink->set_level(static_cast<spdlog::level::level_enum>(level));
        m_sink->set_pattern(pattern);
    }
};

ConsoleSink::ConsoleSink(const LogLevel& level, const String& pattern) :
    m_impl(makePimpl<ConsoleSinkImpl>(this, level, pattern))
{
}

ConsoleSink::~ConsoleSink() noexcept = default;

String ConsoleSink::getName() const
{
    return "spdlog::sinks::ansicolor_stdout_sink_mt";
}

LogLevel ConsoleSink::getLevel() const
{
    return m_impl->m_level;
}

String ConsoleSink::getPattern() const
{
    return m_impl->m_pattern;
}

spdlog::sink_ptr ConsoleSink::get() const
{
    return m_impl->m_sink;
}