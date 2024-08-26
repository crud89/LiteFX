#pragma once

#include <litefx/config.h>

#if !defined (LITEFX_LOGGING_API)
#  if defined(LiteFX_Logging_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_LOGGING_API __declspec(dllexport)
#  elif (defined(LiteFX_Logging_EXPORTS) || defined(__APPLE__)) && defined __GNUC__ && __GNUC__ >= 4
#    define LITEFX_LOGGING_API __attribute__ ((visibility ("default")))
#  elif !defined(LiteFX_Logging_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_LOGGING_API __declspec(dllimport)
#  endif
#endif 

#ifndef LITEFX_LOGGING_API
#  define LITEFX_LOGGING_API
#endif

#include <litefx/core.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>

namespace LiteFX::Logging {
    using namespace LiteFX;

    /// <summary>
    /// Defines the various log levels.
    /// </summary>
    enum class LogLevel {
        Trace = SPDLOG_LEVEL_TRACE,
        Debug = SPDLOG_LEVEL_DEBUG,
        Info = SPDLOG_LEVEL_INFO,
        Warning = SPDLOG_LEVEL_WARN,
        Error = SPDLOG_LEVEL_ERROR,
        Fatal = SPDLOG_LEVEL_CRITICAL,
        Off = SPDLOG_LEVEL_OFF,
        Invalid = 0xFF
    };

    /// <summary>
    /// Interface for a class that receives log messages.
    /// </summary>
    class LITEFX_LOGGING_API ISink {
    public:
        /// <summary>
        /// Gets the minimum log level for messages to get written to the log.
        /// </summary>
        virtual LogLevel getLevel() const = 0;

        /// <summary>
        /// Gets the name of the sink.
        /// </summary>
        virtual String getName() const = 0;

        /// <summary>
        /// Gets the pattern used to format the messages for the sink.
        /// </summary>
        virtual String getPattern() const = 0;

    protected:
        friend class Logger;
        virtual spdlog::sink_ptr get() const = 0;
    };

    /// <summary>
    /// Writes log messages to the console.
    /// </summary>
    class LITEFX_LOGGING_API ConsoleSink : public ISink {
        LITEFX_IMPLEMENTATION(ConsoleSinkImpl);

    public:
        /// <summary>
        /// Creates a new console sink instance.
        /// </summary>
        /// <param name="level">The minimum log level for messages to be displayed on the console.</param>
        /// <param name="pattern">The default format for log messages.</param>
        ConsoleSink(LogLevel level = LogLevel::Info, const String& pattern = "%+");
        ConsoleSink(const ConsoleSink&) = delete;
        ConsoleSink(ConsoleSink&&) = delete;
        virtual ~ConsoleSink() noexcept;

    public:
        /// <inheritdoc />
        LogLevel getLevel() const override;

        /// <inheritdoc />
        String getName() const override;

        /// <inheritdoc />
        String getPattern() const override;

    protected:
        spdlog::sink_ptr get() const override;
    };

    /// <summary>
    /// Writes log messages to a rolling file.
    /// </summary>
    class LITEFX_LOGGING_API RollingFileSink : public ISink {
        LITEFX_IMPLEMENTATION(RollingFileSinkImpl);

    public:
        /// <summary>
        /// Creates a new rolling file sink instance.
        /// </summary>
        /// <param name="fileName">The name of the log file.</param>
        /// <param name="level">The minimum log level for messages to be saved to the file.</param>
        /// <param name="pattern">The default format for log messages.</param>
        /// <param name="truncate">`true`, if the log messages should be truncated to the contents of the file. `false` to overwrite existing messages.</param>
        /// <param name="maxFiles">The maximum number of files to keep.</param>
        RollingFileSink(const String& fileName, LogLevel level = LogLevel::Info, const String& pattern = "%+", bool truncate = false, int maxFiles = 0);
        RollingFileSink(const RollingFileSink&) = delete;
        RollingFileSink(RollingFileSink&&) = delete;
        virtual ~RollingFileSink() noexcept;

    public:
        /// <inheritdoc />
        LogLevel getLevel() const override;

        /// <inheritdoc />
        String getName() const override;

        /// <inheritdoc />
        String getPattern() const override;

        /// <summary>
        /// Gets the file name of the log file.
        /// </summary>
        /// <returns>The file name of the log file.</returns>
        virtual String getFileName() const;

        /// <summary>
        /// Returns `true`, if the log messages should be truncated to the contents of the file, or `false` if existing messages are overwritten.
        /// </summary>
        /// <returns>`true`, if the log messages should be truncated to the contents of the file, or `false` if existing messages are overwritten.</returns>
        virtual bool getTruncate() const;

        /// <summary>
        /// Returns the maximum number of log files to keep.
        /// </summary>
        /// <returns>The maximum number of log files to keep.</returns>
        virtual int getMaxFiles() const;

    protected:
        spdlog::sink_ptr get() const override;
    };

    /// <summary>
    /// Forcefully terminates the application, if a log message of a certain level or higher is output.
    /// </summary>
    /// <remarks>
    /// The purpose of this sink is to allow tests to catch errors during application runtime. You probably do not want to use this sink in an actual
    /// application.
    /// </remarks>
    class LITEFX_LOGGING_API TerminationSink : public ISink {
        LITEFX_IMPLEMENTATION(TerminationSinkImpl);

    public:
        /// <summary>
        /// The default status code used to exit the application.
        /// </summary>
        static constexpr int DEFAULT_TERMINATION_STATUS = 0xFF455252;

