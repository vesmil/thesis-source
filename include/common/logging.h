#ifndef SRC_LOGGING_H
#define SRC_LOGGING_H

#include <cstdio>
#include <string>

/// Logging isn't visible with normal usage - it should use file instead
class Logging {
public:
    template <typename... Args>
    static void Info(const char *format, Args... args) {
        Log(Level::INFO, format, args...);
    }

    template <typename... Args>
    static void Debug(const char *format, Args... args) {
        Log(Level::DEBUG, format, args...);
    }

    template <typename... Args>
    static void Warn(const char *format, Args... args) {
        Log(Level::WARN, format, args...);
    }

    template <typename... Args>
    static void Error(const char *format, Args... args) {
        Log(Level::ERROR, format, args...);
    }

    template <typename... Args>
    static void Fatal(const char *format, Args... args) {
        Log(Level::FATAL, format, args...);
    }

    enum class Level {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };

private:
    template <typename... Args>
    static void log_printf(const char *format, Args... args) {
        printf(format, args...);
    }

    template <typename... Args>
    static void Log(Level level, const char *format, Args... args) {
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
};

#endif  // SRC_LOGGING_H
