#ifndef LOGGER_H
#define LOGGER_H

#include "terminal.h"
#include "string_utils.h"
#include "io.h"

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static void setLogLevel(LogLevel level) {
        currentLogLevel = level;
    }

    template <typename... Args>
    static void log(LogLevel level, const char* format, Args... args) {
        if (level >= currentLogLevel) {
            char buffer[512];  // Adjust size as needed
            int length = format_string(buffer, sizeof(buffer), format, args...);

            term_printf("&%x%s %s\n", get_log_color(level), getLogLevelPrefix(level), buffer);
        }
    }

    template <typename... Args>
    static void logn(LogLevel level, const char* format, Args... args) {
        if (level >= currentLogLevel) {
            char buffer[512];  // Adjust size as needed
            int length = format_string(buffer, sizeof(buffer), format, args...);

            term_printf("&%x%s %s", get_log_color(level), getLogLevelPrefix(level), buffer);
        }
    }

    template <typename... Args>
    static void debug(const char* format, Args... args) {
        log(DEBUG, format, args...);
    }

    template <typename... Args>
    static void info(const char* format, Args... args) {
        log(INFO, format, args...);
    }

    template <typename... Args>
    static void warning(const char* format, Args... args) {
        log(WARNING, format, args...);
    }

    template <typename... Args>
    static void error(const char* format, Args... args) {
        log(ERROR, format, args...);
    }

    template <typename... Args>
    static void critical(const char* format, Args... args) {
        log(CRITICAL, format, args...);
    }

    static void serial_log(const char* format, ...) {
        char buffer[512]; // Buffer size for formatted message
        va_list args; // Variable argument list

        va_start(args, format); // Initialize args to retrieve additional arguments
        int result = vformat_string(buffer, sizeof(buffer), format, args); // Format the message
        va_end(args); // Clean up the variable argument list


        const char* message = buffer;
        while (*message) {
            outb(0x3F8, *message++);
        }
    }   

private:
    static inline LogLevel currentLogLevel = INFO;

    static const char* getLogLevelPrefix(LogLevel level) {
        switch (level) {
            case DEBUG:     return "[DEBUG]";
            case INFO:      return "[INFO]";
            case WARNING:   return "[WARNING]";
            case ERROR:     return "[ERROR]";
            case CRITICAL:  return "[CRITICAL]";
            default:        return "[UNKNOWN]";
        }
    }

    static const uint8_t get_log_color(LogLevel level) {
        switch (level) {
            case DEBUG:     return VGA_LIGHT_GRAY;
            case INFO:      return VGA_LIGHT_GREEN;
            case WARNING:   return VGA_YELLOW;
            case ERROR:     return VGA_LIGHT_RED;
            case CRITICAL:  return VGA_MAGENTA;
            default:        return VGA_WHITE; // Default case
        }
    }

};

#endif // LOGGER_H