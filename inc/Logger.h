#pragma once
#include <mutex>
#include <unistd.h>

class Logger {
public:

enum LogLevel {
    INFO,
    ERROR
};

#define LOG_ERROR(fmt, ...) Logger::Locate().Log(Logger::ERROR, fmt, ##__VA_ARGS__);
#define LOG_INFO(fmt, ...) Logger::Locate().Log(Logger::INFO, fmt, ##__VA_ARGS__);

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
