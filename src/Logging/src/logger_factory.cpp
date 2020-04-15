#include <litefx/logging.hpp>
#include <spdlog/spdlog.h>

using namespace LiteFX::Logging;

static std::once_flag isInitialized;
static UniquePtr<Logger> m_instance;

class Logger::LoggerImpl {
private:
    Array<spdlog::sink_ptr> m_sinks;

private:
    static void initialize() { m_instance = UniquePtr<Logger>(); }

public:
    static Logger& getLogger() noexcept {
        std::call_once(isInitialized, LoggerImpl::initialize);
        return *m_instance.get();
    }

    static Array<spdlog::sink_ptr>& getSinks() noexcept {
        std::call_once(isInitialized, LoggerImpl::initialize);
        return m_instance->m_impl->m_sinks;
    }
};

Log Logger::get(const String& name)
{
    // Get the log.
    auto log = spdlog::get(name);
    
    // If it does not exist, create it from the current sinks.
    if (log == nullptr)
    {
        const auto& sinks = LoggerImpl::getSinks();
        spdlog::register_logger(makeShared<spdlog::logger>(name, std::begin(sinks), std::end(sinks)));
    }

    return Log(name);
}

template <typename TSink, std::enable_if_t<std::is_convertible_v<TSink*, ISink*>, int>, typename ...TArgs>
static void Logger::sinkTo(const LogLevel& level, const String& name, const String& pattern, TArgs&&... args)
{
    LoggerImpl::getSinks().push_back(TSink(level, name, pattern, std::forward<TArgs>(args)...));
}