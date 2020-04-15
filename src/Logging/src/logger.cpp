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

template<typename ...TArgs>
void Log::log(const LogLevel& level, const String& format, TArgs&&... args)
{
    spdlog::memory_buf_t message;
    fmt::format_to(message, format, std::forward<TArgs>(args)...);

    this->log(level, message);
}

template<typename ...TArgs>
void Log::trace(const String& format, TArgs&&... args)
{
    return this->log(LogLevel::Trace, format, std::forward<TArgs>(_args)...);
}

template<typename ...TArgs>
void Log::debug(const String& format, TArgs&&... args)
{
    return this->log(LogLevel::Debug, format, std::forward<TArgs>(_args)...);
}

template<typename ...TArgs>
void Log::info(const String& format, TArgs&&... args)
{
    return this->log(LogLevel::Info, format, std::forward<TArgs>(_args)...);
}

template<typename ...TArgs>
void Log::warning(const String& format, TArgs&&... args)
{
    return this->log(LogLevel::Warning, format, std::forward<TArgs>(_args)...);
}

template<typename ...TArgs>
void Log::error(const String& format, TArgs&&... args)
{
    return this->log(LogLevel::Error, format, std::forward<TArgs>(_args)...);
}

template<typename ...TArgs>
void Log::fatal(const String& format, TArgs&&... args)
{
    return this->log(LogLevel::Fatal, format, std::forward<TArgs>(_args)...);
}