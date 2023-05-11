#ifndef SRC_LOGGING_H
#define SRC_LOGGING_H

#include <cstdio>
#include <string>

/**
 * @brief Placeholder class for logging
 * @note Should be replace with proper logging library in the future
 */
class Logging {
public:
    ///  Log a formatted message with info level
    template <typename... Args>
    static void Info(const char *format, Args... args) {
        Log(Level::INFO, format, args...);
    }

    /// @brief Log a formatted message with debug level
    template <typename... Args>
    static void Debug(const char *format, Args... args) {
        Log(Level::DEBUG, format, args...);
    }

    /// @brief Log a formatted message with warn level
    template <typename... Args>
    static void Warn(const char *format, Args... args) {
        Log(Level::WARN, format, args...);
    }

    /// @brief Log a formatted message with error level
    template <typename... Args>
    static void Error(const char *format, Args... args) {
        Log(Level::ERROR, format, args...);
    }

    /// @brief Log a formatted message with fatal level
    template <typename... Args>
    static void Fatal(const char *format, Args... args) {
        Log(Level::FATAL, format, args...);
    }

    /// @brief Supported logging levels
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

        return "[UNKNOWN]";
    }

    template <typename... Args>
    static void Log(Level level, const char *format, Args... args) {
        std::string format_str = get_prefix(level) + " " + format + "\n";
        fprintf(stderr, format_str.c_str(), args...);
    }

    static void Log(Level level, const char *format) {
        std::string format_str = get_prefix(level) + " " + format + "\n";
        fprintf(stderr, "%s", format_str.c_str());
    }
};

#endif  // SRC_LOGGING_H
