#include <litefx/logging.hpp>
#include <spdlog/spdlog.h>

using namespace LiteFX::Logging;

class Logger::LoggerImpl {
private:
    static std::once_flag isInitialized;
    static UniquePtr<Logger> m_instance;

private:
    static void initialize()
    {
        m_instance = UniquePtr<Logger>();
    }

public:
    static Logger& getLogger() noexcept {
        std::call_once(LoggerImpl::isInitialized, LoggerImpl::initialize);
        return *LoggerImpl::m_instance.get();
    }
};

Log Logger::get(const String& name)
{
    // Get the log.
    auto log = spdlog::get(name);
    
    // If it does not exist, create it from the current sinks.
    if (log == nullptr)
    {
        // TODO: create log
        //LoggerImpl::getLogger().sinks()
        //spdlog::register_logger(logger);
        throw;
    }

    return Log(name);
}

template <typename TSink, std::enable_if_t<std::is_convertible_v<TSink*, ISink*>, int>, typename ...TArgs>
static void Logger::sinkTo(const LogLevel& level, const String& name, const String& pattern, TArgs&&... args)
{
    //LoggerImpl::getLogger().sinks().push_back(TSink(level, name, pattern, std::forward<TArgs>(args)...));
    throw;
}