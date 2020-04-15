#include <litefx/logging.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>

using namespace LiteFX::Logging;

class RollingFileSink::RollingFileSinkImpl {
private:
    String m_name, m_pattern, m_fileName;
    LogLevel m_level;
    bool m_truncate;
    int m_maxFiles;
    SharedPtr<spdlog::sinks::daily_file_sink_mt> m_sink;

public:
    RollingFileSinkImpl(const LogLevel& level, const String& name, const String& fileName, const String& pattern, bool truncate, int maxFiles) :
        m_name(name), m_pattern(pattern), m_level(level), m_fileName(fileName), m_truncate(truncate), m_maxFiles(maxFiles),
        m_sink(makeShared<spdlog::sinks::daily_file_sink_mt>(fileName, 23, 59, truncate, maxFiles)) { }

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

    const String& getFileName() const
    {
        return m_fileName;
    }

    const bool& getTruncate() const 
    {
        return m_truncate;
    }

    const int& getMaxFiles() const
    {
        return m_maxFiles;
    }

    spdlog::sink_ptr get() const
    {
        return m_sink;
    }
};

RollingFileSink::RollingFileSink(const LogLevel& level, const String& name, const String& pattern, const String& fileName, const bool& truncate, const int& maxFiles) noexcept :
    m_impl(makePimpl<RollingFileSinkImpl>(level, name, fileName, pattern, truncate, maxFiles))
{
}

RollingFileSink::~RollingFileSink() noexcept = default;

const String& RollingFileSink::getName() const
{
    return m_impl->getName();
}

const LogLevel& RollingFileSink::getLevel() const
{
    return m_impl->getLevel();
}

const String& RollingFileSink::getFileName() const
{
    return m_impl->getFileName();
}

const String& RollingFileSink::getPattern() const
{
    return m_impl->getPattern();
}

bool RollingFileSink::getTruncate() const
{
    return m_impl->getTruncate();
}

int RollingFileSink::getMaxFiles() const
{
    return m_impl->getMaxFiles();
}

spdlog::sink_ptr RollingFileSink::get() const
{
    return m_impl->get();
}