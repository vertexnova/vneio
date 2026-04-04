#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Common logging configuration for VneIo examples (headless I/O demos).
 * Mirrors vneinteraction/examples/common/logging_guard.h; requires
 * VNEIO_EXAMPLES_HAS_LOGGING when using the real implementation (set by
 * examples CMake when vnelogging headers are available).
 * ----------------------------------------------------------------------
 */

#if defined(VNEIO_EXAMPLES_HAS_LOGGING)

#include <vertexnova/logging/logging.h>

CREATE_VNE_LOGGER_CATEGORY("vneio.examples")

namespace vne::io::examples {

/**
 * @class LoggingGuard
 * @brief RAII guard for console logging configuration in examples.
 *
 * Initializes the logging system with console output in its constructor
 * and shuts it down in its destructor. Construct at the start of each
 * run*Example() entry point (same pattern as VneInteraction examples).
 *
 * Usage:
 * @code
 * int runMyExample() {
 *     LoggingGuard logging_guard;
 *     // ... example code ...
 *     return 0;
 * }
 * @endcode
 */
class LoggingGuard {
   public:
    LoggingGuard() {
        vne::log::LoggerConfig config;
        config.name = vne::log::kDefaultLoggerName;
        config.sink = vne::log::LogSinkType::eConsole;
        config.console_pattern = "[%l] [%n] %v";
        config.log_level = vne::log::LogLevel::eInfo;
        config.async = false;

        vne::log::Logging::configureLogger(config);
    }

    ~LoggingGuard() { vne::log::Logging::shutdown(); }

    LoggingGuard(const LoggingGuard&) = delete;
    LoggingGuard& operator=(const LoggingGuard&) = delete;
};

}  // namespace vne::io::examples

#else

#include <iostream>
#include <sstream>

namespace vne::io::examples {

/**
 * @brief Streams one log line to cout/cerr when vnelogging is absent.
 *
 * Matches the `VNE_LOG_INFO << ...` usage pattern from vertexnova/logging.
 */
class ExampleLogStream {
   public:
    explicit ExampleLogStream(std::ostream& out, const char* prefix = nullptr)
        : out_(out)
        , prefix_(prefix) {}

    template<typename T>
    ExampleLogStream& operator<<(const T& msg) {
        oss_ << msg;
        return *this;
    }

    ~ExampleLogStream() {
        if (prefix_) {
            out_ << prefix_;
        }
        out_ << oss_.str() << '\n';
    }

    ExampleLogStream(const ExampleLogStream&) = delete;
    ExampleLogStream& operator=(const ExampleLogStream&) = delete;

   private:
    std::ostringstream oss_;
    std::ostream& out_;
    const char* prefix_;
};

/** No-op RAII when vnelogging is not linked; console stubs still work below. */
struct LoggingGuard {};

}  // namespace vne::io::examples

#define VNE_LOG_TRACE ::vne::io::examples::ExampleLogStream(std::cout, "[TRACE] ")
#define VNE_LOG_DEBUG ::vne::io::examples::ExampleLogStream(std::cout, "[DEBUG] ")
#define VNE_LOG_INFO ::vne::io::examples::ExampleLogStream(std::cout)
#define VNE_LOG_WARN ::vne::io::examples::ExampleLogStream(std::cerr, "[WARN] ")
#define VNE_LOG_ERROR ::vne::io::examples::ExampleLogStream(std::cerr, "[ERROR] ")
#define VNE_LOG_FATAL ::vne::io::examples::ExampleLogStream(std::cerr, "[FATAL] ")

#endif
