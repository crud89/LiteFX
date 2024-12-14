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

    class LITEFX_LOGGING_API ISink {
    protected:
        ISink() noexcept = default;
        ISink(const ISink&) = default;
        ISink(ISink&&) noexcept = default;
        ISink& operator=(const ISink&) = default;
        ISink& operator=(ISink&&) noexcept = default;

    public:
        virtual ~ISink() noexcept = default;

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
        ~ConsoleSink() noexcept override;

        ConsoleSink(const ConsoleSink&) = delete;
        ConsoleSink(ConsoleSink&&) noexcept = delete;
        auto operator=(const ConsoleSink&) = delete;
        auto operator=(ConsoleSink&&) noexcept = delete;

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
        ~RollingFileSink() noexcept override;

        RollingFileSink(const RollingFileSink&) = delete;
        RollingFileSink(RollingFileSink&&) noexcept = delete;
        auto operator=(const RollingFileSink&) = delete;
        auto operator=(RollingFileSink&&) noexcept = delete;

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
        virtual ~Log() noexcept;

        Log(Log&&) noexcept = delete;
        Log(const Log&) = delete;
        auto operator=(Log&&) noexcept = delete;
        auto operator=(const Log&) = delete;

    public:
        /// <summary>
        /// Gets the name of the logger.
        /// </summary>
        virtual const String& getName() const noexcept;

    protected:
        virtual void log(LogLevel level, StringView message);

    public:
        template<typename ...TArgs>
        inline void log(LogLevel level, std::format_string<TArgs...> format, TArgs&&... args) {
            this->log(level, std::format(format, std::forward<TArgs>(args)...));
        }

        template<typename ...TArgs>
        inline void trace([[maybe_unused]] std::format_string<TArgs...> format, [[maybe_unused]] TArgs&&... args) {
#ifndef NDEBUG
            this->log(LogLevel::Trace, format, std::forward<TArgs>(args)...);
#endif
        }

        template<typename ...TArgs>
        inline void debug([[maybe_unused]] std::format_string<TArgs...> format, [[maybe_unused]] TArgs&&... args) {
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
    private:
        Logger() noexcept;

    public:
        virtual ~Logger() noexcept;

        Logger(Logger&&) noexcept = delete;
        Logger(const Logger&) = delete;
        auto operator=(const Logger&) = delete;
        auto operator=(Logger&&) noexcept = delete;

    public:
        // TODO: Cache logs by name and return them, instead of re-creating them with each call.
        static Log get(StringView name);
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