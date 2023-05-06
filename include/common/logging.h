#ifndef SRC_LOGGING_H
#define SRC_LOGGING_H

#include <cstdio>

/// Logging isn't visible with normal usage - it should use file instead
namespace Log {

enum class Level {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
};

namespace {
template <typename... Args>
void Log(Level level, const char *format, Args... args) {
    switch (level) {
        case Level::DEBUG:
            printf("[DEBUG] ");
            break;
        case Level::INFO:
            printf("[INFO] ");
            break;
        case Level::WARN:
            printf("[WARN] ");
            break;
        case Level::ERROR:
            printf("[ERROR] ");
            break;
        case Level::FATAL:
            printf("[FATAL] ");
            break;
    }

    printf(format, args...);
    printf("\n");
}
}  // namespace

template <typename... Args>
void Info(const char *format, Args... args) {
    Log(Level::INFO, format, args...);
}

template <typename... Args>
void Debug(const char *format, Args... args) {
    Log(Level::DEBUG, format, args...);
}

template <typename... Args>
void Warn(const char *format, Args... args) {
    Log(Level::WARN, format, args...);
}

template <typename... Args>
void Error(const char *format, Args... args) {
    Log(Level::ERROR, format, args...);
}

template <typename... Args>
void Fatal(const char *format, Args... args) {
    Log(Level::FATAL, format, args...);
}

}  // namespace Log

#endif  // SRC_LOGGING_H
