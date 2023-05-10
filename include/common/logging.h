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
    static std::string get_prefix(Level level) {
        switch (level) {
            case Level::DEBUG:
                return "[DEBUG]";
            case Level::INFO:
                return "[INFO]";
            case Level::WARN:
                return "[WARN]";
            case Level::ERROR:
                return "[ERROR]";
            case Level::FATAL:
                return "[FATAL]";
        }
    }

    template <typename... Args>
    static void Log(Level level, const char *format, Args... args) {
        std::string format_str = get_prefix(level) + " " + format + "\n";
        printf(format_str.c_str(), args...);
    }

    template <typename...>
    static void Log(Level level, const std::string &format) {
        Log(level, "%s", format.c_str());
    }
};

#endif  // SRC_LOGGING_H
