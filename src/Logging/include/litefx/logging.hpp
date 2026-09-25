#pragma once

#include <litefx/config.h>
#include <litefx/core.h>
#include <litefx/logging_export.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>

namespace LiteFX::Logging {
    using namespace LiteFX;

    /// @brief Defines the various log levels.
    enum class LogLevel : std::uint8_t {
        Trace = SPDLOG_LEVEL_TRACE,
        Debug = SPDLOG_LEVEL_DEBUG,
        Info = SPDLOG_LEVEL_INFO,
        Warning = SPDLOG_LEVEL_WARN,
        Error = SPDLOG_LEVEL_ERROR,
        Fatal = SPDLOG_LEVEL_CRITICAL,
        Off = SPDLOG_LEVEL_OFF,
        Invalid = 0xFF
    };

    /// @brief Interface for a class that receives log messages.
    class LITEFX_LOGGING_API ISink {
    protected:
        ISink() noexcept = default;
        ISink(const ISink&) = default;
        ISink(ISink&&) noexcept = default;
        ISink& operator=(const ISink&) = default;
        ISink& operator=(ISink&&) noexcept = default;

    public:
        virtual ~ISink() noexcept = default;

        /// @brief Gets the minimum log level for messages to get written to the log.
        virtual LogLevel getLevel() const = 0;

        /// @brief Gets the name of the sink.
        virtual String getName() const = 0;

        /// @brief Gets the pattern used to format the messages for the sink.
        virtual String getPattern() const = 0;

    protected:
        friend class Logger;
        virtual spdlog::sink_ptr get() const = 0;
    };

    /// @brief Writes log messages to the console.
    class LITEFX_LOGGING_API ConsoleSink : public ISink {
        LITEFX_IMPLEMENTATION(ConsoleSinkImpl);

    public:
        /// @brief Creates a new console sink instance.
        ///
        /// @param level The minimum log level for messages to be displayed on the console.
        /// @param pattern The default format for log messages.
        ConsoleSink(LogLevel level = LogLevel::Info, const String& pattern = "%+");
        ~ConsoleSink() noexcept override;

        ConsoleSink(const ConsoleSink&) = delete;
        ConsoleSink(ConsoleSink&&) noexcept = delete;
        auto operator=(const ConsoleSink&) = delete;
        auto operator=(ConsoleSink&&) noexcept = delete;

    public:
        /// @copydoc ISink::getLevel
        LogLevel getLevel() const override;

        /// @copydoc ISink::getName
        String getName() const override;

        /// @copydoc ISink::getPattern
        String getPattern() const override;

    protected:
        spdlog::sink_ptr get() const override;
    };

    /// @brief Writes log messages to a rolling file.
    class LITEFX_LOGGING_API RollingFileSink : public ISink {
        LITEFX_IMPLEMENTATION(RollingFileSinkImpl);

    public:
        /// @brief Creates a new rolling file sink instance.
        ///
        /// @param fileName The name of the log file.
        /// @param level The minimum log level for messages to be saved to the file.
        /// @param pattern The default format for log messages.
        /// @param truncate `true`, if the log messages should be truncated to the contents of the file. `false` to overwrite existing messages.
        /// @param maxFiles The maximum number of files to keep.
        RollingFileSink(const String& fileName, LogLevel level = LogLevel::Info, const String& pattern = "%+", bool truncate = false, int maxFiles = 0);
        ~RollingFileSink() noexcept override;

        RollingFileSink(const RollingFileSink&) = delete;
        RollingFileSink(RollingFileSink&&) noexcept = delete;
        auto operator=(const RollingFileSink&) = delete;
        auto operator=(RollingFileSink&&) noexcept = delete;

    public:
        /// @copydoc ISink::getLevel
        LogLevel getLevel() const override;

        /// @copydoc ISink::getName
        String getName() const override;

        /// @copydoc ISink::getPattern
        String getPattern() const override;

        /// @brief Gets the file name of the log file.
        ///
        /// @return The file name of the log file.
        virtual String getFileName() const;

        /// @brief Returns `true`, if the log messages should be truncated to the contents of the file, or `false` if existing messages are overwritten.
        ///
        /// @return `true`, if the log messages should be truncated to the contents of the file, or `false` if existing messages are overwritten.
        virtual bool getTruncate() const;

        /// @brief Returns the maximum number of log files to keep.
        ///
        /// @return The maximum number of log files to keep.
        virtual int getMaxFiles() const;

    protected:
        spdlog::sink_ptr get() const override;
    };

    /// @brief Forcefully terminates the application, if a log message of a certain level or higher is output.
    ///
    /// The purpose of this sink is to allow tests to catch errors during application runtime. You probably do not want to use this sink in an actual application.
    class LITEFX_LOGGING_API TerminationSink : public ISink {
        LITEFX_IMPLEMENTATION(TerminationSinkImpl);

    public:
        /// @brief The default status code used to exit the application.
        static constexpr std::uint32_t DEFAULT_TERMINATION_STATUS = 0xFF455252;

    public:
        /// @brief Creates a new exception sink instance.
        ///
        /// @param level The minimum log level at which an exception is thrown.
        /// @param status The status code used to exit the application.
        TerminationSink(const LogLevel& level = LogLevel::Info, int status = static_cast<int>(DEFAULT_TERMINATION_STATUS));
        ~TerminationSink() noexcept override;

