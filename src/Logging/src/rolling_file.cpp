#include <litefx/logging.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>

using namespace LiteFX::Logging;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RollingFileSink::RollingFileSinkImpl : public Implement<RollingFileSink> {
public:
    friend class RollingFileSink;

private:
    String m_pattern, m_fileName;
    LogLevel m_level;
    bool m_truncate;
    int m_maxFiles;
    SharedPtr<spdlog::sinks::daily_file_sink_mt> m_sink;

public:
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
    RollingFileSinkImpl(RollingFileSink* parent, LogLevel level, const String& fileName, const String& pattern, bool truncate, int maxFiles) :
        base(parent), m_pattern(pattern), m_fileName(fileName), m_level(level), m_truncate(truncate), m_maxFiles(maxFiles),
        m_sink(makeShared<spdlog::sinks::daily_file_sink_mt>(fileName, 23, 59, truncate, maxFiles)) 
    {
        m_sink->set_level(static_cast<spdlog::level::level_enum>(level));
        m_sink->set_pattern(pattern);
    }
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

RollingFileSink::RollingFileSink(const String& fileName, LogLevel level, const String& pattern, bool truncate, int maxFiles) :
    m_impl(makePimpl<RollingFileSinkImpl>(this, level, fileName, pattern, truncate, maxFiles))
{
}

RollingFileSink::~RollingFileSink() noexcept = default;

String RollingFileSink::getName() const
{
    return "spdlog::sinks::daily_file_sink_mt";
}

LogLevel RollingFileSink::getLevel() const
{
    return m_impl->m_level;
}

String RollingFileSink::getFileName() const
{
    return m_impl->m_fileName;
}

String RollingFileSink::getPattern() const
{
    return m_impl->m_pattern;
}

bool RollingFileSink::getTruncate() const
{
    return m_impl->m_truncate;
}

int RollingFileSink::getMaxFiles() const
{
    return m_impl->m_maxFiles;
}

spdlog::sink_ptr RollingFileSink::get() const
{
    return m_impl->m_sink;
}