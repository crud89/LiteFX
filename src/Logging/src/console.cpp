#include <litefx/logging.hpp>
#include <spdlog/sinks/ansicolor_sink-inl.h>

using namespace LiteFX::Logging;

class ConsoleSink::ConsoleSinkImpl {
private:
    String m_pattern;
    LogLevel m_level;
    SharedPtr<spdlog::sinks::ansicolor_stdout_sink_mt> m_sink;

public:
    ConsoleSinkImpl(const LogLevel& level, const String& pattern) : 
        m_pattern(pattern), m_level(level), m_sink(makeShared<spdlog::sinks::ansicolor_stdout_sink_mt>()) 
    { 
        m_sink->set_level(static_cast<spdlog::level::level_enum>(level));
        m_sink->set_pattern(pattern);
    }

public:
    const LogLevel& getLevel() const
    {
        return m_level;
    }

    const String& getPattern() const
    {
        return m_pattern;
    }

    spdlog::sink_ptr get() const
    {
        return m_sink;
    }
};

ConsoleSink::ConsoleSink(const LogLevel& level, const String& pattern) noexcept :
    m_impl(makePimpl<ConsoleSinkImpl>(level, pattern))
{
}

ConsoleSink::~ConsoleSink() noexcept = default;

String ConsoleSink::getName() const
{
    return "spdlog::sinks::ansicolor_stdout_sink_mt";
}

LogLevel ConsoleSink::getLevel() const
{
    return m_impl->getLevel();
}

String ConsoleSink::getPattern() const
{
    return m_impl->getPattern();
}

spdlog::sink_ptr ConsoleSink::get() const
{
    return m_impl->get();
}