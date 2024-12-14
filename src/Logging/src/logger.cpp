#include <litefx/logging.hpp>
#include <spdlog/spdlog.h>

using namespace LiteFX::Logging;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class Log::LogImpl {
public:
    friend class Log;

private:
    String m_name;

public:
    LogImpl(String name) : 
        m_name(std::move(name)) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Log::Log(const String& name) :
    m_impl(name)
{
}

Log::~Log() noexcept = default;

const String& Log::getName() const noexcept
{
    return m_impl->m_name;
}

void Log::log(LogLevel level, StringView message)
{
    auto logger = spdlog::get(this->getName());
    assert(logger != nullptr);

    switch (level)
    {
    case LogLevel::Trace:
        logger->log(spdlog::level::trace, message);
        break;
    case LogLevel::Debug:
        logger->log(spdlog::level::debug, message);
        break;
    case LogLevel::Info:
        logger->log(spdlog::level::info, message);
        break;
    case LogLevel::Warning:
        logger->log(spdlog::level::warn, message);
        break;
    case LogLevel::Error:
        logger->log(spdlog::level::err, message);
        break;
    case LogLevel::Fatal:
        logger->log(spdlog::level::critical, message);
        break;
    default:
        throw std::invalid_argument("The specified log level is not valid.");
    }
}