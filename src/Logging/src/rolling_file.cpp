#include <litefx/logging.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>

using namespace LiteFX::Logging;

class RollingFileSink::RollingFileSinkImpl {
private:
    String m_pattern, m_fileName;
    LogLevel m_level;
    bool m_truncate;
    int m_maxFiles;
    SharedPtr<spdlog::sinks::daily_file_sink_mt> m_sink;

public:
    RollingFileSinkImpl(const LogLevel& level, const String& fileName, const String& pattern, bool truncate, int maxFiles) :
        m_pattern(pattern), m_level(level), m_fileName(fileName), m_truncate(truncate), m_maxFiles(maxFiles),
        m_sink(makeShared<spdlog::sinks::daily_file_sink_mt>(fileName, 23, 59, truncate, maxFiles)) 
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

RollingFileSink::RollingFileSink(const String& fileName, const LogLevel& level, const String& pattern, const bool& truncate, const int& maxFiles) noexcept :
    m_impl(makePimpl<RollingFileSinkImpl>(level, fileName, pattern, truncate, maxFiles))
{
}

RollingFileSink::~RollingFileSink() noexcept = default;

String RollingFileSink::getName() const
{
    return "spdlog::sinks::daily_file_sink_mt";
}

LogLevel RollingFileSink::getLevel() const
{
    return m_impl->getLevel();
}

String RollingFileSink::getFileName() const
{
    return m_impl->getFileName();
}

String RollingFileSink::getPattern() const
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