    public:
        /// <summary>
        /// Creates a new exception sink instance.
        /// </summary>
        /// <param name="level">The minimum log level at which an exception is thrown.</param>
        /// <param name="status">The status code used to exit the application.</param>
        TerminationSink(const LogLevel& level = LogLevel::Info, int status = DEFAULT_TERMINATION_STATUS);
        TerminationSink(const TerminationSink&) = delete;
        TerminationSink(TerminationSink&&) = delete;
        virtual ~TerminationSink() noexcept;

    public:
        /// <inheritdoc />
        virtual LogLevel getLevel() const override;

        /// <inheritdoc />
        virtual String getName() const override;

        /// <inheritdoc />
        virtual String getPattern() const override;

    protected:
        virtual spdlog::sink_ptr get() const override;
    };

    /// <summary>
    /// A log to which messages are written to.
    /// </summary>
    /// <remarks>
    /// Note that in release builds, message at <see cref="LogLevel::Trace" /> and <see cref="LogLevel::Debug" /> are not forwarded and the corresponding functions are disabled. If you *really*
    /// want to log such messages, you have to specify the log level explicitly by calling <see cref="Log::log" />.
    /// </remarks>
    class LITEFX_LOGGING_API Log {
        LITEFX_IMPLEMENTATION(LogImpl);

    public:
        /// <summary>
        /// Creates a new log instance.
        /// </summary>
        /// <param name="name">The name of the log.</param>
        Log(const String& name);
        Log(Log&&) = delete;
        Log(const Log&) = delete;
        virtual ~Log() noexcept;

    public:
        /// <summary>
        /// Gets the name of the logger.
        /// </summary>
        virtual const String& getName() const noexcept;

    protected:
        virtual void log(LogLevel level, StringView message);

    public:
        /// <summary>
        /// Logs a message of <paramref name="level" /> with <paramref name="format" />.
        /// </summary>
        /// <param name="level">The log level of the message.</param>
        /// <param name="format">The format of the message.</param>
        template<typename ...TArgs>
        inline void log(LogLevel level, std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(level, std::format(format, std::forward<TArgs>(args)...));
        }

        /// <summary>
        /// Logs a trace message with <paramref name="format" />.
        /// </summary>
        /// <param name="format">The format of the message.</param>
        template<typename ...TArgs>
        inline void trace(std::format_string<TArgs...> format, TArgs&&... args) {
#ifndef NDEBUG
            this->log(LogLevel::Trace, format, std::forward<TArgs>(args)...);
#endif
        }

        /// <summary>
        /// Logs a debug message with <paramref name="format" />.
        /// </summary>
        /// <param name="format">The format of the message.</param>
        template<typename ...TArgs>
        inline void debug(std::format_string<TArgs...> format, TArgs&&... args) {
#ifndef NDEBUG
            this->log(LogLevel::Debug, format, std::forward<TArgs>(args)...);
#endif
        }

        /// <summary>
        /// Logs an info message with <paramref name="format" />.
        /// </summary>
        /// <param name="format">The format of the message.</param>
        template<typename ...TArgs>
        inline void info(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Info, format, std::forward<TArgs>(args)...);
        }

        /// <summary>
        /// Logs a warning message with <paramref name="format" />.
        /// </summary>
        /// <param name="format">The format of the message.</param>
        template<typename ...TArgs>
        inline void warning(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Warning, format, std::forward<TArgs>(args)...);
        }

        /// <summary>
        /// Logs an error message with <paramref name="format" />.
        /// </summary>
        /// <param name="format">The format of the message.</param>
        template<typename ...TArgs>
        inline void error(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Error, format, std::forward<TArgs>(args)...);
        }

        /// <summary>
        /// Logs a fatal error message with <paramref name="format" />.
        /// </summary>
        /// <param name="format">The format of the message.</param>
        template<typename ...TArgs>
        inline void fatal(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Fatal, format, std::forward<TArgs>(args)...);
        }
    };

    /// <summary>
    /// A provider for <see cref="Log" /> instances.
    /// </summary>
    class LITEFX_LOGGING_API Logger {
        LITEFX_IMPLEMENTATION(LoggerImpl);

    public:
        Logger(Logger&&) = delete;
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        virtual ~Logger() noexcept;

    private:
        Logger() noexcept;

        // TODO: Cache logs by name and return them, instead of re-creating them with each call.
    public:
        /// <summary>
        /// Retrieves a log from <paramref name="name" />.
        /// </summary>
        /// <param name="name">The name of the log to query.</param>
        /// <returns>A instance of a log.</returns>
        static Log get(StringView name);

        /// <summary>
        /// Allows a log to write messages to <paramref name="sink" />.
        /// </summary>
        /// <param name="sink">The sink to write log messages to.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="sink" /> is not initialized.</exception>
        static void sinkTo(const ISink* sink);
    };

}

#ifndef NDEBUG
#define LITEFX_TRACE(log, format, ...) LiteFX::Logging::Logger::get(log).trace(format, __VA_ARGS__)
#define LITEFX_DEBUG(log, format, ...) LiteFX::Logging::Logger::get(log).debug(format, __VA_ARGS__)
#else
#define LITEFX_TRACE(log, format, ...) 
#define LITEFX_DEBUG(log, format, ...) 
#endif

#define LITEFX_INFO(log, format, ...) LiteFX::Logging::Logger::get(log).info(format, __VA_ARGS__)
#define LITEFX_WARNING(log, format, ...) LiteFX::Logging::Logger::get(log).warning(format, __VA_ARGS__)
#define LITEFX_ERROR(log, format, ...) LiteFX::Logging::Logger::get(log).error(format, __VA_ARGS__)
#define LITEFX_FATAL_ERROR(log, format, ...) LiteFX::Logging::Logger::get(log).fatal(format, __VA_ARGS__)