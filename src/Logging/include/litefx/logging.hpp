#pragma once

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

    enum class LITEFX_LOGGING_API LogLevel {
        Trace = SPDLOG_LEVEL_TRACE,
        Debug = SPDLOG_LEVEL_DEBUG,
        Info = SPDLOG_LEVEL_INFO,
        Warning = SPDLOG_LEVEL_WARN,
        Error = SPDLOG_LEVEL_ERROR,
        Fatal = SPDLOG_LEVEL_CRITICAL,
        Off = SPDLOG_LEVEL_OFF,
        Invalid = 0xFF
    };

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

    class LITEFX_LOGGING_API ConsoleSink : public ISink {
        LITEFX_IMPLEMENTATION(ConsoleSinkImpl);

    public:
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

    class LITEFX_LOGGING_API RollingFileSink : public ISink {
        LITEFX_IMPLEMENTATION(RollingFileSinkImpl);

    public:
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

        virtual String getFileName() const;

        virtual bool getTruncate() const;

        virtual int getMaxFiles() const;

    protected:
        spdlog::sink_ptr get() const override;
    };

    class LITEFX_LOGGING_API Log {
        LITEFX_IMPLEMENTATION(LogImpl);

    public:
        Log(const String& name);
        Log(Log&&) = delete;
        Log(const Log&) = delete;
        virtual ~Log() noexcept;

    public:
        /// <summary>
        /// Gets the name of the logger.
        /// </summary>
        virtual inline const String& getName() const noexcept;

    protected:
        virtual void log(LogLevel level, StringView message);

    public:
        template<typename ...TArgs>
        inline void log(LogLevel level, std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(level, std::format(format, std::forward<TArgs>(args)...));
        }

        template<typename ...TArgs>
        inline void trace(std::format_string<TArgs...> format, TArgs&&... args) {
#ifndef NDEBUG
            this->log(LogLevel::Trace, format, std::forward<TArgs>(args)...);
#endif
        }

        template<typename ...TArgs>
        inline void debug(std::format_string<TArgs...> format, TArgs&&... args) {
#ifndef NDEBUG
            this->log(LogLevel::Debug, format, std::forward<TArgs>(args)...);
#endif
        }

        template<typename ...TArgs>
        inline void info(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Info, format, std::forward<TArgs>(args)...);
        }

        template<typename ...TArgs>
        inline void warning(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Warning, format, std::forward<TArgs>(args)...);
        }

        template<typename ...TArgs>
        inline void error(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Error, format, std::forward<TArgs>(args)...);
        }

        template<typename ...TArgs>
        inline void fatal(std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(LogLevel::Fatal, format, std::forward<TArgs>(args)...);
        }
    };

    class LITEFX_LOGGING_API Logger {
        LITEFX_IMPLEMENTATION(LoggerImpl);

    public:
        Logger(Logger&&) = delete;
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        virtual ~Logger() noexcept;

    private:
        Logger() noexcept;

    public:
        // TODO: Cache logs by name and return them, instead of re-creating them with each call.
        static Log get(StringView name);
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