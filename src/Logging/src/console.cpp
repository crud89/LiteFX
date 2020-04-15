#include <litefx/logging.hpp>
#include <spdlog/sinks/ansicolor_sink-inl.h>

using namespace LiteFX::Logging;

class ConsoleSink::ConsoleSinkImpl {
private:
    String m_name, m_pattern;
    LogLevel m_level;
    SharedPtr<spdlog::sinks::ansicolor_stdout_sink_mt> m_sink;

public:
    ConsoleSinkImpl(const LogLevel& level, const String& name, const String& pattern) :
        m_name(name), m_pattern(pattern), m_level(level), m_sink(makeShared<spdlog::sinks::ansicolor_stdout_sink_mt>()) { }

public:
    const LogLevel& getLevel() const
    {
        return m_level;
    }

    const String& getName() const
    {
        return m_name;
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

ConsoleSink::ConsoleSink(const LogLevel& level, const String& name, const String& pattern) noexcept :
    m_impl(makePimpl<ConsoleSinkImpl>(level, name, pattern))
{
}

ConsoleSink::~ConsoleSink() noexcept = default;

const String& ConsoleSink::getName() const
{
    return m_impl->getName();
}

const LogLevel& ConsoleSink::getLevel() const
{
    return m_impl->getLevel();
}

const String& ConsoleSink::getPattern() const
{
    return m_impl->getPattern();
}

spdlog::sink_ptr ConsoleSink::get() const
{
    return m_impl->get();
}