        TerminationSink(const TerminationSink&) = delete;
        TerminationSink(TerminationSink&&) noexcept = delete;
        auto operator=(const TerminationSink&) = delete;
        auto operator=(TerminationSink&&) noexcept = delete;

    public:
        /// @copydoc ISink::getLevel
        LogLevel getLevel() const override;

        /// @copydoc ISink::getName
        String getName() const override;

        /// @copydoc ISink::getPattern
        String getPattern() const override;

    protected:
        spdlog::sink_ptr get() const override;
    };

    /// @brief A log to which messages are written to.
    ///
    /// Note that in release builds, message at @ref LogLevel::Trace and @ref LogLevel::Debug are not forwarded and the corresponding functions are disabled. If you *really* want to log such messages, you
    /// have to specify the log level explicitly by calling @ref Log::log.
    class LITEFX_LOGGING_API Log {
        LITEFX_IMPLEMENTATION(LogImpl);

    public:
        /// @brief Creates a new log instance.
        ///
        /// @param name The name of the log.
        Log(const String& name);
        virtual ~Log() noexcept;

        Log(Log&&) noexcept = delete;
        Log(const Log&) = delete;
        auto operator=(Log&&) noexcept = delete;
        auto operator=(const Log&) = delete;

    public:
        /// @brief Gets the name of the logger.
        virtual const String& getName() const noexcept;

    protected:
        virtual void log(LogLevel level, StringView message);

    public:
        /// @brief Logs a message of @p level with @p format.
        ///
        /// @param level The log level of the message.
        /// @param format The format of the message.
        template<typename ...TArgs>
        inline void log(LogLevel level, std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(level, std::format(format, std::forward<TArgs>(args)...));
        }

        /// @brief Logs a trace message with @p format.
        ///
        /// @param format The format of the message.
        template<typename ...TArgs>
        inline void trace([[maybe_unused]] std::format_string<TArgs...> format, [[maybe_unused]] TArgs&&... args) {
#ifndef NDEBUG
            this->log(LogLevel::Trace, format, std::forward<TArgs>(args)...);
#endif
        }

        /// @brief Logs a debug message with @p format.
        ///
        /// @param format The format of the message.
        template<typename ...TArgs>
        inline void debug([[maybe_unused]] std::format_string<TArgs...> format, [[maybe_unused]] TArgs&&... args) {
#ifndef NDEBUG
            this->log(LogLevel::Debug, format, std::forward<TArgs>(args)...);
#endif
        }

        /// @brief Logs an info message with @p format.
        ///
        /// @param format The format of the message.
        template<typename ...TArgs>
        inline void info(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Info, format, std::forward<TArgs>(args)...);
        }

        /// @brief Logs a warning message with @p format.
        ///
        /// @param format The format of the message.
        template<typename ...TArgs>
        inline void warning(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Warning, format, std::forward<TArgs>(args)...);
        }

        /// @brief Logs an error message with @p format.
        ///
        /// @param format The format of the message.
        template<typename ...TArgs>
        inline void error(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Error, format, std::forward<TArgs>(args)...);
        }

        /// @brief Logs a fatal error message with @p format.
        ///
        /// @param format The format of the message.
        template<typename ...TArgs>
        inline void fatal(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Fatal, format, std::forward<TArgs>(args)...);
        }
    };

    /// @brief A provider for @ref Log instances.
    class LITEFX_LOGGING_API Logger {
    private:
        Logger() noexcept;

    public:
        virtual ~Logger() noexcept;

        Logger(Logger&&) noexcept = delete;
        Logger(const Logger&) = delete;
        auto operator=(const Logger&) = delete;
        auto operator=(Logger&&) noexcept = delete;

        // TODO: Cache logs by name and return them, instead of re-creating them with each call.
    public:
        /// @brief Retrieves a log from @p name.
        ///
        /// @param name The name of the log to query.
        /// @return A instance of a log.
        static Log get(StringView name);

        /// @brief Allows a log to write messages to @p sink.
        ///
        /// @param sink The sink to write log messages to.
        /// @throws InvalidArgumentException Thrown, if @p sink is not initialized.
        static void sinkTo(const ISink* sink);
    };

}

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#ifndef NDEBUG
#define LITEFX_TRACE(log, format, ...) LiteFX::Logging::Logger::get(log).trace(format, ##__VA_ARGS__)
#define LITEFX_DEBUG(log, format, ...) LiteFX::Logging::Logger::get(log).debug(format, ##__VA_ARGS__)
#else
#define LITEFX_TRACE(log, format, ...) 
#define LITEFX_DEBUG(log, format, ...) 
#endif

#define LITEFX_INFO(log, format, ...) LiteFX::Logging::Logger::get(log).info(format, ##__VA_ARGS__)
#define LITEFX_WARNING(log, format, ...) LiteFX::Logging::Logger::get(log).warning(format, ##__VA_ARGS__)
#define LITEFX_ERROR(log, format, ...) LiteFX::Logging::Logger::get(log).error(format, ##__VA_ARGS__)
#define LITEFX_FATAL_ERROR(log, format, ...) LiteFX::Logging::Logger::get(log).fatal(format, ##__VA_ARGS__)

// NOLINTEND(cppcoreguidelines-macro-usage)