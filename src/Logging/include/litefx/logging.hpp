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
#include <spdlog/sinks/sink.h>
#include <mutex>

namespace LiteFX::Logging {
    using namespace LiteFX;

    enum class LITEFX_LOGGING_API LogLevel {
        Trace = 0x01,
        Debug = 0x02,
        Info = 0x03,
        Warning = 0x04,
        Error = 0x05,
        Fatal = 0x06,
        Invalid = 0xFF
    };

    class LITEFX_LOGGING_API ISink {
    public:
        /// <summary>
        /// Gets the minimum log level for messages to get written to the log.
        /// </summary>
        virtual const LogLevel& getLevel() const = 0;

        /// <summary>
        /// Gets the name of the sink.
        /// </summary>
        virtual const String& getName() const = 0;

        /// <summary>
        /// Gets the pattern used to format the messages for the sink.
        /// </summary>
        virtual const String& getPattern() const = 0;

    protected:
        friend class Logger;
        virtual SharedPtr<spdlog::sinks::sink> get() const = 0;
    };

    class LITEFX_LOGGING_API ConsoleSink : public ISink {
        LITEFX_IMPLEMENTATION(ConsoleSinkImpl)

    public:
        ConsoleSink(const LogLevel& level, const String& name, const String& pattern) noexcept;
        ConsoleSink(const ConsoleSink&) = delete;
        ConsoleSink(ConsoleSink&&) = delete;
        virtual ~ConsoleSink() noexcept;

    public:
        /// <inheritdoc />
        virtual const LogLevel& getLevel() const override;

        /// <inheritdoc />
        virtual const String& getName() const override;

        /// <inheritdoc />
        virtual const String& getPattern() const override;

    protected:
        virtual SharedPtr<spdlog::sinks::sink> get() const override;
    };

    class LITEFX_LOGGING_API RollingFileSink : public ISink {
        LITEFX_IMPLEMENTATION(RollingFileSinkImpl)

    public:
        RollingFileSink(const LogLevel& level, const String& name, const String& pattern, const String& fileName, const bool& truncate = false, const int& maxFiles = 0) noexcept;
        RollingFileSink(const RollingFileSink&) = delete;
        RollingFileSink(RollingFileSink&&) = delete;
        virtual ~RollingFileSink() noexcept;

    public:
        /// <inheritdoc />
        virtual const LogLevel& getLevel() const override;

        /// <inheritdoc />
        virtual const String& getName() const override;

        /// <inheritdoc />
        virtual const String& getPattern() const override;

        virtual const String& getFileName() const;

        virtual bool getTruncate() const;

        virtual int getMaxFiles() const;

    protected:
        virtual SharedPtr<spdlog::sinks::sink> get() const override;
    };

    class LITEFX_LOGGING_API Log {
        LITEFX_IMPLEMENTATION(LogImpl)

    public:
        Log(const String& name) noexcept;
        Log(Log&&) = delete;
        Log(const Log&) = delete;
        virtual ~Log() noexcept;

    public:
        /// <summary>
        /// Gets the name of the logger.
        /// </summary>
        virtual const String& getName() const noexcept;

    protected:
        virtual void log(const LogLevel& level, const String& message);

    public:
        template<typename ...TArgs>
        void log(const LogLevel& level, const String& format, TArgs&&... args);

        template<typename ...TArgs>
        void trace(const String& format, TArgs&&... args);

        template<typename ...TArgs>
        void debug(const String& format, TArgs&&... args);

        template<typename ...TArgs>
        void info(const String& format, TArgs&&... args);

        template<typename ...TArgs>
        void warning(const String& format, TArgs&&... args);

        template<typename ...TArgs>
        void error(const String& format, TArgs&&... args);

        template<typename ...TArgs>
        void fatal(const String& format, TArgs&&... args);
    };

    class LITEFX_LOGGING_API Logger {
        LITEFX_IMPLEMENTATION(LoggerImpl)

    public:
        Logger(Logger&&) = delete;
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        virtual ~Logger() noexcept;

    private:
        Logger() noexcept;

    public:
        // TODO: Cache logs by name and return them, instead of re-creating them with each call.
        static Log get(const String & name);

        // Adds a sink.
        template <typename TSink, std::enable_if_t<std::is_convertible_v<TSink*, ISink*>, int> = 0, typename ...TArgs>
        static void sinkTo(const LogLevel & level, const String & name, const String & pattern, TArgs &&... args);
    };

}