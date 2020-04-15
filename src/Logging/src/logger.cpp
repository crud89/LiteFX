#include <litefx/logging.hpp>
#include <spdlog/spdlog.h>

using namespace LiteFX::Logging;

class Log::LogImpl {
private:
    String m_name;

public:
    LogImpl(const String& name) noexcept : m_name(name) { }

public:
    const String& getName() const noexcept
    {
        return m_name;
    }
};

Log::Log(const String& name) noexcept :
    m_impl(makePimpl<LogImpl>(name))
{
}

Log::~Log() noexcept = default;

const String& Log::getName() const noexcept
{
    return m_impl->getName();
}

void Log::log(const LogLevel& level, const String& message)
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