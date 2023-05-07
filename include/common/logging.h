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

std::string file;

template <typename... Args>
void log_printf(const char *format, Args... args) {
    if (file.empty()) {
        printf(format, args...);
    } else {
        FILE *f = fopen(file.c_str(), "a");
        if (f == nullptr) {
            printf("Failed to open log file %s\n", file.c_str());
            return;
        }

        fprintf(f, format, args...);
        fclose(f);
    }
}

template <typename... Args>
void Log(Level level, const char *format, Args... args) {
    switch (level) {
        case Level::DEBUG:
            log_printf("[DEBUG] ");
            break;
        case Level::INFO:
            log_printf("[INFO] ");
            break;
        case Level::WARN:
            log_printf("[WARN] ");
            break;
        case Level::ERROR:
            log_printf("[ERROR] ");
            break;
        case Level::FATAL:
            log_printf("[FATAL] ");
            break;
    }

    log_printf(format, args...);
    log_printf("\n");
}
}  // namespace

inline void set_logging_file(const std::string &path) {
    file = path;
}

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
