/**
 * @file Logger.h
 * @brief Header file for Logger class
 */

#pragma once
#include <mutex>
#include <unistd.h>

/**
 * @class Logger
 * @brief A class for logging.
 *
 * This class provides logging API which can be used
 * from any place of the project. It is a singleton class.
 * To log anything just use macros LOG_TRACE, LOG_INFO, LOG_ERROR.
 *
 */
class Logger {
public:

enum LogLevel {
    TRACE,
    INFO,
    ERROR
};

#define LOG_ERROR(fmt, ...) Logger::Locate().Log(Logger::ERROR, fmt, ##__VA_ARGS__);
#define LOG_INFO(fmt, ...) Logger::Locate().Log(Logger::INFO, fmt, ##__VA_ARGS__);
#define LOG_TRACE(fmt, ...) Logger::Locate().Log(Logger::TRACE, fmt, ##__VA_ARGS__);

    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator = (const Logger&) = delete;
    ~Logger() = default;

    static Logger& Locate();

    template<typename ...Args>
    void Log(LogLevel level, const char *fmt, Args&&... args) {
        std::lock_guard l(mutex);

        if (level < currentLogLevel) return;

        if (level == ERROR) {
            dprintf(STDERR_FILENO, fmt, args...);
        } else {
            printf(fmt, args...);
        }
    };
    void SetLogLevel(LogLevel level) {
        currentLogLevel = level;
    };

private:
    Logger() = default;

    std::mutex mutex;
    LogLevel   currentLogLevel = ERROR;
};
